/*
 * This file is part of GreatFET
 *
 * Core verb defititions -- these provide solid implementations
 * of the Core Verbs, as provided by libgreat.
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <greatfet_core.h>
#include <rom_iap.h>
#include <libopencm3/lpc43xx/wwdt.h>

#include <drivers/comms.h>
#include <debug.h>

#define CLASS_NUMBER_CORE 0

char version_string[] = VERSION_STRING;

int core_verb_read_board_id(struct command_transaction *trans)
{
	comms_response_add_uint32_t(trans, BOARD_ID);
	return 0;
}


int core_verb_read_version_string(struct command_transaction *trans)
{
	comms_response_add_string(trans, version_string);
	return 0;
}


int core_verb_read_part_id(struct command_transaction *trans)
{
	iap_cmd_res_t iap_cmd_res;

	// Don't allow a read if we can't fit a full response.
	if (trans->data_out_max_length < 8)
		return EINVAL;

	// Read the IAP part number...
	iap_cmd_res.cmd_param.command_code = IAP_CMD_READ_PART_ID_NO;
	iap_cmd_call(&iap_cmd_res);

	// ... and build our response from it.
	for (int i = 0; i < 2; ++i)
		comms_response_add_uint32_t(trans, iap_cmd_res.status_res.iap_result[i]);

	// Return whether our data is valid.
	return iap_cmd_res.status_res.status_ret;
}


int core_verb_read_serial_number(struct command_transaction *trans)
{
	iap_cmd_res_t iap_cmd_res;

	// Don't allow reads if we can't fit a full response.
	if (trans->data_out_max_length < 16)
		return EINVAL;

	// Read the board's serial number.
	iap_cmd_res.cmd_param.command_code = IAP_CMD_READ_SERIAL_NO;
	iap_cmd_call(&iap_cmd_res);

	// Add in each of the blocks of the serial number.
	for (int i = 0; i < 4; ++i)
		comms_response_add_uint32_t(trans, iap_cmd_res.status_res.iap_result[i]);

	return iap_cmd_res.status_res.status_ret;
}

/**
 * Arguments:
 *
 *		value = 0: regular reset
 *		value = 1: switch to an external clock after eset
 */
int core_verb_request_reset(struct command_transaction *trans)
{
	uint32_t reset_reason_command = comms_argument_parse_uint32_t(trans);

	if(reset_reason_command == 1) {
		pr_info("Performing soft reset and switching to external clock...\n");
		reset_reason = RESET_REASON_USE_EXTCLOCK;
	} else {
		pr_info("Performing soft reset...\n");
		reset_reason = RESET_REASON_SOFT_RESET;
	}

	wwdt_reset(100000);
	return 0;
}
