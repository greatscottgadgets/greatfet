/*
 * This file is part of libgreat
 *
 * USB driver backend to the libgreat communications API.
 */

#include <stddef.h>
#include <stdint.h>
#include <debug.h>

#include <drivers/comms.h>
#include <drivers/comms_backend.h>
#include <drivers/usb/comms_backend.h>

#include <drivers/usb/lpc43xx/usb_standard_request.h>
#include <drivers/usb/lpc43xx/usb_queue.h>

#define LIBGREAT_REQUEST_CANCEL_VALUE (0xDEAD)

/** Flag indicating that the host does not expect us to send a response. */
/* This allows us to skip half of the USB transaction. */
#define LIBGREAT_REQUEST_FLAG_SKIP_RESPONSE (1 << 0)

/** Flag indicating that the host expects us to use the same input (class/verb/arguments)
 *  for the next transaction. This allows us to skip half of the USB transaction.
 *  Currently only valid when performing a follow-up IN request. */
#define LIBGREAT_REQUEST_FLAG_REPEAT_LAST (1 << 1)


struct comm_backend_driver usb_backend_driver = {
	.name = "USB",
};

/** stores the currently active transaction; invalidated when
 * no transaction is currently active. */
static struct command_transaction active_transaction;
static bool transaction_underway = false;

// FIXME: abstract the maximum size, here
uint8_t usb_data_in_buffer[4096] ATTR_ALIGNED(4);
uint8_t usb_data_out_buffer[4096] ATTR_ALIGNED(4);

/** Clears our position in the current transaction. */
static void libgreat_clear_position_in_active_transaction(void)
{
	active_transaction.data_out_length = 0;
	active_transaction.data_in_position = active_transaction.data_in;
	active_transaction.data_out_position = active_transaction.data_out;
	active_transaction.data_in_remaining = active_transaction.data_in_length;
	active_transaction.data_in_status = COMMS_PARSE_OKAY;
	active_transaction.data_out_status = COMMS_PARSE_OKAY;
	active_transaction.last_error_number = 0;
}


static usb_request_status_t libgreat_comms_vendor_request_out_handler(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	int rc;
	unsigned data_length;

	// Get a reference to the USB prelude buffer,
	// and to the data that follows directly after.
	struct libgreat_command_prelude *prelude = (void *)usb_data_in_buffer;
	uint8_t *post_prelude_buffer = &usb_data_in_buffer[sizeof(*prelude)];

	// If we've gotten out of sync, cancel the existing transaction and
	// stall this request. This likely means the host misbehaved.
	if ((stage != USB_TRANSFER_STAGE_STATUS) && transaction_underway) {
		const char *class_name = comms_get_class_name(active_transaction.class_number, "<unknown-class>");
		const char *handler_name = comms_get_handler_name(active_transaction.class_number, active_transaction.verb,
				"<class-handler>", "<unknown-verb>");

		pr_error("comms error: host issued a new request while an active request is being handled "
				"(handling %s.%s)! \n", class_name, handler_name);
		transaction_underway = false;
		return USB_REQUEST_STATUS_STALL;
	}

	// If we can't accomodate requests of the given size, stall.
	if (endpoint->setup.length > sizeof(usb_data_in_buffer)) {
		return USB_REQUEST_STATUS_STALL;
	}

	// If this isn't at least the size of our command prelude, stall.
	if (endpoint->setup.length < sizeof(*prelude)) {
		return USB_REQUEST_STATUS_STALL;
	}

	// Compute how much non-prelude data we have.
	data_length = endpoint->setup.length - sizeof(*prelude);

	// If this is the setup stage of the transaction, schedule the data
	// read itself.
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		rc = usb_transfer_schedule_block(endpoint->out, usb_data_in_buffer,
				endpoint->setup.length, NULL, NULL);
		return rc ? USB_REQUEST_STATUS_STALL : USB_REQUEST_STATUS_OK;
	}

	// If we've just completed the data stage, we have data to work with!
	if (stage == USB_TRANSFER_STAGE_DATA) {

		// Determine if the host is opting to skip reading a response.
		bool skip_response = (endpoint->setup.index & LIBGREAT_REQUEST_FLAG_SKIP_RESPONSE);

		// Populate the transaction details.
		active_transaction.class_number = prelude->class_number;
		active_transaction.verb = prelude->verb;
		active_transaction.data_in = post_prelude_buffer;
		active_transaction.data_in_length = data_length;
		active_transaction.data_out = usb_data_out_buffer;
		active_transaction.data_out_max_length = sizeof(usb_data_out_buffer);
		libgreat_clear_position_in_active_transaction();
		transaction_underway = true;

		// Submit the command to the backend for execution.
		// TODO: do we want to set up a back/front to handle these outside
		// of the interrupt context? or perhaps check for an early-ack for the given class/verb?
		active_transaction.last_error_number = comms_backend_submit_command(&usb_backend_driver, &active_transaction);

		// If the host is opting to skip reading a response, the transaction is compelte, here.
		if (skip_response) {
			transaction_underway = false;
		}

		// If any error occurred, stall.
		if (active_transaction.last_error_number) {
			return USB_REQUEST_STATUS_STALL;
		}
		// Otherwise, ACK the transcation.
		else {
			rc = usb_transfer_schedule_ack(endpoint->in);
			return rc ? USB_REQUEST_STATUS_STALL : USB_REQUEST_STATUS_OK;
		}
	}

	return USB_REQUEST_STATUS_OK;
}

/**
 * Re-issues the most recent libgreat command with the same in-arguments.
 */
static void libgreat_comms_reissue_command(void)
{
	// Reset our positions and status within the transaction.
	libgreat_clear_position_in_active_transaction();

	// Submit the command to the backend for execution.
	// TODO: do we want to set up a back/front to handle these outside
	// of the interrupt context? or perhaps check for an early-ack for the given class/verb?
	transaction_underway = true;
	active_transaction.last_error_number = comms_backend_submit_command(&usb_backend_driver, &active_transaction);
}


static usb_request_status_t libgreat_comms_vendor_request_in_handler(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	int rc;

	// If this is the setup stage of the transaction, schedule the data
	// read itself.
	if (stage == USB_TRANSFER_STAGE_SETUP) {

		// If this is a request for a repeat of the previous command with the same in-arguments,
		// re-issue the command.
		if (endpoint->setup.index & LIBGREAT_REQUEST_FLAG_REPEAT_LAST) {
			libgreat_comms_reissue_command();
		}

		// Check to make sure we have an active transaction to respond to.
		if (!transaction_underway) {
			pr_error("comms error: requested a USB response when no communications were underway! (stage: %d)\n", stage);
			return USB_REQUEST_STATUS_STALL;
		}

		// Transmit the amount of returned data, or the requested
		// data; whichever is less.
		uint32_t data_length = active_transaction.data_out_length;
		if (endpoint->setup.length < data_length) {
			data_length = endpoint->setup.length;
		}
		if (sizeof(usb_data_out_buffer) < data_length) {
			data_length = sizeof(usb_data_out_buffer);
		}
		// Schedule the transfer itself.
		rc = usb_transfer_schedule_block(endpoint->in, usb_data_out_buffer,
				data_length, NULL, NULL);
		return rc ? USB_REQUEST_STATUS_STALL : USB_REQUEST_STATUS_OK;
	}

	// If this is the end of the DATA stage, queue an ACK for the status stage.
	// and mark the transaction as dispatched.
	if (stage == USB_TRANSFER_STAGE_DATA) {
		transaction_underway = false;
		rc = usb_transfer_schedule_ack(endpoint->out);
		return rc ? USB_REQUEST_STATUS_STALL : USB_REQUEST_STATUS_OK;
	}

	return USB_REQUEST_STATUS_OK;
}

/**
 * Handler for special cancellation requests, which abort execution of an
 * existing command. These allow us to fail gracefully rather than being
 * confused as to where we are in the command.
 */
static usb_request_status_t libgreat_comms_vendor_request_cancel_handler(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	int rc;
	static uint32_t last_errno;

	// If this is the setup stage, perform the actual cancellation and query our response.
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		pr_debug("usb comms: aborting active command at host's request\n");

		// Grab the most recent transaction's error number, and invalidate the existing transaction.
		last_errno = active_transaction.last_error_number;
		transaction_underway = false;

		if(endpoint->setup.length != sizeof(last_errno)) {
			pr_warning("usb comms: received an invalid abort request (bad length of %d)!\n", endpoint->setup.length);
			return USB_REQUEST_STATUS_STALL;
		}

		// Schedule a respone
		rc = usb_transfer_schedule_block(endpoint->in, &last_errno, sizeof(last_errno), NULL, NULL);
		return rc ? USB_REQUEST_STATUS_STALL : USB_REQUEST_STATUS_OK;
	}

	// If this is the data stage, ACk our transaction and complete.
	if (stage == USB_TRANSFER_STAGE_DATA) {
		rc = usb_transfer_schedule_ack(endpoint->out);
		return rc ? USB_REQUEST_STATUS_STALL : USB_REQUEST_STATUS_OK;
	}


	return USB_REQUEST_STATUS_OK;
}


/**
 * Top-level handler for vendor requests for LPC43xx devices that are
 * communicating via a libgreat backend.
 */
usb_request_status_t libgreat_comms_vendor_request_handler(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	bool is_in_request;

	// If this is a cancel request, cancel the active request.
	if (endpoint->setup.value == LIBGREAT_REQUEST_CANCEL_VALUE) {
		return libgreat_comms_vendor_request_cancel_handler(endpoint, stage);
	}

	// If this is an IN request, we're being asked for the response
	// to a previous query. Handle that accordingly.
	is_in_request = endpoint->setup.request_type
		>> USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_shift;
	if (is_in_request) {
		return libgreat_comms_vendor_request_in_handler(endpoint, stage);
	} else {
		return libgreat_comms_vendor_request_out_handler(endpoint, stage);
	}

}
