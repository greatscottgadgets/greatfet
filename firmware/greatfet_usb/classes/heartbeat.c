/*
 * This file is part of GreatFET
 */


#include <drivers/comms.h>
#include <debug.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#include <pins.h>

#define CLASS_NUMBER_HEARTBEAT (0x102)

static volatile bool heartbeat_mode_enabled = true;
static volatile uint32_t heartbeat_period = 2500000;


/**
 * Prepares the system to use heartbeat mode.
 */
void heartbeat_init(void)
{
	// FIXME: resevere the heartbeat LED for this class in the pin manager?
	led_on(HEARTBEAT_LED);
}



static int heartbeat_verb_stop(struct command_transaction *trans)
{
	(void)trans;

	heartbeat_mode_enabled = false;
	return 0;
}


static int heartbeat_verb_start(struct command_transaction *trans)
{
	(void)trans;

	heartbeat_mode_enabled = true;
	return 0;
}


static int heartbeat_verb_set_period(struct command_transaction *trans)
{
	uint32_t new_period = comms_argument_parse_uint32_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	heartbeat_period = new_period;
	return 0;
}


static int heartbeat_verb_get_period(struct command_transaction *trans)
{
	comms_response_add_uint32_t(trans, heartbeat_period);
	return 0;
}

static struct comms_verb heartbeat_verbs[] = {
	{ .verb_number = 0x0, .name = "stop", .handler = heartbeat_verb_stop,
		.in_signature ="", .out_signature = "", .doc = "Disables heartbeat mode, free'ing the LED for user use." },
	{ .verb_number = 0x1, .name = "start", .handler = heartbeat_verb_start,
		.in_signature ="", .out_signature = "", .doc = "Enables heartbeat mode, e.g. after the heartbeat has been stopped." },
	{ .verb_number = 0x2, .name = "set_period", .handler = heartbeat_verb_set_period, .in_signature = "<I",
		.out_signature = "", .in_param_names = "period", .doc = "Sets the base period for the hearbeat LED. Arbitary units." },
	{ .verb_number = 0x3, .name = "get_period", .handler = heartbeat_verb_get_period, .in_signature = "",
		.out_signature = "<I", .out_param_names = "period", .doc = "Returns the base period for the hearbeat LED. Arbitary units." },
	{}
};
COMMS_DEFINE_SIMPLE_CLASS(heartbeat, CLASS_NUMBER_HEARTBEAT, "heartbeat", heartbeat_verbs,
		"Controls the device's idle ('heartbeat') LED.");


/**
 * Performs a single unit of heartbeat mode's work.
 * This should be called repeatedly from the main loop.
 *
 * Note that we use an iteration count, rather than e.g. our ms timer,
 * as this gives the LED's period the nice property of being proportional
 * to the amount of work being done e.g. in interrupts.
 */
void service_heartbeat(void)
{
	static uint32_t iteration_count = 0;

	// If heartbeat mode is disabled, do nothing.
	if (!heartbeat_mode_enabled) {
		return;
	}

	// Count a heartbeat iteration.
	iteration_count++;

	// If we've exceeded our heartbeat period, flip the LED,
	// and start over.
	if (iteration_count > heartbeat_period) {
		led_toggle(HEARTBEAT_LED);
		iteration_count = 0;
	}
}

DEFINE_TASK(service_heartbeat);
