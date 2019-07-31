/*
 * This file is part of GreatFET
 */


#include <debug.h>

#include <drivers/comms.h>
#include <drivers/platform_clock.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#define CLASS_NUMBER_SELF (0x11)

static int selftest_verb_measure_clock_frequencies(struct command_transaction *trans)
{
	// Until we run out of clocks to measure,
	while(comms_argument_data_remaining(trans) >= sizeof(uint32_t)) {
		uint32_t frequency_measured;
		clock_source_t source_to_measure = comms_argument_parse_uint32_t(trans);

		if (!comms_transaction_okay(trans)) {
			return EBADMSG;
		}

		// Measure the relevant clock, and add its frequency to our response.
		frequency_measured = platform_detect_clock_source_frequency(source_to_measure);
		comms_response_add_uint32_t(trans, frequency_measured);
	}

	return 0;
}


static int selftest_verb_measure_raw_clock_frequencies(struct command_transaction *trans)
{
	// Until we run out of clocks to measure,
	while(comms_argument_data_remaining(trans) >= sizeof(uint32_t)) {
		uint32_t frequency_measured;
		clock_source_t source_to_measure = comms_argument_parse_uint32_t(trans);

		if (!comms_transaction_okay(trans)) {
			return EBADMSG;
		}

		// Measure the relevant clock, and add its frequency to our response.
		frequency_measured = platform_detect_clock_source_frequency_directly(source_to_measure);
		comms_response_add_uint32_t(trans, frequency_measured);
	}

	return 0;
}


static struct comms_verb _verbs[] = {
	{ .name = "measure_clock_frequencies", .handler = selftest_verb_measure_clock_frequencies,
		.in_signature ="*I", .out_signature = "*I", .in_param_names = "clock_numbers",
		.out_param_names ="clock_frequencies_mhz", .doc = "Returns the frequencies of the provided clocks, in MHz.\n"
		"Returns 0 MHz to indicate a stopped clock, or clock that could not be brought up." },
	{ .name = "measure_raw_clock", .handler = selftest_verb_measure_raw_clock_frequencies,
		.in_signature ="*I", .out_signature = "*I", .in_param_names = "clock_numbers",
		.out_param_names ="clock_frequencies_mhz", .doc =
		"Returns the frequencies of the provided clocks, in MHz.\n"
		"Does not use any of the techniques we use to improve measurement accuracy; so we get\n"
		"a raw frequency count of the clock. Much less accurate, but useful in diagnosing PLL issues.\n"
		"\n"
		"Returns 0 MHz to indicate a stopped clock, or clock that could not be brought up." },
	{} // Sentinel.
};
COMMS_DEFINE_SIMPLE_CLASS(selftest, CLASS_NUMBER_SELF, "selftest", _verbs,
		"Provides functionality for a GreatFET board to self-test itself.");

