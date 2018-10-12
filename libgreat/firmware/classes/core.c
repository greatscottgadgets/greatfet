/*
 * This file is part of libgreat
 *
 * Core communications class -- provides 
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <toolchain.h>

#include <drivers/comms.h>
#include <drivers/comms_backend.h>

#define CLASS_NUMBER_CORE 0

extern struct comms_class *class_head;

WEAK int core_verb_read_board_id(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}


WEAK int core_verb_read_version_string(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}


WEAK int core_verb_read_part_id(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}


WEAK int core_verb_read_serial_number(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}

/**
 * TODO: get me out of here!
 */
WEAK int core_verb_request_reset(struct command_transaction *trans)
{
	(void)trans;
	return ENOSYS;
}


/**
 * Internal introspection command that returns the list of supported classes.
 */ 
static int verb_get_available_classes(struct command_transaction *trans)
{
	struct comms_class *cls;

	// Add each class number to the list.
	for (cls = class_head; cls; cls = cls->next) {
		comms_response_add_uint32_t(trans, cls->class_number);	
	}

	return 0;
}


/**
 * Internal introspection command that returns the list of supported classes.
 */ 
static int verb_get_verb_name(struct command_transaction *trans)
{
	uint32_t class_number = comms_argument_parse_uint32_t(trans);
	uint32_t verb_number = comms_argument_parse_uint32_t(trans);

	// Fetch the relevant class.
	struct comms_verb *verb = comms_get_object_for_verb(class_number, verb_number);

	// If we don't have one, abort.
	if (!verb)
		return EINVAL;

	// Respond with the verb's name, if we have one.
	if (!verb->name)
		comms_response_add_string(trans, "");
	else
		comms_response_add_string(trans, verb->name);

	return 0;
}


/**
 * Internal introspection command that returns the list of verbs for a given class.
 */ 
static int verb_get_available_verbs(struct command_transaction *trans)
{
	uint32_t class_number = comms_argument_parse_uint32_t(trans);
	struct comms_verb *verb;

	// Fetch the relevant class.
	struct comms_class *relevant_class = comms_get_class_by_number(class_number);

	// Iterate through the array of command verbs, adding them to our response.
	for (verb = relevant_class->command_verbs; verb->handler; ++verb) {
		comms_response_add_uint32_t(trans, verb->verb_number);	
	}

	return 0;
}


/**
 * Verbs for the core API.
 */
static struct comms_verb core_verbs[] = {
		{ .verb_number = 0x0, .name = "read_board_id", .handler = core_verb_read_board_id },
		{ .verb_number = 0x1, .name = "read_version_string", .handler = core_verb_read_version_string },
		{ .verb_number = 0x2, .name = "read_part_id", .handler = core_verb_read_part_id },
		{ .verb_number = 0x3, .name = "read_serial_number", .handler = core_verb_read_serial_number },
		{ .verb_number = 0x4, .name = "get_available_classes", .handler = verb_get_available_classes },
		{ .verb_number = 0x5, .name = "get_avaiable_verbs", .handler = verb_get_available_verbs },
		{ .verb_number = 0x6, .name = "get_verb_name", .handler = verb_get_verb_name },

		// TODO: move this out of core!
		{ .verb_number = 0x20, .handler = core_verb_request_reset },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(core_api, CLASS_NUMBER_CORE, "core", core_verbs);
