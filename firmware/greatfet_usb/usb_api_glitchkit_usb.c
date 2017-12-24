/*
 * This file is part of GreatFET
 *
 * GlichKit: simple condition counter
 *
 * Feel free to expand me to support arbitrarily complex conditions, but be
 * aware that runtime in the ISR shouldn't grow too long or we'll miss conditions.
 */

#include "usb_api_glitchkit_simple.h"
#include "glitchkit.h"

#include <greatfet_core.h>
#include <string.h>

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"

#include "usb_host.h"
#include "usb_queue_host.h"
#include "usb_registers.h"


static char usb_packet_buffer[512];

static volatile uint32_t read_size = -1;


static void mark_glitchkit_read_complete(void * const user_data,
		unsigned int transferred, bool stalled, bool error)
{
	(void)user_data;
	(void)stalled,
	(void)error;

	// Store the amount actually transferred and complete the transaction.
	// TODO: handle errors/stalls
	read_size = transferred;
}


/**
 * Vendor request that issues a precisely-time control IN request.
 *
 * Data stage:
 *		the eight-byte setup packet to issue
 */
usb_request_status_t usb_vendor_request_glitchkit_control_in_start(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {

		// Ensure we have a well-formed setup packet.
		if(endpoint->setup.length != 8) {
			return USB_REQUEST_STATUS_STALL;
		}

		// Read in the configuration command from the host.
		usb_transfer_schedule_block(endpoint->out, usb_packet_buffer, endpoint->setup.length, NULL, NULL);

	} else if(stage == USB_TRANSFER_STAGE_DATA) {

		read_size = -1;

		// Ack early, as we'll stick in here for quite a while.
		usb_transfer_schedule_ack(endpoint->in);

		// Provide VBUS to the target, if possible.
		// TODO: maybe this should be its own vendor request
		usb_provide_vbus(&usb_peripherals[1]);

		// Synchronize ourself to any synchronizations requested.
		// Note that we do this _after_ VBUS is present to give any devices we power a chance
		// to boot.
		glitchkit_wait_for_events(0);

		// Set up the host controller.
		usb_controller_reset(&usb_peripherals[1]);
		usb_host_init(&usb_peripherals[1]);

		// Enable the USB controller-- this will allow start the point
		// where interrupts can be issued.
		usb_run(&usb_peripherals[1]);

		// Repeatedly reset the device until it shows up.
		// TODO: Do we need to do this? Are our delays just off?
		while (!(USB_REG(1)->PORTSC1 & USB0_PORTSC1_H_CCS)) {
			// Reset the device...
			delay(100 * 1000);
			usb_host_reset_device(&usb_peripherals[1]);
			delay(100 * 1000);
		}

		// FIXME: Read the speed from the status register here,
		// and set up the maximum packet size accordingly.

		// TODO: Generalize me!
		ehci_queue_head_t *qh = set_up_asynchronous_endpoint_queue(&usb_peripherals[1], 0,
				0, USB_SPEED_FULL, true, 64);

		// Send the setup packet...
		usb_host_transfer_schedule(
				&usb_peripherals[1],
				qh,
				USB_PID_TOKEN_SETUP,
				usb_packet_buffer,
				endpoint->setup.length,
				NULL,
				NULL
			);
		// Read the result from the device
		usb_host_transfer_schedule(
				&usb_peripherals[1],
				qh,
				USB_PID_TOKEN_IN,
				usb_packet_buffer,
				sizeof(usb_packet_buffer),
				mark_glitchkit_read_complete,
				NULL
			);
		// ACK stage
		usb_host_transfer_schedule(
				&usb_peripherals[1],
				qh,
				USB_PID_TOKEN_OUT,
				NULL,
				0,
				NULL,
				NULL
			);

	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Vendor request that reads the total amount of data available from a previous
 * (control) IN request. Returns 0xFFFFFFFF if the relevant event has not 
 * yet completed.
 */
usb_request_status_t usb_vendor_request_glitchkit_usb_result_length(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->in, &read_size, sizeof(read_size), NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Returns the data collected in a previous GlitchKit USB request.
 */
usb_request_status_t usb_vendor_request_glitchkit_usb_read_result(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		if(endpoint->setup.length > sizeof(usb_packet_buffer) ||
				(endpoint->setup.length > read_size)) {
			return USB_REQUEST_STATUS_STALL;
		}

		// Transmit the data back.
		usb_transfer_schedule_block(endpoint->in, usb_packet_buffer, endpoint->setup.length, NULL, NULL);

	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}

