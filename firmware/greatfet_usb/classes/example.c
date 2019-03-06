/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include <debug.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#define CLASS_NUMBER_EXAMPLE (0x100)

/**
 * TODO: document adding a new class here
 */


/**
 * Example command that demonstrates how a verb might work.
 * Finds the sum and difference of the two uint32_t arguments provided.
 */
static int example_verb_sum_and_difference(struct command_transaction *trans)
{
	uint32_t a, b, sum, difference;

	// Parse our arguments from the command transaction.
	// Each function reads as single argument from the transaction;
	// the order of the calls should match the argument order used in python.
	a = comms_argument_parse_uint32_t(trans);
	b = comms_argument_parse_uint32_t(trans);

    // Check to see if we were able to handle all of the arguments thus far.
    // If this isn't true, our arguments aren't actually valid, and we should
    // probably bail out before we use that data for anything!
    //
    // No matter whether we bail or not; a transaction that's errored
    // will issue a CommsError (e.g. a USB stall) -- but we ideally want to
    // prevent doing any work with garbage data.
    if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	// Perform the verb's actual work.
	// This is where you'd do whatever your want the verb to do.
	sum = a + b;
	difference = a - b;

	// Prepare our reponse. The order we specify here determines
	// the order in which arguments are returned on the python side.
	comms_response_add_uint32_t(trans, sum);
	comms_response_add_uint32_t(trans, difference);

	return 0;
}


/**
 * Example command that demonstrates how a verb might work.
 * Finds the sum and difference of the two uint32_t arguments provided.
 */
static int example_verb_capitalize(struct command_transaction *trans)
{
	uint32_t num_chars_to_capitalize;
	uint32_t chars_read;
	char *to_capitalize, *capitalized;

	// Read the number of characters to capitalize.
	num_chars_to_capitalize = comms_argument_parse_uint16_t(trans);

	// Read the string to be capitalized.
	to_capitalize = comms_argument_read_buffer(trans, -1, &chars_read);

	// Reserve space in the buffer for our response.
	capitalized = comms_response_reserve_space(trans, chars_read);

	// If we didn't get the buffers we needed, fail out.
	if (!to_capitalize || !capitalized || !comms_transaction_okay(trans)) {
		return EBADMSG;
    }

	// Defensive programming: ensure the last character read is a null.
	to_capitalize[chars_read - 1] = 0;

	// Finally, do our task: copy values from one buffer to another.
	for (uint32_t i = 0; i < chars_read; ++i) {

		if (i < num_chars_to_capitalize) {
			capitalized[i] = toupper(to_capitalize[i]);
		} else {
			capitalized[i] = to_capitalize[i];
		}
	}

	return 0;
}


/**
 * Verbs for the firmware API.
 */
static struct comms_verb example_verbs[] = {
		{
			// The number of the verb. Each verb must have a unique number.
			.verb_number = 0x0,

			// The name of the verb. These are typically named like C funciton names.
			.name = "sum_and_difference",

			// The handler function for the given verb. This is the verb definition we
			// provided above.
			.handler = example_verb_sum_and_difference,

			// A short piece of documentation for the verb.
			.doc = "Computes the sum and difference of two ints.",

			// The signature for the verb's arguments. This roughly matches python's
			// struct.pack format; see the wiki documentation for more information.
			.in_signature = "<II",

			// The signature for the verb's return values. This roughly matches python's
			// struct.pack format; see the wiki documentation for more information.
			.out_signature = "<II",

            // The names of the arguments to the verb.
            .in_param_names = "a, b",

            // The names of the return values for the verb.
            .out_param_names = "sum, difference"
		},

		// A more compact verb entry, like you'd typically see.
		{ .verb_number = 0x1, .name = "capitalize", .handler = example_verb_capitalize, .in_signature = "<HS",
		   .out_signature = "<S", .in_param_names = "chars_to_capitalize, string", .out_param_names = "capitalized",
           .doc = "Capitalizes the first N characters of the provided string." },

		// Every verb list ends with an empty entry. This acts like a null terminator
		// for our list of verbs.
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(example, CLASS_NUMBER_EXAMPLE, "example", example_verbs,
        "Example API that demonstrates how GreatFET communications work.");

