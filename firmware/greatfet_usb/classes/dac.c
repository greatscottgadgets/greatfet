/*
 * This file is part of GreatFET
 *
 * DAC functions
 */

#include <drivers/comms.h>
#include <stddef.h>
#include <greatfet_core.h>
#include <dac.h>
#include <pins.h>

#define CLASS_NUMBER_SELF (0x105)

int dac_verb_set(struct command_transaction *trans)
{
	// set J2_P5 up as a DAC pin
	scu_pinmux(SCU_PINMUX_GPIO2_3, SCU_GPIO_FAST | SCU_GPIO_PUP | SCU_CONF_FUNCTION0);
	static struct gpio_t dac_pin = GPIO(2, 3);
	gpio_input(&dac_pin); 

	uint16_t value = comms_argument_parse_uint16_t(trans);
	DAC_CR = DAC_CR_VALUE(value) & DAC_CR_VALUE_MASK;
	DAC_CTRL = DAC_CTRL_DMA_ENA;

	return 0;
}

/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "set", .handler = dac_verb_set,
            .in_signature = "<I", .out_signature = "",
            .in_param_names = "value",
            .doc = "Sets the DAC" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(dac, CLASS_NUMBER_SELF, "dac", _verbs,
		"API for simple DAC manipulations.");