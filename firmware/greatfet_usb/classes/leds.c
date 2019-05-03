/*
 * This file is part of GreatFET
 */

#include <greatfet_core.h>
#include <drivers/comms.h>
#include <errno.h>

#define CLASS_NUMBER_SELF (0x10A)

static int leds_verb_toggle(struct command_transaction *trans)
{
	uint8_t led_num = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	led_toggle(led_num - 1);

	return 0;
}

static int leds_verb_on(struct command_transaction *trans)
{
	uint8_t led_num = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	led_on(led_num - 1);

	return 0;
}

static int leds_verb_off(struct command_transaction *trans)
{
	uint8_t led_num = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	led_off(led_num - 1);

	return 0;
}

/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "toggle", .handler = leds_verb_toggle,
			.in_signature = "<B", .out_signature = "",
			.in_param_names = "led_num",
			.doc = "Toggle an LED" },
		{ .name = "on", .handler = leds_verb_on,
			.in_signature = "<B", .out_signature = "",
			.in_param_names = "led_num",
			.doc = "Turn an LED on" },
		{ .name = "off", .handler = leds_verb_off,
			.in_signature = "<B", .out_signature = "",
			.in_param_names = "led_num",
			.doc = "Turn an LED off" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(leds, CLASS_NUMBER_SELF, "leds", _verbs,
		"API for LED configuration.");
