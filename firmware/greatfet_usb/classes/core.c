/*
 * This file is part of GreatFET
 *
 * Core verb defititions -- these provide solid implementations
 * of the Core Verbs, as provided by libgreat.
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <config.h>
#include <greatfet_core.h>
#include <rom_iap.h>

#include <drivers/comms.h>
#include <drivers/reset.h>
#include <debug.h>

#define CLASS_NUMBER_CORE 0

char version_string[] = CONFIG_VERSION_STRING;

/**
 * Method that determines the board's ID.
 * Overrideable if derivative firmware wants to auto-detect which board it's running on.
 */
uint32_t ATTR_WEAK core_get_board_id()
{
	return CONFIG_BOARD_ID;
}


int core_verb_read_board_id(struct command_transaction *trans)
{
	comms_response_add_uint32_t(trans, core_get_board_id());
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

typedef enum  {

	//. Normal soft-reset. Resets everything.
	RESET_REQUEST_NORMAL = 0,

	// Request to switch to an external clock.
	RESET_REQUEST_SWITCH_TO_EXTCLOCK = 1,

	// Soft reset the entire device _minus_ the always-on power domain.
	RESET_REQUEST_MAINTAIN_ALWAYS_ON_DOMAIN = 2,

	// Normal reset request, but prints a message that we're rebooting into new firmware.
	RESET_REQUEST_POST_FIRMWARE_FLASH = 3,

} reset_request_commands_t;

/**
 * Arguments is a reset_request_commands_t.
 */
int core_verb_request_reset(struct command_transaction *trans)
{
	reset_request_commands_t reset_reason_command = comms_argument_parse_uint32_t(trans);
	reset_reason_t reset_reason;

	bool reset_always_on_domain = true;

	switch (reset_reason_command) {

		case RESET_REQUEST_SWITCH_TO_EXTCLOCK:
			pr_info("Performing soft reset and switching to external clock...\n");
			reset_reason = RESET_REASON_USE_EXTCLOCK;
			break;

		case RESET_REQUEST_MAINTAIN_ALWAYS_ON_DOMAIN:
			pr_info("Performing soft reset, but not resetting the always-on (RTC) power domain.\n");

			reset_reason = RESET_REASON_SOFT_RESET;
			reset_always_on_domain = false;
			break;

		case RESET_REQUEST_POST_FIRMWARE_FLASH:
			pr_info("Resetting device into newly-flashed firmware...\n");
			reset_reason = RESET_REASON_NEW_FIRMWARE;
			break;

		default:
			pr_info("Performing soft reset...\n");
			reset_reason = RESET_REASON_SOFT_RESET;
			break;
	}

	// Perform the actual reset. Should never return.
	system_reset(reset_reason, reset_always_on_domain);
	return 0;
}
