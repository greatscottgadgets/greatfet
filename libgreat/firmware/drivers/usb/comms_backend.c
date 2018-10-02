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

struct comm_backend_driver usb_backend_driver = {
	.name = "USB",
};

/** stores the currently active transaction; invalidated when
 * no transaction is currently active. */
static struct command_transaction active_transaction;
static bool transaction_underway = false;

// FIXME: abstract the maximum size, here
uint8_t usb_data_in_buffer[4096] ;
uint8_t usb_data_out_buffer[4096];



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
		pr_error("comms error: host issued a new request while an active request is being handled "
				"(handling %d:%d)! \n", active_transaction.class_number, active_transaction.verb);
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
		usb_transfer_schedule_block(endpoint->out, usb_data_in_buffer,
				endpoint->setup.length, NULL, NULL);
		return USB_REQUEST_STATUS_OK;
	}

	// If we've just completed the data stage, we have data to work with!
	if (stage == USB_TRANSFER_STAGE_DATA) {

		// Populate the transaction details.
		active_transaction.class_number = prelude->class_number;
		active_transaction.verb = prelude->verb;
		active_transaction.data_in = post_prelude_buffer;
		active_transaction.data_in_length = data_length;
		active_transaction.data_out = usb_data_out_buffer;
		active_transaction.data_out_max_length = sizeof(usb_data_out_buffer);
		active_transaction.data_out_length = 0;
		active_transaction.data_in_position = active_transaction.data_in;
		active_transaction.data_out_position = active_transaction.data_out;
		active_transaction.data_in_remaining = active_transaction.data_in_length;
		transaction_underway = true;

		// Submit the command to the backend for execution.
		// TODO: do we want to set up a back/front to handle these outside
		// of the interrupt context?
		rc = comms_backend_submit_command(&usb_backend_driver, &active_transaction);

		// If any error occurred, stall.
		if (rc) {
			return USB_REQUEST_STATUS_STALL;
		}
		// Otherwise, ACK the transcation.
		else {
			usb_transfer_schedule_ack(endpoint->in);
		}
	}

	return USB_REQUEST_STATUS_OK;
}


static usb_request_status_t libgreat_comms_vendor_request_in_handler(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	// If this is the setup stage of the transaction, schedule the data
	// read itself.
	if (stage == USB_TRANSFER_STAGE_SETUP) {

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
		usb_transfer_schedule_block(endpoint->in, usb_data_out_buffer,
				data_length, NULL, NULL);
	}

	// If this is the end of the DATA stage, queue an ACK for the status stage.
	// and mark the transaction as dispatched.
	if (stage == USB_TRANSFER_STAGE_DATA) {
		transaction_underway = false;
		usb_transfer_schedule_ack(endpoint->out);
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
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		pr_debug("usb comms: aborting active command at host's request\n");
		transaction_underway = false;
		usb_transfer_schedule_ack(endpoint->in);
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
