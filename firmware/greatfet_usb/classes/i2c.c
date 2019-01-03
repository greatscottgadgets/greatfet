/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <greatfet_core.h>
#include <i2c_bus.h>
#include <i2c.h>
#include <errno.h>
#include <drivers/comms.h>


#define CLASS_NUMBER_SELF (0x108)

static uint16_t duty_cycle_count;
static uint8_t status;

static int i2c_verb_start(struct command_transaction *trans)
{
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

static int i2c_verb_xfer(struct command_transaction *trans)
{
	uint32_t tx_length;
	uint16_t address 		= comms_argument_parse_uint16_t(trans);
	uint16_t rx_length 		= comms_argument_parse_uint16_t(trans);
	uint8_t *data_to_write 	= comms_argument_read_buffer(trans, -1, &tx_length);
	uint8_t *i2c_rx_buffer 	= comms_response_reserve_space(trans, rx_length);
	// TODO: data validation

	if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	status = i2c_bus_transfer(&i2c0, address & 0xff, data_to_write, tx_length, 
					 i2c_rx_buffer, rx_length);
	comms_response_add_uint8_t(trans, status);

	return 0;
}

/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "start", .handler = i2c_verb_start,
			.in_signature = "<I", .out_signature = "",
			.in_param_names = "value, duty_cycle_count",
			.doc = "Initialize and transmit a start bit to an I2C device" },
		{ .name = "stop", .handler = i2c_verb_stop,
			.in_signature = "", .out_signature = "",
			.in_param_names = "",
			.doc = "Transmit a stop bit to an I2C device" },
		{ .name = "xfer", .handler = i2c_verb_xfer,
			.in_signature = "<HH*X", .out_signature = "<*B",
			.in_param_names = "value, index, data", .out_param_names = "response, status",
			.doc = "Transmits data over the I2C bus and responds accordingly" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(i2c, CLASS_NUMBER_SELF, "i2c", _verbs,
		"API for I2C communication.");

