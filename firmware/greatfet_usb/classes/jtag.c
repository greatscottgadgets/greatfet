/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <greatfet_core.h>
#include <jtag.h>
#include <errno.h>
#include <drivers/comms.h>

#define CLASS_NUMBER_SELF (0x10B)

static int jtag_verb_setup(struct command_transaction *trans)
{
    (void)trans;
    jtag_setup();
	return 0;
}

static int jtag_verb_stop(struct command_transaction *trans)
{
    (void)trans;
    jtag_stop();
	return 0;
}

static int jtag_verb_ir_shift(struct command_transaction *trans)
{
    uint8_t in = comms_argument_parse_uint8_t(trans);
    comms_response_add_uint8_t(trans, jtag_ir_shift_8(in));
	return 0;
}

static int jtag_verb_dr_shift(struct command_transaction *trans)
{
    uint16_t in = comms_argument_parse_uint16_t(trans);
    comms_response_add_uint16_t(trans, jtag_dr_shift_16(in));
	return 0;
}

static int jtag_verb_reset_tap(struct command_transaction *trans)
{
    (void)trans;
    jtag_reset_tap();
	return 0;
}

static int jtag_verb_reset_target(struct command_transaction *trans)
{
    (void)trans;
	jtag_reset_tap();
	jtag_reset_target();
	return 0;
}

static int jtag_verb_detect_ir_width(struct command_transaction *trans)
{
	jtag_reset_tap();
    comms_response_add_uint16_t(trans, jtag_detect_ir_width());
	return 0;
}

static int jtag_verb_detect_chain_length(struct command_transaction *trans)
{
	jtag_reset_tap();
    comms_response_add_uint16_t(trans, jtag_detect_chain_length());
	return 0;
}

static int jtag_verb_get_device_id(struct command_transaction *trans)
{
    uint16_t chip = comms_argument_parse_uint16_t(trans);
	jtag_reset_tap();
    comms_response_add_uint32_t(trans, jtag_get_device_id(chip));
	return 0;
}

/*
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "setup", .handler = jtag_verb_setup,
			.in_signature = "", .out_signature = "",
			.doc = "" },
		{ .name = "stop", .handler = jtag_verb_stop,
			.in_signature = "", .out_signature = "",
			.doc = "" },
		{ .name = "ir_shift", .handler = jtag_verb_ir_shift,
			.in_signature = "<B", .out_signature = "<B",
			.in_param_names = "instruction", .out_param_names = "output",
			.doc = "" },
		{ .name = "dr_shift", .handler = jtag_verb_dr_shift,
			.in_signature = "<H", .out_signature = "<H",
			.in_param_names = "data", .out_param_names = "output",
			.doc = "" },
		{ .name = "reset_tap", .handler = jtag_verb_reset_tap,
			.in_signature = "", .out_signature = "",
			.doc = "" },
		{ .name = "reset_target", .handler = jtag_verb_reset_target,
			.in_signature = "", .out_signature = "",
			.doc = "Resets the target device" },
		{ .name = "detect_ir_width", .handler = jtag_verb_detect_ir_width,
			.in_signature = "", .out_signature = "<H",
			.out_param_names = "ir_width",
			.doc = "Detects the total bits in the IR register" },
		{ .name = "detect_chain_length", .handler = jtag_verb_detect_chain_length,
			.in_signature = "", .out_signature = "<H",
			.out_param_names = "chain_length",
			.doc = "Detects JTAG chain length" },
		{ .name = "get_device_id", .handler = jtag_verb_get_device_id,
			.in_signature = "<H", .out_signature = "<I",
			.in_param_names = "chip", .out_param_names = "device_id",
			.doc = "Gets the chip ID of the chip specified" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(jtag, CLASS_NUMBER_SELF, "jtag", _verbs,
                          "Functions for debugging over JTAG.");

