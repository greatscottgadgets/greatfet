/*
 * This file is part of GreatFET
 */


#include <toolchain.h>
#include <time.h>

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



static int verb_read_samples(struct command_transaction *trans)
{
	const uint32_t adc_sample_mask = 0x03FF;

	uint8_t  adc_number         = comms_argument_parse_uint8_t(trans);
	uint8_t  pin_channel_number = comms_argument_parse_uint8_t(trans);
	uint16_t sample_count       = comms_argument_parse_uint16_t(trans);

	volatile uint32_t *adc_cr;
	volatile uint32_t *adc_gdr;

	uint16_t sampled_value;

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}


	// TODO: support other amounts of significant bits by using burst mode.
	set_up_onboard_adc(adc_number == 1, 1 << pin_channel_number, 10);


	// Sanity checks.
	if (adc_number == 0) {
		adc_cr = &ADC0_CR;
		adc_gdr = &ADC0_GDR;
	}
	else if (adc_number == 1) {
		adc_cr = &ADC1_CR;
		adc_gdr = &ADC1_GDR;
	} else {
		pr_error("adc: invalid adc number %" PRIu8 " provided (must be <= 1)!\n", adc_number);
		return EINVAL;
	}

	if (pin_channel_number > 8) {
		pr_error("adc: error: invalid pin number %" PRIu8 " provided (must be <= 8)!\n", pin_channel_number);
		return EINVAL;
	}

	while (sample_count--) {

		uint32_t time_base = get_time();

		// Start a conversion.
		*adc_cr |= ADC_CR_START(1);

		// And wait for it to complete.
		while(!(*adc_gdr & ADC_DR_DONE) || (((*adc_gdr >> 24) & 0x7) != pin_channel_number)) {
			if (get_time_since(time_base) > 500) {
				return ETIMEDOUT;
			}
		}

		// Read the most recently sampled V_REF from the global data register.
		sampled_value = (*adc_gdr >> 6) & adc_sample_mask;

		comms_response_add_uint16_t(trans, sampled_value);
	}

	return 0;
}


/**
 * Verbs for the ADC API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "read_samples", .handler = verb_read_samples,
			.in_signature = "<BBH", .out_signature = "<*H",
			.in_param_names = "adc_number, channel_number, sample_count",
			.out_param_names = "adc_samples",
			.doc =
				"Initialize the specified ADC for usage, with the given parameters.\n"
				"\n"
				"Params:\n"
				"    adc_number -- which ADC to use to do the reading (should be 0 or 1)\n"
				"    channel_number -- which channel to use to do the reading (0-7)\n"
				"    sample_count -- number of samples to read\n"
				"Returns: the raw samples read from the ADC."
		},
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(adc, CLASS_NUMBER_SELF, "adc", _verbs,
		"read from the GF's on-board ADC");
