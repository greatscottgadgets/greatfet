/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <greatfet_core.h>
#include <debug.h>

#include <drivers/comms.h>

#define CLASS_NUMBER_DEBUG (0x10)


/**
 * Command to read the contents of the debug ring buffer.
 */
static int verb_read_dmesg(struct command_transaction *trans)
{
	trans->data_out_length = debug_ring_read(trans->data_out, trans->data_out_max_length, false);
	return 0;
}

/**
 * Commmand that clears the debug ring.
 */
static int verb_clear_dmesg(struct command_transaction *trans)
{
	// Perform a read where we discard the results.
	trans->data_out_length = debug_ring_read(trans->data_out, trans->data_out_max_length, true);
	return 0;
}

static int verb_peek(struct command_transaction *trans)
{
	volatile uint32_t *address = (void *)comms_argument_parse_uint32_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	comms_response_add_uint32_t(trans, *address);
	return 0;
}

static int verb_poke(struct command_transaction *trans)
{
	volatile uint32_t *address = (void *)comms_argument_parse_uint32_t(trans);
	uint32_t value = comms_argument_parse_uint32_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	*address = value;
	return 0;
}

// TODO: verb for setting the current log level

/**
 * Verbs for the debug API.
 */
struct comms_verb debug_verbs[] = {
		{ .name = "read_dmesg",  .handler = verb_read_dmesg,
            .in_signature = "", .out_signature="<S", .out_param_names = "log",
            .doc = "Fetches the content of the device's debug ring (log)."
        },
		{ .name = "clear_dmesg",  .handler = verb_clear_dmesg,
            .in_signature = "", .out_signature="<S", .out_param_names = "log",
            .doc = "Fetches and clears content of the device's debug ring (log)."
        },
		{ .name = "peek",  .handler = verb_peek,
            .in_signature = "<I", .out_signature="<I", .in_param_names = "address", .out_param_names = "value",
            .doc = "Reads a raw LPC4330 memory address; for debug."
        },
		{ .name = "poke",  .handler = verb_poke,
            .in_signature = "<II", .out_signature="", .in_param_names = "address, value", .out_param_names = "",
            .doc = "Writes a raw LPC4330 memory address; for debug."
        },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(debug_api, CLASS_NUMBER_DEBUG, "debug", debug_verbs,
        "API for accessing the device's debug state.");
