/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include <glitchkit.h>
#include <debug.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#include <drivers/usb/lpc43xx/usb.h>
#include <drivers/usb/lpc43xx/usb_queue.h>
#include "../../usb_endpoint.h"

#include <drivers/usb/lpc43xx/usb_host.h>
#include <drivers/usb/lpc43xx/usb_queue_host.h>
#include <drivers/usb/lpc43xx/usb_registers.h>

#define CLASS_NUMBER_SELF (0x107)


/**
 *  Callback function executed once the core transaction completes.
 *  Used to capture details regarding the full GlitchKit transaction.
 */
static void mark_glitchkit_read_complete(void *const user_data,
		unsigned int transferred, bool stalled, bool error)
{
	volatile struct command_transaction *trans = (struct command_transaction *)user_data;

	// Store the amount actually transferred and complete the transaction.
	trans->data_out_length = transferred;

	// TODO: handle errors/stalls
	(void)stalled;
	(void)error;
}



static int glitchkit_usb_verb_control_in(struct command_transaction *trans)
{
	static volatile ehci_queue_head_t active_qh;
	int rc;

	uint32_t ep_speed_bits;
	uint32_t ep_speed, max_packet_size_for_usb_speed;

	uint32_t setup_packet_length, response_length;
	uint32_t response_max_length = comms_response_space_available(trans);

	void *setup_packet = comms_argument_read_buffer(trans, -1, &setup_packet_length);
	void *response = comms_response_reserve_space(trans, response_max_length);

	if (!comms_transaction_okay(trans) || !setup_packet || !response) {
		return EBADMSG;
	}

	// Provide VBUS to the target, if possible.
	// TODO: maybe this should be its own vendor request
	usb_provide_vbus(&usb_peripherals[1]);
	glitchkit_notify_event(GLITCHKIT_USBHOST_VBUS_ENABLED);

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
		delay_us(100 * 1000);
		usb_host_reset_device(&usb_peripherals[1]);
		delay_us(100 * 1000);
	}

	// Determine the way the endpoint should talk to the device...
	ep_speed_bits = (USB_REG(1)->PORTSC1 >> USB0_PORTSC1_H_PSPD_SHIFT) & USB0_PORTSC1_H_PSPD_MASK;
	if (ep_speed_bits) {
		ep_speed = USB_SPEED_LOW;
		max_packet_size_for_usb_speed = 8;
	} else {
		ep_speed = USB_SPEED_FULL;
		max_packet_size_for_usb_speed = 65;
	}

	// ... and apply the configuration.
	usb_host_set_up_asynchronous_endpoint_queue(&usb_peripherals[1], &active_qh, 0,
			0, ep_speed, true, false, max_packet_size_for_usb_speed);

	// FIXME: use the return codes here for each of the below in a sane way

	// Send the setup packet. (SETUP stage)
	uint8_t *view = setup_packet;
	rc = usb_host_transfer(
			&usb_peripherals[1],
			&active_qh,
			USB_PID_TOKEN_SETUP,
			0,
			setup_packet,
			setup_packet_length,
			0,
			NULL
		);
	if (rc) {
		return rc;
	}

	// Read the response from the device (RESPONSE stage).
	// TODO: parse the setup packet and determine if we should skip this?
	rc = usb_host_transfer(
			&usb_peripherals[1],
			&active_qh,
			USB_PID_TOKEN_IN,
			1,
			response,
			response_max_length,
			0,
			&response_length
		);
	if (rc) {
		return rc;
	}
	trans->data_out_length = response_length;

	// handshake stage
	rc = usb_host_transfer(
			&usb_peripherals[1],
			&active_qh,
			USB_PID_TOKEN_OUT,
			1,
			NULL,
			0,
			0,
			NULL
		);
	if (rc) {
		return rc;
	}
	return 0;
}


static struct comms_verb _verbs[] = {

	/* Control requests. */
	{  .name = "control_in", .handler = glitchkit_usb_verb_control_in, .in_signature = "<*X",
	   .out_signature = "<*X", .in_param_names = "setup_packet", .out_param_names="response",
	   .doc = "Issues a control IN request as GlitchKit stimuli and captures the response." },

	/* Sentinel. */
	{}
};
COMMS_DEFINE_SIMPLE_CLASS(glitchkit_usb, CLASS_NUMBER_SELF, "glitchkit_usb", _verbs,
		"tools for USB fault injection");

