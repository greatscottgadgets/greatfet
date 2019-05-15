/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include <drivers/sgpio.h>
#include <drivers/usb/usb.h>
#include <drivers/usb/usb_queue.h>

#include "../usb_streaming.h"

#include <drivers/platform_clock.h>

#include <errno.h>
#include <debug.h>
#include <toolchain.h>

#define CLASS_NUMBER_SELF (0x10D)

// Set the default frequency for our logic analyzer.
#define LOGIC_ANALYZER_DEFAULT_FREQUENCY (17 * 1000000)
#define LOGIC_ANALYZER_DEFAULT_WIDTH     (8)

static bool use_bank_b = false;

/**
 * Default set of pin mappings for each of the logic analyzer functions.
 */
static sgpio_pin_configuration_t logic_analyzer_pins[] = {
	{ .sgpio_pin = 0,  .scu_group = 0, .scu_pin =  0, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 1,  .scu_group = 0, .scu_pin =  1, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 2,  .scu_group = 1, .scu_pin = 15, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 3,  .scu_group = 1, .scu_pin = 16, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 4,  .scu_group = 6, .scu_pin =  3, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 5,  .scu_group = 6, .scu_pin =  6, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 6,  .scu_group = 2, .scu_pin =  2, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 7,  .scu_group = 1, .scu_pin =  0, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 8,  .scu_group = 9, .scu_pin =  6, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 9,  .scu_group = 4, .scu_pin =  3, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 10, .scu_group = 1, .scu_pin = 14, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 11, .scu_group = 1, .scu_pin = 17, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 12, .scu_group = 1, .scu_pin = 18, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 13, .scu_group = 4, .scu_pin =  8, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 14, .scu_group = 4, .scu_pin =  9, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 15, .scu_group = 4, .scu_pin = 10, .pull_resistors = SCU_NO_PULL},
};

/**
 * Definition of a set of logic analyzer functions.
 */
static sgpio_function_t logic_analyzer_functions[] = {
	{
		.enabled                 = true,

		// We're observing only; and not generating a pattern.
		.mode                    = SGPIO_MODE_STREAM_DATA_IN,

		// Bind each of the lower eight pins to their proper places,
		// and by deafault sample the eight of them.
		.pin_configurations      = logic_analyzer_pins,
		.bus_width               = LOGIC_ANALYZER_DEFAULT_WIDTH,

		// Shift at the default logic analyzer frequency, unless configured otherwise.
		.shift_clock_source      = SGPIO_CLOCK_SOURCE_COUNTER,
		.shift_clock_frequency   = LOGIC_ANALYZER_DEFAULT_FREQUENCY,
		.shift_clock_qualifier   = SGPIO_ALWAYS_SHIFT_ON_SHIFT_CLOCK,

		// Capture our data into the USB bulk buffer, all ready to be sent up to the host.
		.buffer                  = usb_bulk_buffer,
		.buffer_order            = 15, // 16384 * 2 (LOGIC_ANALYZER_NUM_BUFFERS * LOGIC_ANALYZER_BUFFER_SIZE)
	},
};

/**
 * Logic analyzer configuration using SGPIO.
 */
static sgpio_t analyzer  = {
	.functions      = logic_analyzer_functions,
	.function_count = ARRAY_SIZE(logic_analyzer_functions),
};


static int verb_configure(struct command_transaction *trans)
{
	uint32_t desired_sample_rate = comms_argument_parse_uint32_t(trans);
	uint8_t  desired_bus_width   = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	// Set up the shape of our logic analyzer capture...
	logic_analyzer_functions[0].shift_clock_frequency = desired_sample_rate;
	logic_analyzer_functions[0].bus_width = desired_bus_width;

	// ... and put us in capture mode.
	sgpio_set_up_functions(&analyzer);

	// Respond with the parameters of our sampling.
	comms_response_add_uint32_t(trans, logic_analyzer_functions[0].shift_clock_frequency);
	comms_response_add_uint32_t(trans, USB_STREAMING_BUFFER_SIZE);
	comms_response_add_uint8_t(trans,  USB_STREAMING_IN_ADDRESS);

	// And ensure we service the logic analyzer task routine.
	return 0;
}


static int verb_start(struct command_transaction *trans)
{
	(void)trans;

	// Set up our USB buffer for acquisition...
	logic_analyzer_functions[0].position_in_buffer = 0;
	logic_analyzer_functions[0].data_in_buffer     = 0;


	// ... and start a capture.
	usb_streaming_start_streaming_to_host(
		&logic_analyzer_functions[0].position_in_buffer, &logic_analyzer_functions->data_in_buffer);
	sgpio_run(&analyzer);
	return 0;
}



static int verb_stop(struct command_transaction *trans)
{
	(void)trans;

	// Disable our stream-to-host, and disable the SGPIO capture.
	usb_streaming_stop_streaming_to_host();
	sgpio_halt(&analyzer);

	// .. and disable the pipe we use to transmit samples.
	return 0;
}


static int verb_dump_sgpio_config(struct command_transaction *trans)
{
	bool include_unused = comms_argument_parse_bool(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	sgpio_dump_configuration(LOGLEVEL_INFO, &analyzer, include_unused);
	return 0;
}


static int verb_change_first_pin(struct command_transaction *trans)
{
	uint8_t new_first_pin = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	// Validate that the new first pin is within our range.
	if (new_first_pin > 15) {
		pr_error("logic analyzer: tried to select an invalid first pin (SGPIO%d),"
				" which doesn't exist\n", new_first_pin);
		return EINVAL;
	}

	if (new_first_pin >= 8) {
		use_bank_b = true;
	} else {
		use_bank_b = false;
	}

	logic_analyzer_functions[0].pin_configurations = &logic_analyzer_pins[new_first_pin];
	return 0;
}


static int verb_use_alt_mappings(struct command_transaction *trans)
{
	bool use_rhoda_mappings = comms_argument_parse_bool(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	if (use_rhoda_mappings) {

		// SGPIO8 = J1_21 / P1_12
		logic_analyzer_pins[8].scu_group = 1;
		logic_analyzer_pins[8].scu_pin   = 12;

		// SGPIO8 = J2_9 / P4_3
		logic_analyzer_pins[9].scu_group = 4;
		logic_analyzer_pins[9].scu_pin   = 3;
	} else {
		// SGPIO8 = J1_21 / P1_12
		logic_analyzer_pins[8].scu_group = 9;
		logic_analyzer_pins[8].scu_pin   = 6;

		// SGPIO8 = J2_9 / P4_3
		logic_analyzer_pins[9].scu_group = 4;
		logic_analyzer_pins[9].scu_pin   = 3;
	}

	return 0;
}



static struct comms_verb logic_analyzer_verbs[] = {

	/* Configuration. */
	{ .name = "configure", .handler = verb_configure,
		.in_signature = "<IB", .out_signature = "<IIB",
		.in_param_names = "sample_rate_hz, num_channels", .out_param_names = "sample_rate_hz, buffer_size, endpoint",
		.doc = "Configures a logic analyzer capture; should be called before calling start." },
	{ .name = "change_first_pin", .handler = verb_change_first_pin,
		.in_signature = "<B", .out_signature = "",
		.in_param_names = "new_first_pin",
		.doc = "Changes the first SGPIO pin captured in future captures, but can "
			   "significantly diminish performance; you likely don't want any value but 0 or 8." },

	/* Hackish config. */
	{ .name = "configure_alt_mappings", .handler = verb_use_alt_mappings,
		.in_signature = "<?", .out_signature = "",
		.in_param_names = "use_usb_mappings",
		.doc = "Swaps the locations of pins 8/9 for use with Rhodadendron's USB-FS analyzer." },

	/* Actions */
	{ .name = "start", .handler = verb_start,
		.in_signature = "", .out_signature = "", .doc = "Starts a logic analyzer capture, which will run until stop is called." },
	{ .name = "stop", .handler = verb_stop,
		.in_signature = "", .out_signature = "", .doc = "Terminates an active logic analyzer capture." },


	/* Debug. */
	{ .name = "dump_sgpio_configuration",  .handler = verb_dump_sgpio_config,
		.in_signature = "<?", .out_signature="", .in_param_names = "include_unused",
		.doc = "Requests that the system dumps its SGPIO configuration state to the debug ring." },

	{}
};
COMMS_DEFINE_SIMPLE_CLASS(logic_analyzer, CLASS_NUMBER_SELF, "logic_analyzer", logic_analyzer_verbs,
		"Controls the logic analyzer function using the SGPIO peripheral");
