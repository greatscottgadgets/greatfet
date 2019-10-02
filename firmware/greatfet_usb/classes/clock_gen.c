
/*
 * This file is part of GreatFET
 *
 * Clock output control.
 */

#include <debug.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#include <drivers/comms.h>
#include <drivers/platform_clock.h>
#include <drivers/scu.h>

#define CLASS_NUMBER_SELF (0x116)


static int _verb_output_clock(struct command_transaction *trans)
{
	platform_scu_registers_t *scu = platform_get_scu_registers();
	platform_clock_generation_register_block_t *cgu = get_platform_clock_generation_registers();

	// Select a configuration to be applied to the relevant CLK pin.
	platform_scu_pin_configuration_t clk_pin_config = {
		.function = 1, // setting up CLKOUT is always on function 1
		.pull_resistors = RESISTOR_CONFIG_NO_PULL,
		.input_buffer_enabled = 0,
		.use_fast_slew = 1,
		.disable_glitch_filter = 1,
	};

	// Read the clock number and source.
	uint8_t clock_number = comms_argument_parse_uint8_t(trans);
	uint8_t clock_source = comms_argument_parse_uint8_t(trans);

    if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	// If the user requested an invalid clock pin, error out.
	if (clock_number > 3) {
		pr_error("error: clock_gen: platform has four clockouts (0-3); requested %u\n", clock_number);
		return EINVAL;
	}

	// Set the source (and enable) the system's generic clock out...
	platform_enable_base_clock(&cgu->out);
	platform_select_base_clock_source(&cgu->out, clock_source);

	// ... and then route it to the appropriate pin.
	scu->clk[clock_number] = clk_pin_config;

	return 0;
}


/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {

		// Clock control.
		{ .name = "output_clock", .handler = _verb_output_clock, .in_signature = "<BB",
		   .out_signature = "", .in_param_names = "clock_pin, source",
           .doc = "Provide the target clock on a given CLKOUT pin," },

		// Every verb list ends with an empty entry. This acts like a null terminator
		// for our list of verbs.
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(clock_gen, CLASS_NUMBER_SELF, "clock_gen", _verbs,
        "API for controlling clock generation.");

