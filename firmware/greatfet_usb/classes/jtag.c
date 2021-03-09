/*
 * This file is part of GreatFET
 */

#include <debug.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include <drivers/gpio.h>
#include <drivers/comms.h>
#include <drivers/platform_clock.h>

#define CLASS_NUMBER_SELF (0x10B)

// For now, we're using a very simple bit-banged implementation of JTAG.
// I have an SGPIO-based implementation, but it winds up with very inconvenient
// pin locations (and an inability to go below ~50 kHz). It's likely we'll eventually
// include both implementations as "jtag" and "fast_jtag", but it's worth mainlining
// this simple implementation first. ~ktemkin
#define MAX_ACHIEVABLE_FREQUENCY (400 * 1000) // 400 kHz

/**
 * Define each of our JTAG pins as GPIO. We'll avoid using
 * the SGPIO until it's proven necessary here; as ths pin layouts
 * are a lot cleaner using GPIO, and this gives us a lot of remapping
 * flexibility if we want to, later.
 */
static const gpio_pin_t tdo_gpio = { .port = 0, .pin =  0 }; // J1_P3
static const gpio_pin_t tdi_gpio = { .port = 5, .pin = 13 }; // J1_P4
static const gpio_pin_t tck_gpio = { .port = 0, .pin =  1 }; // J1_P6
static const gpio_pin_t tms_gpio = { .port = 5, .pin = 14 }; // J1_P5


/**
 * JTAG configuration variables.
 */
uint32_t half_period_delay = 0;
uint32_t next_edge_time    = 0;


static int verb_configure(struct command_transaction *trans)
{
	const gpio_pin_t pins_to_configure[] = { tdo_gpio, tdi_gpio, tck_gpio, tms_gpio};
	uint32_t frequency = comms_argument_parse_uint32_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// FIXME: reserve these with our pin manager!

	// Configure each of the pins to be used as GPIO.
	for (unsigned i = 0; i < ARRAY_SIZE(pins_to_configure); ++i) {
		int rc = gpio_configure_pinmux(pins_to_configure[i]);

		uint8_t port = pins_to_configure[i].port;
		uint8_t pin  = pins_to_configure[i].pin;
		if (rc) {
			pr_error("error: jtag: failed to grab GPIO pin %u[%u]\n", port, pin);
			return rc;
		}
	}

	// Set the directions of each our JTAG pins.
	gpio_set_pin_direction(tdo_gpio, false);
	gpio_set_pin_direction(tdi_gpio, true);
	gpio_set_pin_direction(tms_gpio, true);
	gpio_set_pin_direction(tck_gpio, true);

	// Compute how many delays we'll need per cycle.
	if (frequency > MAX_ACHIEVABLE_FREQUENCY) {
		half_period_delay = 0;
	} else {
		// Figure out the half-period time, rounding up to the next whole microsecond.
		half_period_delay = ((1000000 + (frequency - 1)) / frequency) / 2;
	}
	pr_debug("jtag: using half-period delay of %u microseconds to achieve frequency of %u\n", half_period_delay, frequency);
	next_edge_time = get_time() + half_period_delay;

	// Return the total number of bits we can shift per single call.
	// We'll assume we can fit essentially a full buffer, so we'll say 4000 * 8 to allow for argument overhead.
	comms_response_add_uint32_t(trans, 4090 * 8);
	return 0;
}


static inline void wait_for_next_edge_time(void)
{
	// If we don't have a delay, return immediately.
	if (likely(!half_period_delay)) {
		return;
	}

	while (get_time() < next_edge_time);
	next_edge_time = get_time() + half_period_delay;
}


/**
 * Generates a single cycle of bit-banged JTAG.
 */
static inline bool jtag_tick(bool tdi)
{
	bool tdo;

	wait_for_next_edge_time();

	tdo = gpio_get_pin_value(tdo_gpio);
	gpio_set_pin(tck_gpio);

	// Apply our TDI and TMS values.
	gpio_set_pin_value(tdi_gpio, tdi);

	// Create a falling edge.
	wait_for_next_edge_time();
	gpio_clear_pin(tck_gpio);

	// Return the scanned in value from TDI.
	return tdo;
}

//
// Note that there's a lot of redundancy in these three functions.
// This is because GCC's not particularly good at splitting a single function with
// constants into speed-critical properties; even with proper annotations.
// (LLVM does much better, but gcc support is neccessary, so c'est la vie).
//


static int verb_scan(struct command_transaction *trans)
{
	uint8_t transmit_byte = 0, receive_byte = 0;
	uint8_t bits_left_in_tx_byte = 0, bits_left_in_rx_byte = 8;

	uint32_t bit_count         = comms_argument_parse_uint32_t(trans);
	uint32_t advance_state     = comms_argument_parse_bool(trans);

	// Ensure TMS isn't set, so we don't advance through the FSM.
	gpio_clear_pin(tms_gpio);

	// While there are any bits remaining to transfer, do so.
	while (bit_count) {

		bool to_transmit, received;
		bool should_pulse_tms = advance_state && (bit_count == 1);

		// If we don't have any bits queued to process for the current byte,
		// get a new byte, and commit the data we've already worked with.
		if (!bits_left_in_tx_byte) {
			transmit_byte = comms_argument_parse_uint8_t(trans);
			bits_left_in_tx_byte = 8;
		}
		if (!bits_left_in_rx_byte) {
			comms_response_add_uint8_t(trans, receive_byte);
			bits_left_in_rx_byte = 8;
		}

		// Grab the LSB, and queue it for transmit.
		to_transmit = transmit_byte & 1;
		transmit_byte >>= 1;

		// If this is the final bit and we're advancing state, set TMS.
		if (unlikely(should_pulse_tms)) {
			gpio_set_pin(tms_gpio);
		}

		// Perform out transmission.
		received = jtag_tick(to_transmit);

		// Merge the received bit into our active rx byte.
		receive_byte = (receive_byte >> 1) | (received ? 0x80 : 0);

		--bit_count;
		--bits_left_in_tx_byte;
		--bits_left_in_rx_byte;
	}

	// If we have bits left in the byte we're building, and we're expecting a response,
	// shift our data into the LSB spot and then add it to our response.
	receive_byte >>= bits_left_in_rx_byte;
	comms_response_add_uint8_t(trans, receive_byte);

	return 0;
}


static int verb_scan_out(struct command_transaction *trans)
{
	uint8_t transmit_byte = 0;
	uint8_t bits_left_in_tx_byte = 0;

	uint32_t bit_count         = comms_argument_parse_uint32_t(trans);
	uint32_t advance_state     = comms_argument_parse_bool(trans);

	// Ensure TMS isn't set, so we don't advance through the FSM.
	gpio_clear_pin(tms_gpio);

	// While there are any bits remaining to transfer, do so.
	while (bit_count) {

		bool to_transmit;
		bool should_pulse_tms = advance_state && (bit_count == 1);

		// If we don't have any bits queued to process for the current byte,
		// get a new byte, and commit the data we've already worked with.
		if (!bits_left_in_tx_byte) {
			transmit_byte = comms_argument_parse_uint8_t(trans);
			bits_left_in_tx_byte = 8;
		}

		// Grab the LSB, and queue it for transmit.
		to_transmit = transmit_byte & 1;
		transmit_byte >>= 1;

		// If this is the final bit and we're advancing state, set TMS.
		if (unlikely(should_pulse_tms)) {
			gpio_set_pin(tms_gpio);
		}

		// Perform out transmission.
		jtag_tick(to_transmit);

		--bit_count;
		--bits_left_in_tx_byte;
	}

	return 0;
}


static int verb_scan_in(struct command_transaction *trans)
{
	uint8_t receive_byte = 0;
	uint8_t bits_left_in_rx_byte = 8;

	uint32_t bit_count         = comms_argument_parse_uint32_t(trans);
	uint32_t advance_state     = comms_argument_parse_bool(trans);

	// Ensure TMS isn't set, so we don't advance through the FSM.
	gpio_clear_pin(tms_gpio);

	// While there are any bits remaining to transfer, do so.
	while (bit_count) {

		bool received;
		bool should_pulse_tms = advance_state && (bit_count == 1);

		// If we don't have any bits queued to process for the current byte,
		// get a new byte, and commit the data we've already worked with.
		if (!bits_left_in_rx_byte) {
			comms_response_add_uint8_t(trans, receive_byte);
			bits_left_in_rx_byte = 8;
		}

		// If this is the final bit and we're advancing state, set TMS.
		if (unlikely(should_pulse_tms)) {
			gpio_set_pin(tms_gpio);
		}

		// Perform out transmission.
		received = jtag_tick(0);

		// Merge the received bit into our active rx byte.
		receive_byte = (receive_byte >> 1) | (received ? 0x80 : 0);

		--bit_count;
		--bits_left_in_rx_byte;
	}

	// If we have bits left in the byte we're building, and we're expecting a response,
	// shift our data into the LSB spot and then add it to our response.
	receive_byte >>= bits_left_in_rx_byte;
	comms_response_add_uint8_t(trans, receive_byte);

	return 0;
}


static int verb_run_clock(struct command_transaction *trans)
{
	uint32_t bit_count         = comms_argument_parse_uint32_t(trans);
	uint32_t advance_state     = comms_argument_parse_bool(trans);

	// Ensure TMS isn't set, so we don't advance through the FSM.
	gpio_clear_pin(tms_gpio);

	while(bit_count--) {
		bool should_pulse_tms = advance_state && !bit_count;

		// If this is the final bit and we're advancing state, set TMS.
		if (unlikely(should_pulse_tms)) {
			gpio_set_pin(tms_gpio);
		}

		jtag_tick(0);
	}

	return 0;
}


/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {

		// Configuration.
		{ .name = "configure", .handler = verb_configure, .in_signature = "<I",
		    .out_signature = "<I", .in_param_names = "max_frequency", .out_param_names = "bits_max",
            .doc = "Configures a JTAG scan chain; can be run multiple times, but must be run before use.\n"
			"\n"
			"Paramters:\n"
			"    max_frequency -- The maximum frequency to which communications should be constrainted.\n"
			"                     Should not exceed 102MHz.\n"
			"\n"
			"Returns:\n"
			"    bits_max -- The maximum number of bits that can be provided to our shift() verb.\n"
		},

		// Basic communications.
		{ .name = "scan", .handler = verb_scan, .in_signature = "<I?*X",
		    .out_signature = "<*X", .in_param_names = "bits_to_scan, advance_state, data_to_shift", .out_param_names = "response",
            .doc = "Scans a set of data out to the chain, and returns a response."
			"\n"
			"Paramters:\n"
			"    bits_to_scan  -- The total number of bits to scan.\n"
			"    advance_state -- If true, TMS will be asserted during the last bit of the scan.\n"
			"    data_to_shift -- The packed data to be shifted out; as a sequence of bytes that will each be scanned out, LSB first.\n"
			"\n"
			"Returns:\n"
			"    response -- The packed data that was returned on TDI; in the same format as received.\n"
		},
		{ .name = "scan_out", .handler = verb_scan_out, .in_signature = "<I?*X",
		    .out_signature = "<", .in_param_names = "bits_to_scan, advance_state, data_to_shift", .out_param_names = "",
            .doc = "Scans a set of data out to the chain, discarding any response.\n"
			"\n"
			"Paramters:\n"
			"    bits_to_scan  -- The total number of bits to scan.\n"
			"    advance_state -- If true, TMS will be asserted during the last bit of the scan.\n"
			"    data_to_shift -- The packed data to be shifted out; as a sequence of bytes that will each be scanned out, LSB first.\n"
		},
		{ .name = "scan_in", .handler = verb_scan_in, .in_signature = "<I?",
		    .out_signature = "<*X", .in_param_names = "bits_to_scan, advance_state, data_to_shift", .out_param_names = "",
            .doc = "Scans a set of data in from the chain, scanning out all filler bits.\n"
			"\n"
			"Paramters:\n"
			"    bits_to_scan  -- The total number of bits to scan.\n"
			"    advance_state -- If true, TMS will be asserted during the last bit of the scan.\n"
			"\n"
			"Returns:\n"
			"    response -- The packed data that was returned on TDI; in the same format as received.\n"
		},
		{ .name = "run_clock", .handler = verb_run_clock, .in_signature = "<I?",
		    .out_signature = "<", .in_param_names = "bits_to_scan, advance_state", .out_param_names = "",
            .doc = "Pulses the clock for the chain; but neither scans in nor scans out meaningful data.\n"
			"\n"
			"Paramters:\n"
			"    bits_to_scan  -- The total number of bits to scan.\n"
			"    advance_state -- If true, TMS will be asserted during the last bit of the scan.\n"
		},


		// Every verb list ends with an empty entry. This acts like a null terminator
		// for our list of verbs.
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(jtag, CLASS_NUMBER_SELF, "jtag", _verbs,
        "Class that facilitates controlling a JTAG scan chain from the host.")

