/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include <debug.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#include <drivers/uart.h>

#define UART_BUFFER_SIZE  256
#define CLASS_NUMBER_SELF (0x112)

// TODO: abstract the UART count
static uart_t uart[4];

static int verb_initialize(struct command_transaction *trans)
{
	const uart_parity_type_t parity_look_up_table[] = {
		NO_PARITY, ODD_PARITY, EVEN_PARITY, PARITY_STUCK_AT_ONE, PARITY_STUCK_AT_ZERO
	};

	uart_parity_type_t parity_mode;
	uint8_t stop_bits;

	// FIXME: prevent double allocations
	// translate repeat initialization calls to either 1) free first, or 2) reconfigure live

	uint8_t uart_number         = comms_argument_parse_uint8_t(trans);
	uart[uart_number].baud_rate = comms_argument_parse_uint32_t(trans);
	uart[uart_number].data_bits = comms_argument_parse_uint8_t(trans);

	// Read the parity type.
	parity_mode = comms_argument_parse_uint8_t(trans);
	if (parity_mode >= ARRAY_SIZE(parity_look_up_table)) {
		pr_error("uart: invalid argument for parity type!\n");
		return EINVAL;
	}

	// Read the number of stop bits.
	stop_bits = comms_argument_parse_int8_t(trans);

	uart[uart_number].parity_mode = parity_look_up_table[parity_mode];
	uart[uart_number].stop_bits = (stop_bits == 2) ? TWO_STOP_BITS : ONE_STOP_BIT;
	uart[uart_number].number = uart_number;

	// Enable asynchronous data capture.
	uart[uart_number].buffer_size = UART_BUFFER_SIZE;

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	uart_init(&uart[uart_number]);

	comms_response_add_uint32_t(trans, uart->baud_rate_achieved);
	return 0;
}


static int verb_synchronous_transmit(struct command_transaction *trans)
{
	uint8_t uart_number = comms_argument_parse_uint8_t(trans);
	uint8_t byte = comms_argument_parse_uint8_t(trans);

	if(!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	uart_transmit_synchronous(&uart[uart_number], byte);
	return 0;
}

static int verb_read(struct command_transaction *trans)
{
	uint32_t count_requested = comms_argument_parse_uint32_t(trans);
	uint32_t buffer_size;


	if(!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// Grab the total amount of space available for response.
 	buffer_size = trans->data_out_max_length;

	// If the count requested isn't zero, truncate it to the requested amount.
	if (count_requested != 0) {
		if (count_requested < buffer_size) {
			buffer_size = count_requested;
		}
	}


	pr_info("uart: reading %d bytes!\n", buffer_size);


	// Read the actual data from the UART's buffer.
	trans->data_out_length = uart_read(uart, trans->data_out, buffer_size);
	pr_info("uart: read %d bytes!\n", trans->data_out_length);


	return 0;
}

static struct comms_verb _verbs[] = {

		/* Control. */
		{ .name = "initialize", .handler = verb_initialize,
			.in_signature = "<BIBBB", .out_signature = "<I",
			.in_param_names = "uart_number, baud_rate, data_bits, parity_mode, stop_bits",
			.out_param_names = "baud_achieved", .doc =
			"Prepares a UART for use by the rest of this API.\n"
			"\n"
			"Parameters:\n"
			"    uart_number -- The number of the UART to use.\n"
			"    baud_rate -- The desired baud rate for comms.\n" // TODO: or 0 to autobaud
			"    data_bits -- The number of data bits per frame.\n"
			"    parity mode -- The parity mode to use (0 = none, 1 = odd, 2 = even, 3 = always one, 4 = always zero).\n"
			"    stop_bits -- The number of stop bits.\n"
			"Returns the actual baud rate achieved, in Hz."
		},

		/* Low-level transmit and receive. */
		{ .name = "synchronous_transmit", .handler = verb_synchronous_transmit,
			.in_signature = "<BB", .out_signature = "",
			.in_param_names = "uart_number, byte",
			.doc = "Transmits the provided byte over the given UART."
		},

		/* File-like API for bulk reads and writes. */
		{ .name = "read", .handler = verb_read,
			.in_signature = "<I", .out_signature = "<*X",
			.in_param_names = "count", .out_param_names="data",
			.doc =
				"Reads from the UART buffer, capturing recently received data.\n"
				"\n"
				"Parameters:\n"
				"	count -- The total bytes to read, or 0 to read all available.\n"
				"\n"
				"Returns:\n"
				"	a bytes object containing the data read; which may be up to `count` bytes long"
		},

		{}
};
COMMS_DEFINE_SIMPLE_CLASS(uart, CLASS_NUMBER_SELF, "uart", _verbs,
        "functions to enable talking 'serial'")

