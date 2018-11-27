/*
 * This file is part of libgreat
 *
 * High-level communications API -- definition of device
 * class handlers; for use by frontends (command/pipe providers).
 */


#include <debug.h>
#include <errno.h>
#include <stdbool.h>

#include <drivers/comms.h>
#include <drivers/comms_backend.h>

/** Head for the comms-class linked list. */
struct comms_class *class_head = NULL;


/**
 * Determines whether a provided comms class requires verb-number auto-assignments.
 * A class with every verb with a verb number of zero will have all verbs auto-assigned
 * sequential numbers. (This doesn't affect classes with only a single verb with number 0).
 */
static bool comms_class_requires_verb_assignment(struct comms_class *comms_class)
{
	struct comms_verb *verb;

	// Iterate through the array of command verbs.
	for (verb = comms_class->command_verbs; verb->handler; ++verb) {

		// If this verb has a non-zero number, this isn't a
		// candidate for auto-assignment.
		if (verb->verb_number != 0) {
			return false;
		}
	}

	// If every verb has a number of zero, this class needs auto-assignment.
	return true;
}


static void comms_auto_assign_verb_numbers(struct comms_class *comms_class)
{
	struct comms_verb *verb;
	int next_verb_number = 0;

	// Iterate through the array of command verbs.
	for (verb = comms_class->command_verbs; verb->handler; ++verb) {
		verb->verb_number = next_verb_number;
		++next_verb_number;
	}
}



/**
 * Registers a given class for use with libgreat; which implicitly provides it
 * with an ability to handle commands.
 *
 * @param comms_class The comms class to be registered. This object will continue
 *	to be held indefinition, so it must be permanently allocated.
 */
void comms_register_class(struct comms_class *comms_class)
{
	// Sanity guard: error out if someone tries to register a null class.
	if (!comms_class) {
		pr_error("ERROR: tried to register a NULL class");
		return;
	}

	// Handle verb-number auto-assignment for any classes that need them.
	if (comms_class_requires_verb_assignment(comms_class)) {
		comms_auto_assign_verb_numbers(comms_class);
	}

	// Link the comms class into our linked list.
	comms_class->next = class_head;
	class_head = comms_class;
}


/**
 * @returns The comms_class object with the given number, or
 *		NULL if none exists.
 */
struct comms_class *comms_get_class_by_number(uint32_t class_number)
{
	struct comms_class *cls;

	// Search the linked list for a class with the given number.
	for (cls = class_head; cls; cls = cls->next) {
		if (cls->class_number == class_number) {
			return cls;
		}
	}

	return NULL;
}


/**
 * Returns a string describing the given class, or default_string
 * if the given class does not exist.
 */
const char *comms_get_class_name(uint32_t class_number, const char *default_string)
{
	struct comms_class *cls = comms_get_class_by_number(class_number);

	if (!cls || !cls->name) {
		return default_string;
	} else {
		return cls->name;	
	}
}


/**
 * Submits a command for execution. Used by command backends.
 *
 * @param backend The command backend driver submitting the given command.
 * @param trans An object representing the command to be submitted, and its
 *		response.
 */
int comms_backend_submit_command(struct comm_backend_driver *backend,
	struct command_transaction *trans)
{
	int rc = 0;
	bool found_handler = false;

	struct comms_verb *verb;
	struct comms_class *handling_class = comms_get_class_by_number(trans->class_number);


	// If we couldn't find a handling class.
	if (!handling_class) {
		pr_warning("warning: backend %s submitted a command for an unknown class %d (%x)\n",
				backend->name, trans->class_number, trans->class_number);
		return EINVAL;
	}

	// If the handling class has a command handler, use it!
	if (handling_class->command_handler) {
		found_handler = true;
		rc = handling_class->command_handler(trans);
	}

	// Otherwise, search for a verb handler for the given class.
	if (!handling_class->command_verbs) {
		pr_warning(
				"WARNING: backend %s submttied a command for class %d, which has neither\n"
				"a command handler nor verb handlers!\n",
				backend->name, handling_class->name);
		return EINVAL;
	}

	// Iterate through the array of command verbs until we find a verb
	// with a NULL handler.
	for (verb = handling_class->command_verbs; verb->handler; ++verb) {
		if (verb->verb_number == trans->verb) {
			found_handler = true;
			rc = verb->handler(trans);
			break;
		}
	}

	// If we couldn't find any handler, abort.
	if (!found_handler) {
		pr_warning("warning: backend %s submttied a command class %s with an unhandled verb %d / %x\n",
				backend->name, handling_class->name, trans->verb, trans->verb);
		return EINVAL;
	}

	// If we appear to have successfully handled the verb, but an error
	// occurred, grab its error code.
	if (!rc && !comms_transaction_okay(trans)) {
		rc = EBADMSG;
	}

	return rc;
}


/**
 * @returns the verb description for the given class and verb number
 */
struct comms_verb *comms_get_object_for_verb(uint32_t class_number, uint32_t verb_number)
{
	struct comms_verb *verb;
	struct comms_class *handling_class = comms_get_class_by_number(class_number);

	// If we couldn't find a handling class, return NULL.
	if (!handling_class) {
		return NULL;
	}

	// If the class has no verb descriptors, return NULL.
	if (!handling_class->command_verbs) {
		return NULL;
	}

	// Iterate through the array of command verbs until we find a verb
	// with a NULL handler.
	for (verb = handling_class->command_verbs; verb->handler; ++verb) {
		if (verb->verb_number == verb_number) {
			return verb;
		}
	}

	// If we couldn't find the relevant verb, return NULL.
	return NULL;
}
