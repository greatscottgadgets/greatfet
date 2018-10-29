/*
 * This file is part of libgreat
 *
 * High-level communications API -- definition of device
 * class handlers; for use by frontends (command/pipe providers).
 */


#include <debug.h>
#include <errno.h>

#include <drivers/comms.h>
#include <drivers/comms_backend.h>

/** Head for the comms-class linked list. */
struct comms_class *class_head = NULL;


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

		pr_debug("dispatching a command via handler function for class %s\n", handling_class->name);
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

			pr_debug("dispatching command %s:%s()\n", handling_class->name, verb->name);
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
