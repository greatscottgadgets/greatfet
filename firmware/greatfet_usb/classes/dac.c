/*
 * This file is part of GreatFET
 *
 * DAC functions
 */

#include <toolchain.h>
#include <errno.h>

#include <drivers/comms.h>
#include <drivers/dac.h>

#define CLASS_NUMBER_SELF (0x105)


static dac_t dac;

int dac_verb_initialize(struct command_transaction *trans)
{
	(void) trans;

	dac_init(&dac);

	return 0;
}


static int dac_verb_set_value(struct command_transaction *trans)
{
	uint16_t value = comms_argument_parse_uint16_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	// Set the DAC to that value.
	dac_set_value(&dac, value);

	return 0;
}


static int dac_verb_set_voltage(struct command_transaction *trans)
{
	uint16_t millivolts = comms_argument_parse_int16_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	// voltage = (value / 1024) * VDDA
	// value = (voltage * 1024) / VDDA

	// TODO: Measure the bandgap via ADC1 channel 7 instead of assuming 3.3 volts.
	uint16_t value = ((millivolts * 1024) / 3.3) / 1000;

	dac_set_value(&dac, value);

	return 0;
}


/**
 * Verbs for the DAC API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "initialize", .handler = dac_verb_initialize,
			.in_signature = "", .out_signature = "",
			.doc = "Initializes the DAC driver. Should be called before anything else."
		},
		{ .name = "set_value", .handler = dac_verb_set_value,
			.in_signature = "<H", .out_signature = "",
			.in_param_names = "value",
			.doc =
				"Sets the DAC value.\n"
				"\n"
				"The resultant voltage = (value / 1024) * VDDA.\n"
				"\n"
				"Parameters:\n"
				"    value -- The value to set the DAC to."
		},
		{ .name = "set_voltage", .handler = dac_verb_set_voltage,
			.in_signature = "<H", .out_signature = "",
			.in_param_names = "voltage_millivolts",
			.doc =
				"Sets the DAC value by voltage in millivolts.\n"
				"\n"
				"Parameters:\n"
				"    voltage_millivolts -- The desired output voltage (in millivolts)."
		},
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(dac, CLASS_NUMBER_SELF, "dac", _verbs,
		"API for simple DAC manipulations.");
