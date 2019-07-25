/*
 * This file is part of GreatFET
 */


#include <errno.h>
#include <debug.h>

#include <greatfet_core.h>
#include <drivers/comms.h>

// FIXME: replace with libgreat driver
#include <libopencm3/lpc43xx/adc.h>


#define CLASS_NUMBER_SELF (0x111)

//
// Author's note: this is just a quick port of @dominicgs's ADC driver.
// A full ADC driver in libgreat is forthcoming.
//


static void set_up_onboard_adc(bool use_adc1, uint32_t pin_mask, uint8_t significant_bits)
{
	// FIXME: for now, since we're not using a libgreat built-in ADC, we'll hardcode the ADC
	// clock divider to meet datasheet requirements
	const uint32_t clkdiv = 45;	// divide 204MHz clock by 45 to get ~4MHz

	// Compute the number of clock cycles required to capture the given number of siginficant bits.
	// Clocks
	uint32_t clks = 10 - significant_bits;

	// Compute the control register value...
	uint32_t cr_value = ADC_CR_SEL(pin_mask) |
			ADC_CR_CLKDIV(clkdiv) |
			ADC_CR_CLKS(clks) |
			ADC_CR_PDN;


	// ... and apply it.
	if (use_adc1) {
		ADC1_CR = cr_value;
	} else {
		ADC0_CR = cr_value;
	}
}



static int verb_read_sample(struct command_transaction *trans)
{
	uint16_t value;

	uint8_t adc_number       = comms_argument_parse_uint8_t(trans);
	uint8_t pin_number       = comms_argument_parse_uint8_t(trans);
	uint8_t significant_bits = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }


	// Sanity check our arguments.
	if (adc_number > 8) {
		pr_error("adc: invalid ADC number %" PRIu8 "provided (must be <= 1)!\n", adc_number);
		return EINVAL;
	}
	if (pin_number > 8) {
		pr_error("adc: invalid pin number %" PRIu8 "provided (must be <= 8)!\n", pin_number);
		return EINVAL;
	}
	if ((significant_bits < 3) || (significant_bits > 10)) {
		pr_error("adc: invalid pin number %" PRIu8 "provided (must be between 3 and 10, inclusive)!\n", significant_bits);
		return EINVAL;
	}

	// Configure the primary ADC to capture the relevant sample.
	set_up_onboard_adc(adc_number, 1 << pin_number, significant_bits);

	// Start a conversion, and then wait for it to complete.
	ADC0_CR |= ADC_CR_START(1);
	while(!(ADC0_DR0 & ADC_DR_DONE));

	// Read the ADC value, and send it
	value = (ADC0_DR0 >> 6) & 0x3ff;
	comms_response_add_uint16_t(trans, value);

	return 0;
}


/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "read_sample", .handler = verb_read_sample,
			.in_signature = "<BBB", .out_signature = "<H",
			.in_param_names = "adc_number, pin_number, significant_bits",
			.out_param_names = "adc_sample",
			.doc =
				"Read a single sample from the ADC.\n"
				"\n"
				"Params:\n"
				"    adc_number       -- which ADC to use to do the reading (should be 0 or 1)\n"
				"    pin_number       -- the ADC pin to read using the given ADC\n"
				"    significant_bits -- the total significant bits to capture: (from 3-10).\n"
				"                     -- fewer bits will result in a faster sample"
			},
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(adc, CLASS_NUMBER_SELF, "adc", _verbs,
		"read from the GF's on-board ADC");
