/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include <glitchkit.h>
#include <debug.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#define CLASS_NUMBER_SELF (0x106)

/**
 * TODO: document adding a new class here
 */


static int glitchkit_verb_set_synchronization_events(struct command_transaction *trans)
{
	glitchkit_event_t event_mask = comms_argument_parse_uint32_t(trans);

    if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	glitchkit_use_event_for_synchronization(event_mask);
	return 0;
}


static int glitchkit_verb_set_trigger_events(struct command_transaction *trans)
{
	glitchkit_event_t event_mask = comms_argument_parse_uint32_t(trans);

    if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	// Disable all events, and then set our new events.
	glitchkit_disable_trigger_on(~0);
	glitchkit_enable_trigger_on(event_mask);
	return 0;
}


static int glitchkit_verb_add_trigger_events(struct command_transaction *trans)
{
	glitchkit_event_t event_mask = comms_argument_parse_uint32_t(trans);

    if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	glitchkit_enable_trigger_on(event_mask);
	return 0;
}


static int glitchkit_verb_provide_target_clock(struct command_transaction *trans)
{
	uint32_t clock_source = comms_argument_parse_uint32_t(trans);
	glitchkit_event_t requirements = comms_argument_parse_uint32_t(trans);

    if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	glitchkit_provide_target_clock(clock_source, requirements);
	return 0;
}


static struct comms_verb _verbs[] = {

        /* Configuration. */
		{  .name = "set_synchronization_events", .handler = glitchkit_verb_set_synchronization_events, .in_signature = "<I",
		   .out_signature = "", .in_param_names = "event_mask",
		   .doc = "Sets a collection of events to be used to synchronize stimuli." },
		{  .name = "set_trigger_events", .handler = glitchkit_verb_set_trigger_events, .in_signature = "<I",
		   .out_signature = "", .in_param_names = "event_mask",
		   .doc = "Sets a collection of events to be used to trigger fault injection." },
		{  .name = "add_trigger_events", .handler = glitchkit_verb_add_trigger_events, .in_signature = "<I",
		   .out_signature = "", .in_param_names = "event_mask",
		   .doc = "Adds to the active collection of events to be used to trigger fault injection." },
		{  .name = "provide_target_clock", .handler = glitchkit_verb_provide_target_clock, .in_signature = "<II",
		   .out_signature = "", .in_param_names = "clock_source, event_mask",
		   .doc = "Sets up the board to provide a clock to a target device." },

		/* TODO: simple triggers */

        /* Sentinel. */
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(glitchkit, CLASS_NUMBER_SELF, "glitchkit", _verbs,
		"tools for fault injection");

