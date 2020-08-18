/*
 * This file is part of GreatFET
 */

// The swra124 class implements the debug transport for debugging and
// programming cc111x, cc243x, and cc251x 8051-based integrated MCU and
// RF transceivers from Texas Instruments (Chipcon).

#include <debug.h>
#include <drivers/comms.h>
#include <stddef.h>
#include <stdint.h>
#include <swra124.h>

#define CLASS_NUMBER_SELF (0x114)

static int swra124_verb_setup()
{
	swra124_setup();
	return 0;
}

static int swra124_verb_debug_init()
{
	swra124_debug_init();
	return 0;
}

static int swra124_verb_chip_erase(struct command_transaction *trans)
{
	swra124_chip_erase();
	return 0;
}

static int swra124_verb_read_status(struct command_transaction *trans)
{
	comms_response_add_uint8_t(trans, swra124_read_status());
	return 0;
}

static int swra124_verb_get_chip_id(struct command_transaction *trans)
{
	comms_response_add_uint16_t(trans, swra124_get_chip_id());
	return 0;
}

static int swra124_verb_halt()
{
	swra124_halt();
	return 0;
}

static int swra124_verb_resume()
{
	swra124_resume();
	return 0;
}

static int swra124_verb_debug_instr(struct command_transaction *trans)
{
	size_t size = 0;
	uint8_t instr[SWRA124_MAX_INSTR_SIZE];
	while (comms_argument_data_remaining(trans) && size <= SWRA124_MAX_INSTR_SIZE) {
		instr[size++] = comms_argument_parse_uint8_t(trans);
	}
	if (comms_argument_data_remaining(trans) || size == 0) {
		pr_error("swra124: invalid instruction");
		return EINVAL;
	}
	comms_response_add_uint8_t(trans, swra124_debug_instr(instr, size));
	return 0;
}

static int swra124_verb_step_instr()
{
	swra124_step_instr();
	return 0;
}

static int swra124_verb_get_pc(struct command_transaction *trans)
{
	comms_response_add_uint16_t(trans, swra124_get_pc());
	return 0;
}

static struct comms_verb swra124_verbs[] =
{
	{
		.name = "setup",
		.handler = swra124_verb_setup,
		.in_signature = "",
		.out_signature = "",
		.doc = "initialize pin mapping for debugging",
	},
	{
		.name = "debug_init",
		.handler = swra124_verb_debug_init,
		.in_signature = "",
		.out_signature = "",
		.doc = "reset target into debugging mode",
	},
	{
		.name = "chip_erase",
		.handler = swra124_verb_chip_erase,
		.in_signature = "",
		.out_signature = "",
		.doc = "erase the chip",
	},
	{
		.name = "read_status",
		.handler = swra124_verb_read_status,
		.in_signature = "",
		.out_signature = "<B",
		.out_param_names = "status",
		.doc = "read status byte from target",
	},
	{
		.name = "get_chip_id",
		.handler = swra124_verb_get_chip_id,
		.in_signature = "",
		.out_signature = "<H",
		.out_param_names = "chip_id",
		.doc = "read chip ID from target",
	},
	{
		.name = "halt",
		.handler = swra124_verb_halt,
		.in_signature = "",
		.out_signature = "",
		.doc = "halt target execution",
	},
	{
		.name = "resume",
		.handler = swra124_verb_resume,
		.in_signature = "",
		.out_signature = "",
		.doc = "resume target execution",
	},
	{
		.name = "debug_instr",
		.handler = swra124_verb_debug_instr,
		.in_signature = "<*X",
		.out_signature = "<B",
		.out_param_names = "a_reg",
		.doc = "execute instruction on target",
	},
	{
		.name = "step_instr",
		.handler = swra124_verb_step_instr,
		.in_signature = "",
		.out_signature = "",
		.doc = "single-step target",
	},
	{
		.name = "get_pc",
		.handler = swra124_verb_get_pc,
		.in_signature = "",
		.out_signature = "<H",
		.out_param_names = "pc",
		.doc = "get program counter from target",
	},
	{},
};
COMMS_DEFINE_SIMPLE_CLASS(swra124, CLASS_NUMBER_SELF, "swra124", swra124_verbs,
		"cc1110/cc243x/cc251x debug transport");
