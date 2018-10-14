/*
 * This file is part of libgreat.
 * This is the 'core' extension for firmware updates.
 *
 * Most devices will need to implement their own versions of these functions.
 */

#include <drivers/comms.h>
#include <debug.h>

#include <stddef.h>
#include <errno.h>


#include <greatfet_core.h>
#include <spiflash.h>
#include <spiflash_target.h>
#include <gpio_lpc.h>


#define CLASS_NUMBER_FIRMWARE (0x1)

/**
 * Initializes the firmware update subsystem of the target device.
 * Accepts no arguments.
 *
 * Returns:
 *  - a uint32_t that indicates the device's page size in bytes.
 *  - a uint32_t that indicates the device's total size, in bytes
 */
WEAK int firmware_verb_initialize(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}


/**
 * Issues a full erase of the target device's firmware.
 *
 * Accepts no arguments.
 */
WEAK int firmware_verb_full_erase(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}

/**
 * Erases a page of the device's firmare.
 *
 * Accepts a uint32_t that indicates the address.
 */
WEAK int firmware_verb_erase_page(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}


/**
 * Writes a page to the device's firmware.
 *
 * Accepts a uint32_t that is the address; followed by a single page of data.
 */
WEAK int firmware_verb_write_page(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}


/**
 * Reads a page from the device's firmare.
 *
 * Accepts a uint32_t that is the address.
 * Returns the relevant page.
 */
WEAK int firmware_verb_read_page(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}


/**
 * Verbs for the firmware API.
 */
static struct comms_verb firmware_verbs[] = {
		{ .verb_number = 0x0, .name = "initialize", .handler = firmware_verb_initialize,
            .in_signature = "", .out_signature = "<II", .out_param_names = "page_size, total_size",
            .doc = "Sets up the board to have its firmware programmed." },
		{ .verb_number = 0x1, .name = "full_erase", .handler = firmware_verb_full_erase,
            .in_signature = "", .out_signature	= "", .doc = "Erases the entire firmware flash chip." },
		{ .verb_number = 0x2, .name = "page_erase", .handler = firmware_verb_erase_page,
            .in_signature = "<I", .out_signature = "", .in_param_names = "address",
            .doc = "Erases the page with the provided address on the fw flash." },
		{ .verb_number = 0x3, .name = "write_page", .handler = firmware_verb_write_page,
            .in_signature = "<I*X", .out_signature = "", .in_param_names = "address, data",
            .doc = "Writes the provided data to a single firmware flash page." },
		{ .verb_number = 0x4, .name = "read_page",	.handler = firmware_verb_read_page,
            .in_signature = "<I", .out_signature = "<*X", .in_param_names = "address", .out_param_names = "data",
            .doc = "Returns the contents of the flash page at the given address." },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(firmware_api, CLASS_NUMBER_FIRMWARE, "firmware", firmware_verbs,
        "Common API for updating firmware on a libgreat device.");
