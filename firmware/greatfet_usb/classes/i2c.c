/*
 * This file is part of GreatFET
 */

#include <debug.h>
#include <errno.h>
#include <string.h>
#include <toolchain.h>

#include <greatfet_core.h>

#include <drivers/comms.h>
#include <drivers/memory/allocator.h>

#include <i2c.h>
#include <i2c_bus.h>

#include "../usb_streaming.h"


#define CLASS_NUMBER_SELF (0x108)


typedef struct {
	uint8_t address;

	uint32_t read_length;
	uint32_t write_length;

	void   *write_data;
	uint8_t read_data[256];

} i2c_stream_t;


// Pointer to a buffer that stores the I2C write data used in e.g. streaming I2C reads.
static i2c_stream_t stream;


static int i2c_verb_start(struct command_transaction *trans)
{
	uint16_t duty_cycle_count;
	uint16_t value = comms_argument_parse_uint16_t(trans);

	if (value == 0) {
		duty_cycle_count = 255;
	} else {
		duty_cycle_count = value;
	}
	i2c_bus_start(&i2c0, duty_cycle_count);

	return 0;
}

static int i2c_verb_stop()
{
	i2c_bus_stop(&i2c0);
	return 0;
}

static int i2c_verb_read(struct command_transaction *trans)
{
	uint16_t address 		= comms_argument_parse_uint16_t(trans);
	uint16_t rx_length 		= comms_argument_parse_uint16_t(trans);
	uint8_t *i2c_rx_buffer 	= comms_response_reserve_space(trans, rx_length);
	// TODO: data validation

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// FIXME: handle the read status
	i2c_bus_read(&i2c0, address, i2c_rx_buffer, rx_length);

	return 0;
}

static int i2c_verb_write(struct command_transaction *trans)
{
	uint32_t tx_length;
	uint16_t address 		= comms_argument_parse_uint16_t(trans);
	uint8_t *data_to_write 	= comms_argument_read_buffer(trans, -1, &tx_length);
	// TODO: data validation

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}
	i2c_bus_write(&i2c0, address, data_to_write, tx_length);

	return 0;
}

static int i2c_verb_repeated_transmit(struct command_transaction *trans)
{
	uint32_t tx_length_single;
	uint16_t address			= comms_argument_parse_uint16_t(trans);
	uint16_t rx_length_single	= comms_argument_parse_uint16_t(trans);
	uint8_t  transmit_count		= comms_argument_parse_uint8_t(trans);
	uint8_t *data_to_write 		= comms_argument_read_buffer(trans, -1, &tx_length_single);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	uint16_t rx_length 			= rx_length_single * transmit_count;
	uint8_t *i2c_rx_buffer 		= comms_response_reserve_space(trans, rx_length);

	if (!i2c_rx_buffer) {
		pr_error("error: i2c: cannot capture %d bytes simultaneously; data won't fit in response!\n");
		return ENOMEM;
	}

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	uint8_t rx_buffer_position = 0;

	for (uint8_t i = 0; i < transmit_count; i++)
	{
		i2c_bus_write(&i2c0, address, data_to_write, (size_t) tx_length_single);
		i2c_bus_read(&i2c0, address, &i2c_rx_buffer[rx_buffer_position], (size_t) rx_length_single);
		rx_buffer_position += rx_length_single;
	}

	return 0;
}


static int i2c_verb_issue_start(struct command_transaction *trans)
{
	(void)trans;
	i2c_tx_start(I2C0_BASE);
	return 0;
}


static int i2c_verb_issue_stop(struct command_transaction *trans)
{
	(void)trans;
	i2c_stop(I2C0_BASE);
	return 0;
}


static int i2c_verb_issue_bytes(struct command_transaction *trans)
{

	while (comms_argument_data_remaining(trans)) {

		// Transmit each byte in our data stream...
		i2c_tx_byte(I2C0_BASE, comms_argument_parse_uint8_t(trans));

		// FIXME: respond with status

	}


	return 0;
}

static int i2c_verb_read_bytes(struct command_transaction *trans)
{
	uint16_t count = comms_argument_parse_uint16_t(trans);
	bool     ack_last = comms_argument_parse_bool(trans);

	uint8_t *data_buffer = comms_response_reserve_space(trans, count);

	if (!data_buffer) {
		pr_error("error: i2c: cannot capture %d bytes simultaneously; data won't fit in response!\n");
		return ENOMEM;
	}

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// Receive each of our bytes.
	for (int i=0; i < count; i++) {
		bool ack = ack_last ? true : (i + 1 != count);
		data_buffer[i] = i2c_rx_byte(I2C0_BASE, ack);
	}

	return 0;
}


static int i2c_verb_scan(struct command_transaction *trans)
{
	uint8_t read_status, write_status;

	uint8_t *write_status_buffer = comms_response_reserve_space(trans, 16);
	uint8_t *read_status_buffer = comms_response_reserve_space(trans, 16);
	uint8_t address;

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	for (address = 0; address < 16; address++) {
		write_status_buffer[address] = 0;
		read_status_buffer[address] = 0;
	}

	for (address = 0; address < 128; address++) {
		write_status = i2c_bus_write(&i2c0, address, NULL, 0);
		if (write_status == 0x18) {
			write_status_buffer[address >> 3] |= 1 << (address & 0x07);
		}

		read_status = i2c_bus_read(&i2c0, address, NULL, 0);
		if (read_status == 0x40) {
			read_status_buffer[address >> 3] |= 1 << (address & 0x07);
		}
	}

	return 0;
}


void stream_i2c_data(void *argument)
{
	i2c_stream_t *stream = (i2c_stream_t *)argument;

	// Perform our I2C read operation...
	i2c_bus_write(&i2c0, stream->address, stream->write_data, stream->write_length);
	i2c_bus_read(&i2c0, stream->address, stream->read_data, stream->read_length);

	// ... and gather the resultant data.
	usb_streaming_send_data(stream->read_data, stream->read_length);
}



static int i2c_verb_stream_periodic_read(struct command_transaction *trans)
{
	uint32_t frequency = comms_argument_parse_uint32_t(trans);
	void *data_to_write;

	stream.address     = comms_argument_parse_uint8_t(trans);
	stream.read_length = comms_argument_parse_uint8_t(trans);
	data_to_write      = comms_argument_read_buffer(trans, -1, &stream.write_length);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// If we already have a buffer, stop any existing transfers and clean up before creating a new one.
	if (stream.write_data) {
		usb_streaming_stop_periodic_gathering();
		free(stream.write_data);
	}

	// Allocate a buffer for which to store our write data, and copy it in.
	stream.write_data = malloc(stream.write_length);
	if (!stream.write_data) {
		pr_error("error: i2c streaming: could not allocate a buffer on the heap!\n");
		return ENOMEM;
	}
	memcpy(stream.write_data, data_to_write, stream.write_length);

	// Finally, schedule our periodic read.
	usb_streaming_start_periodic_data_gathering(frequency, stream_i2c_data, &stream);
	comms_response_add_uint8_t(trans, USB_STREAMING_IN_ADDRESS);
	return 0;
}


static int i2c_verb_stop_periodic_read(struct command_transaction *trans)
{
	(void)trans;

	usb_streaming_stop_periodic_gathering();
	return 0;
}



/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {

		// Basic control.
		{ .name = "start", .handler = i2c_verb_start,
			.in_signature = "<I", .out_signature = "",
			.in_param_names = "value, duty_cycle_count",
			.doc = "Initialize and transmit a start bit to an I2C device" },
		{ .name = "stop", .handler = i2c_verb_stop,
			.in_signature = "", .out_signature = "",
			.in_param_names = "",
			.doc = "Transmit a stop bit to an I2C device" },

		// High-level transmit/receive/scan.
		{ .name = "read", .handler = i2c_verb_read,
			.in_signature = "<HH", .out_signature = "<*B",
			.in_param_names = "address, length", .out_param_names = "response",
			.doc = "Reads from the I2C bus and responds accordingly" },
		{ .name = "write", .handler = i2c_verb_write,
			.in_signature = "<H*X", .out_signature = "<",
			.in_param_names = "address, data", .out_param_names = "",
			.doc = "Writes to the I2C bus and responds accordingly" },
		{ .name = "repeated_transmit", .handler = i2c_verb_repeated_transmit,
			.in_signature = "<HHB*X", .out_signature = "<*B",
			.in_param_names = "address, rx_length, count, data",
			.out_param_names = "response",
			.doc = "Repeats a write/read transmission to the I2C bus" },
		{ .name = "scan", .handler = i2c_verb_scan,
			.in_signature = "", .out_signature = "<*B",
			.in_param_names = "value, index, data", .out_param_names = "states",
			.doc = "Scans all valid I2C addresses for attached devices" },

		// Low-level send/receive, for bus pirate mode.
		{ .name = "issue_start", .handler = i2c_verb_issue_start,
			.in_signature = "<", .out_signature = "<",
			.doc = "Issues a raw start bit onto the I2C bus." },
		{ .name = "issue_stop", .handler = i2c_verb_issue_stop,
			.in_signature = "<", .out_signature = "<",
			.doc = "Issues a raw stop bit on the I2C bus." },
		{ .name = "issue_bytes", .handler = i2c_verb_issue_bytes,
			.in_signature = "<*X", .out_signature = "<*?", .in_param_names = "data", .out_param_names="ack_array",
			.doc = "Issues a raw set of bytes on the I2C bus. Gives low-level control over transmit." },
		{ .name = "read_bytes", .handler = i2c_verb_read_bytes,
			.in_signature = "<H?", .out_signature = "<*X", .in_param_names = "count, ack_last",
			.doc = "Reads a raw set of bytes on the I2C bus. Should follow an issued address." },


		// Functionality for streaming repeated sampes over a bulk pipe.
		{ .name = "stream_periodic_read", .handler = i2c_verb_stream_periodic_read,
			.in_signature = "<IBB*X", .out_signature = "<B",
			.in_param_names = "frequency, address, read_length, write_data", .out_param_names = "pipe_id",
			.doc = "Schedule a periodic I2C transaction, and stream its results to the host." },
		{ .name = "stop_periodic_read", .handler = i2c_verb_stop_periodic_read,
			.in_signature = "", .out_signature = "",
			.doc = "Stop any active periodic read."},

		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(i2c, CLASS_NUMBER_SELF, "i2c", _verbs,
		"API for I2C communication.");

