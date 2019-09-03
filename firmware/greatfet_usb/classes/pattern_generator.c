/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>

#include "../sgpio_isr.h"

#include <drivers/sgpio.h>
#include <drivers/usb/usb.h>
#include <drivers/gpio.h>
#include <drivers/usb/usb_queue.h>

#include "../usb_bulk_buffer.h"
#include "../usb_endpoint.h"

#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/cm3/vector.h>

#include <drivers/platform_clock.h>

#include <errno.h>
#include <debug.h>
#include <string.h>
#include <toolchain.h>

#define CLASS_NUMBER_SELF (0x110)
#define GENERATOR_TOTAL_BUFFER_SIZE (GENERATOR_NUM_BUFFERS * GENERATOR_BUFFER_SIZE)

enum {
	GENERATOR_NUM_BUFFERS              = 2,
	GENERATOR_BUFFER_SIZE              = 0x4000,
	GENERATOR_BUFFER_ORDER             = 15,
};

volatile bool pattern_generator_enabled = false;

// Set the default frequency for our logic analyzer.
#define PATTERN_GENERATOR_DEFAULT_FREQUENCY (1 * 1000000)
#define PATTERN_GENERATOR_DEFAULT_WIDTH     (8)

static bool use_bank_b = false;

/**
 * Default set of pin mappings for each of the logic analyzer functions.
 */
static sgpio_pin_configuration_t pattern_generator_pins[] = {
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
static sgpio_function_t pattern_generator_functions[] =
{
	{
		.enabled                 = true,

		// Generate a fixed pattern, by default.
		.mode                    = SGPIO_MODE_FIXED_DATA_OUT,

		// Bind each of the lower eight pins to their proper places,
		// and by deafault sample the eight of them.
		.pin_configurations      = pattern_generator_pins,
		.bus_width               = PATTERN_GENERATOR_DEFAULT_WIDTH,

		// Shift at the default frequency, unless configured otherwise.
		.shift_clock_source      = SGPIO_CLOCK_SOURCE_COUNTER,
		.shift_clock_frequency   = PATTERN_GENERATOR_DEFAULT_FREQUENCY,
		.shift_clock_qualifier   = SGPIO_ALWAYS_SHIFT_ON_SHIFT_CLOCK,

		// Use the USB bulk buffer as our stream buffer.
		.buffer                  = usb_bulk_buffer,
		.buffer_order            = GENERATOR_BUFFER_ORDER,
	},
};

/**
 * Logic analyzer configuration using SGPIO.
 */
static sgpio_t generator  = {
	.functions      = pattern_generator_functions,
	.function_count = ARRAY_SIZE(pattern_generator_functions),
};


/**
 * Configures the SGPIO hardware to transmit a fixed pattern repeatedly via SGPIO.
 * Small transfers (up to 64-packed-bytes per eight bits) can be done CPU-less-ly; while larger transfers
 * use an interrupt to periodically load new data into the registers.
 *
 * @param sample_rate The sample rate of the samples to be shifted out, in samples per second.
 * @param bus_width The number of bits to simultaneously shift out. Currently can be 1, 2, 4, or 8.
 * @param data A pointer to the data to be shifted out, or NULL if the data's already been loaded into our buffer.
 * @param data_length The length of the pattern to be shifted out. Must be a binary number for rapid repeating.
 */
static int start_fixed_pattern_generation(uint32_t sample_rate, uint8_t bus_width, void *data,
		uint32_t data_length, bool repeat)
{
	int rc;

	// Validate that we have enough buffer to store the fixed pattern. If we don't, bail out.
	if (data_length > (GENERATOR_BUFFER_SIZE * GENERATOR_NUM_BUFFERS)) {
		pr_error("pattern gen: cannot output a %uB fixed pattern; you may want to stream data instead\n", data_length);
		return EINVAL;
	}

	// If we're not binary sized, we can't easily stream out the relevant data. Report that we'll truncate.
	if (!is_binary_sized(data_length)) {
		pr_warning("pattern gen: warning: a non-binary-sized pattern set was provided; data will be truncated!\n");
	}

	// Copy the target data into our pattern buffer, if necessary...
	if (data) {
		memcpy(pattern_generator_functions[0].buffer, data, data_length);
	}

	// ... set up the buffer bounds...
	pattern_generator_functions[0].buffer_order          = size_to_order(data_length);
	pattern_generator_functions[0].position_in_buffer    = 0;

	// ...  set up the sample rate and bus width...
	pattern_generator_functions[0].shift_clock_frequency = sample_rate;
	pattern_generator_functions[0].bus_width             = bus_width;

	// ... configure if we're repeating or one-shot'ing ...
	if (repeat) {
		pattern_generator_functions[0].shift_count_limit = 0;
		pr_info("pattern gen: not limiting runtime\n");
	} else {
		pattern_generator_functions[0].shift_count_limit = (data_length * 8) / bus_width;
		pr_info("pattern gen: limiting runtime to %d shifts!\n", pattern_generator_functions[0].shift_count_limit);
	}

	// ... and configure the SGPIO hardware to issue our pattern.
	pattern_generator_functions[0].mode = SGPIO_MODE_FIXED_DATA_OUT;
	rc = sgpio_set_up_functions(&generator);
	if (rc) {
		return rc;
	}

	// Finally, start the SGPIO streaming for the relevant buffer.
	sgpio_run(&generator);
	return 0;
}


static int verb_generate_simple_pattern(struct command_transaction *trans)
{
	void *data;
	uint32_t total_data;

	uint32_t sample_rate = comms_argument_parse_uint32_t(trans);
	uint8_t  bus_width   = comms_argument_parse_uint8_t(trans);
	bool     repeat      = comms_argument_parse_bool(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	// Grab the data to be transmitted...
	data = comms_argument_read_buffer(trans, GENERATOR_TOTAL_BUFFER_SIZE, &total_data);
	if (!data) {
		pr_error("pattern gen: error: could not read the pattern data to be transmitted!\n");
		return EINVAL;
	}

	// ... and set up a simple pattern.
	return start_fixed_pattern_generation(sample_rate, bus_width, data, total_data, repeat);
}


static int verb_upload_samples(struct command_transaction *trans)
{
	void *samples;
	uint32_t data_length;
	uintptr_t offset_into_buffer = comms_argument_parse_uint32_t(trans);
	uintptr_t target_address;

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	// Grab the data to be transmitted...
	samples = comms_argument_read_buffer(trans, GENERATOR_TOTAL_BUFFER_SIZE, &data_length);
	if (!samples) {
		pr_error("pattern gen: error: could not read the pattern data to be transmitted!\n");
		return EINVAL;
	}

	// ... validate that it'll fit in our buffer...
	if ((offset_into_buffer + data_length) > GENERATOR_TOTAL_BUFFER_SIZE) {
		pr_error("pattern gen: error: tried to write past the end of the sample buffer!\n");
		return EINVAL;
	}

	// Generate an address of the relevant position in the buffer...
	target_address = offset_into_buffer + (uintptr_t)pattern_generator_functions[0].buffer;

	// ... and perform the copy itself.
	memcpy((void *)target_address, samples, data_length);
	pr_info("pattern gen: uploaded %u samples to offset %lu (addr %p)\n", data_length, offset_into_buffer, (void *)target_address);


	return 0;
}


static int verb_generate_pattern(struct command_transaction *trans)
{
	uint32_t sample_rate = comms_argument_parse_uint32_t(trans);
	uint8_t  bus_width   = comms_argument_parse_uint8_t(trans);
	uint32_t total_data  = comms_argument_parse_uint32_t(trans);
	bool     repeat      = comms_argument_parse_bool(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	// Generate the pattern directly from our USB buffer.
	return start_fixed_pattern_generation(sample_rate, bus_width, NULL, total_data, repeat);
}


static int verb_stop(struct command_transaction *trans)
{
	(void)trans;

	// Stop emission of the patterns...
	pattern_generator_enabled = false;

	// Disable the USB endpoint. TODO: use the pipe API!
	//usb_endpoint_disable(&usb0_endpoint_bulk_out);

	// ... disable the shifting hardware ...
	nvic_disable_irq(NVIC_SGPIO_IRQ);
	sgpio_halt(&generator);

	// .. and disable the pipe we use to transmit samples.
	return 0;
}


static int verb_dump_sgpio_config(struct command_transaction *trans)
{
	bool include_unused = comms_argument_parse_bool(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	sgpio_dump_configuration(LOGLEVEL_INFO, &generator, include_unused);
	return 0;
}


static struct comms_verb pattern_generator_verbs[] = {

	/* Core control verbs. */
	{ .name = "stop", .handler = verb_stop,
		.in_signature = "", .out_signature = "", .doc = "Stops all pattern generation functionality." },


	/* Shortcut for short patterns. */
	{ .name = "generate_simple_pattern", .handler = verb_generate_simple_pattern,
		.in_signature = "<IB*X?", .out_signature = "", .in_param_names = "sample_rate_hz, num_channels, data, repeat",
		.doc = "Sets the GreatFET to emit a short pattern.\n\n"
			"    sample_rate_hz -- The target sample rate, in Hz.\n"
			"    num_channels -- The number of channels to emit; up to 8.\n"
			"    data -- The packed data to emit. \n"
			"    repeat -- If set, the pattern will be emitted repeatedly. The pattern must be sized to a binary number of bytes." },

	/* Upload and scan-out longer patterns. */
	{ .name = "upload_samples", .handler = verb_upload_samples,
		.in_signature = "<I*X", .out_signature = "", .in_param_names = "offset_into_buffer, samples",
		.doc = "Uploads a set of samples into the pattern generator's sample buffer."
			"    offset_into_buffer -- The offset into the sample buffer, in bytes..\n"
			"    samples -- The packed samples to be uploaded; usually up to around 4096 samples.\n" },
	{ .name = "generate_pattern", .handler = verb_generate_pattern,
		.in_signature = "<IBI?", .out_signature = "", .in_param_names = "sample_rate_hz, num_channels, pattern_length",
		.doc = "Sets the GreatFET to repeatedly emit a short pattern.\n\n"
			"    sample_rate_hz -- The target sample rate, in Hz.\n"
			"    num_channels -- The number of channels to emit; up to 8.\n"
			"    pattern_length -- The length of the relevant pattern, in samples."
			"    repeat -- If set, the pattern will be emitted repeatedly. The pattern must be sized to a binary number of bytes." },


	/* Debug. */
	{ .name = "dump_sgpio_configuration",  .handler = verb_dump_sgpio_config,
		.in_signature = "<?", .out_signature="", .in_param_names = "include_unused",
		.doc = "Requests that the system dumps its SGPIO configuration state to the debug ring." },

	{}
};
COMMS_DEFINE_SIMPLE_CLASS(pattern_generator, CLASS_NUMBER_SELF, "pattern_generator", pattern_generator_verbs,
		"Generate patterns on the GreatFET's SGPIO pins.");
