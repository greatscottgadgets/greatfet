/*
 * This file is part of GreatFET
 */

#include <debug.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include <drivers/comms.h>
#include <drivers/platform_reset.h>

#define CLASS_NUMBER_SELF (0x115)

extern uint8_t _m0_data_region[];
extern uint32_t _m0_data_region_size;


static int verb_load_m0_page(struct command_transaction *trans)
{
	uint32_t data_length;
	uint32_t offset      = comms_argument_parse_uint32_t(trans);
	void     *data       = comms_argument_read_buffer(trans, -1, &data_length);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// Validate that the loaded data would fit in our buffer.
	if ((offset + data_length) > _m0_data_region_size) {
		pr_error("loadable: error: data would extend beyond loadable region!\n");
		return EINVAL;
	}

	// Copy the data.
	memcpy(&_m0_data_region[offset], data, data_length);

	// And return the total amount copied.
	pr_info("wrote %d bytes to %p\n", data_length, &_m0_data_region[offset]);
	comms_response_add_int32_t(trans, data_length);
	return 0;
}


static int verb_start_m0(struct command_transaction *trans)
{
	(void)trans;
	pr_info("M0 core started.\n");
	platform_start_m0_core(&_m0_data_region);

	return 0;
}


static int verb_halt_m0(struct command_transaction *trans)
{
	(void)trans;

	pr_info("M0 core halted.\n");
	platform_halt_m0_core();

	return 0;
}


/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {

		// Data preparation.
		{ .name = "load_m0_page", .handler = verb_load_m0_page, .in_signature = "<I*X",
			.out_signature = "<I", .in_param_names = "offset_bytes, data", .out_param_names = "total_copied",
			.doc =
				"Copies a page of data into the M0 address space.\n"
				"\n"
				"Should be run with the M0 halted. " },

		// Control.
		{ .name = "start_m0", .handler = verb_start_m0, .in_signature = "", .out_signature = "",
			.doc = "Starts execution of a loaded program on the device's M0 core.\n" },
		{ .name = "halt_m0", .handler = verb_halt_m0, .in_signature = "", .out_signature = "",
			.doc = "Starts execution of a loaded program on the device's M0 core.\n" },


		// Sentinel.
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(loadables, CLASS_NUMBER_SELF, "loadables", _verbs,
        "API that allows loading small user programs to run as part of the device firmware");

