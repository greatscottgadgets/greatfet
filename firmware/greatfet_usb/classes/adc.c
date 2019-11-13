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

#include "../usb_streaming.h"


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


static uint16_t take_adc_sample(uint8_t adc_number, uint8_t pin_channel_number)
{
	const uint32_t adc_sample_mask = 0x03FF;

	volatile uint32_t *adc_cr;
	volatile uint32_t *adc_gdr;

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


	uint32_t time_base = get_time();

	// Start a conversion.
	*adc_cr |= ADC_CR_START(1);

	// And wait for it to complete.
	while(!(*adc_gdr & ADC_DR_DONE) || (((*adc_gdr >> 24) & 0x7) != pin_channel_number)) {
		if (get_time_since(time_base) > 500) {
			return -1;
		}
	}

	// Read the most recently sampled V_REF from the global data register.
	return (*adc_gdr >> 6) & adc_sample_mask;

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
 * Callback that captures ADC data periodically.
 */
void stream_adc_data(void *argument)
{
	(void)argument;

	// TODO: accept channels other than ADC0_0 and implement pinmuxing support
	uint16_t sample = take_adc_sample(0, 0);
	usb_streaming_send_data(&sample, sizeof(uint16_t));
}



static int verb_stream_periodic_read(struct command_transaction *trans)
{
	uint32_t frequency = comms_argument_parse_uint32_t(trans);

	if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	// Set up the ADC.
	// FIXME: support pins other than ADC0/0
	set_up_onboard_adc(0, 1 << 0, 10);

	// Schedule our periodic read.
	usb_streaming_start_periodic_data_gathering(frequency, stream_adc_data, NULL);
	comms_response_add_uint8_t(trans, USB_STREAMING_IN_ADDRESS);

	return 0;
}



static int verb_stop_periodic_read(struct command_transaction *trans)
{
	(void)trans;

	usb_streaming_stop_periodic_gathering();
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

		// Functionality for streaming repeated sampes over a bulk pipe.
		{ .name = "stream_periodic_read", .handler = verb_stream_periodic_read,
			.in_signature = "<I", .out_signature = "<B",
			.in_param_names = "frequency", .out_param_names = "pipe_id",
			.doc = "Schedule a periodic ADC read, and stream its results to the host." },
		{ .name = "stop_periodic_read", .handler = verb_stop_periodic_read,
			.in_signature = "", .out_signature = "",
			.doc = "Stop any active periodic read."},

		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(adc, CLASS_NUMBER_SELF, "adc", _verbs,
		"read from the GF's on-board ADC");
