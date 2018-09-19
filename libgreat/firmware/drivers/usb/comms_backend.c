/*
 * This file is part of libgreat
 *
 * USB driver backend to the libgreat communications API.
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/comms.h>
#include <drivers/comms_backend.h>
#include <drivers/usb/comms_backend.h>

#include <drivers/usb/lpc43xx/usb_standard_request.h>
#include <drivers/usb/lpc43xx/usb_queue.h>

struct comm_backend_driver usb_backend_driver = {
	.name = "USB",
};


// FIXME: abstract the maximum size, here
uint8_t usb_data_in_buffer[4096];
uint8_t usb_data_out_buffer[4096];

/** stores the currently active transaction; invalidated when
 * no transaction is currently active. */
static struct command_transaction active_transaction;
static bool transaction_underway = false;


static usb_request_status_t libgreat_comms_vendor_request_out_handler(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) 
{
	int rc;
	unsigned data_length;

	// Get a reference to the USB prelude buffer,
	// and to the data that follows directly after.
	struct libgreat_command_prelude *prelude = (void *)usb_data_in_buffer;
	uint8_t *post_prelude_buffer = &usb_data_in_buffer[sizeof(*prelude)];

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
	// Excellent; let's issue the command itself.
	if (stage == USB_TRANSFER_STAGE_DATA) {

		// Acknowledge the transmission.
		// TODO: validate the command submission, first?
		usb_transfer_schedule_ack(endpoint->in);

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
		transaction_underway = true;

		// Submit the command to the backend for execution.
		// TODO: do we want to set up a back/front to handle these outside
		// of the interrupt context?
		rc = comms_backend_submit_command(&usb_backend_driver, &active_transaction);

		// If any error occurred, stall.
		if (rc) {
			return USB_REQUEST_STATUS_STALL;
		}
	}

	return USB_REQUEST_STATUS_OK;
}


static usb_request_status_t libgreat_comms_vendor_request_in_handler(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) 
{
	if (!transaction_underway)
		return USB_REQUEST_STATUS_STALL;

	// If this is the setup stage of the transaction, schedule the data
	// read itself.
	if (stage == USB_TRANSFER_STAGE_SETUP) {

		// Transmit the amount of returned data, or the requested
		// data; whichever is less.
		unsigned data_length = active_transaction.data_out_length;
		if (endpoint->setup.length < data_length)
			data_length = endpoint->setup.length;

		// Schedule the transfer itself.
		usb_transfer_schedule_block(endpoint->in, usb_data_out_buffer,
				data_length, NULL, NULL);
		return USB_REQUEST_STATUS_OK;
	}

	// If this is the data stage of a transaction that's underway,
	// mark it as complete.
	if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
		transaction_underway = false;
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
