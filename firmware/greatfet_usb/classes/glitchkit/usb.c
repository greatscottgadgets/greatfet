/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include <glitchkit.h>
#include <debug.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#include <drivers/usb/usb.h>
#include <drivers/usb/usb_queue.h>
#include "../../usb_endpoint.h"

#include <drivers/usb/usb_host.h>
#include <drivers/usb/usb_queue_host.h>
#include <drivers/usb/usb_registers.h>

#define CLASS_NUMBER_SELF (0x107)

/**
 * Persistent configuration flags.
 */
static bool continue_despite_errors = false, disable_vbus_afterwards = false;

/**
 * Print USB error messages for the host.
 */
static void print_usb_rc(int rc)
{
	switch(rc) {
		case EPIPE:
			pr_warning("USB request to target: target device stalled request.\n");
			break;
		case EIO:
			pr_warning("USB request to target: communications error; likely device did not respond.\n");
			break;
		case ETIMEDOUT:
			pr_warning("USB request to target: timed out issuing request.\n");
			break;
	}
}

/**
 * Should be called after a request. Shuts down VBUS after the request iff the
 * module has been configured to do so using configure_requests.
 *
 * WARNING: not timing balanced; only should be called from after the trigger point
 * or after a sure failure
 */
static void shut_down_vbus_if_requested()
{
	if(disable_vbus_afterwards) {
		usb_stop_providing_vbus(&usb_peripherals[1]);
	}
}

static int glitchkit_usb_verb_configure_requests(struct command_transaction *trans)
{
	bool continue_despite = comms_argument_parse_bool(trans);
	bool powerdown_after = comms_argument_parse_bool(trans);

    if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

	// Store configuration for future requests.
	continue_despite_errors = continue_despite;
	disable_vbus_afterwards = powerdown_after;
	return 0;
}


static int glitchkit_usb_verb_control_in(struct command_transaction *trans)
{
	static volatile ehci_queue_head_t active_qh;
	int rc, final_rc = 0;

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

	// Reset the device...
	delay_us(100 * 1000);
	usb_host_reset_device(&usb_peripherals[1]);
	delay_us(100 * 1000);

	// Validate that the device is present, and error out if it isn't.
	if(!(USB_REG(1)->PORTSC1 & USB0_PORTSC1_H_CCS) && !continue_despite_errors) {
		pr_warning("USB request to target: no target device (device's USB pull-ups not detected).\n");
		return ENODEV;
	} else {
		// TODO: balance this case?
		pr_warning("USB request to target: no target device (device's USB pull-ups not detected).\n");
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
	if (rc && continue_despite_errors) {
		final_rc = rc;
	} else if (rc) {
		print_usb_rc(rc);
		shut_down_vbus_if_requested();
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
	if (rc && continue_despite_errors) {
		final_rc = rc;
	} else if (rc) {
		print_usb_rc(rc);
		shut_down_vbus_if_requested();
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
	if (rc && continue_despite_errors) {
		final_rc = rc;
	} else if (rc) {
		print_usb_rc(rc);
		shut_down_vbus_if_requested();
		return rc;
	}

	shut_down_vbus_if_requested();
	return final_rc;
}


static struct comms_verb _verbs[] = {

	/* General configuration. */
	{  .name = "configure_requests", .handler = glitchkit_usb_verb_configure_requests, .in_signature = "<??",
	   .out_signature = "", .in_param_names = "continue_despite_errors, disable_vbus_after",
	   .doc = "Configures future requests. Optional; defaults are somewhat sane." },

	/* Control requests. */
	{  .name = "control_in", .handler = glitchkit_usb_verb_control_in, .in_signature = "<*X",
	   .out_signature = "<*X", .in_param_names = "setup_packet", .out_param_names="response",
	   .doc = "Issues a control IN request as GlitchKit stimuli and captures the response." },

	/* Sentinel. */
	{}
};
COMMS_DEFINE_SIMPLE_CLASS(glitchkit_usb, CLASS_NUMBER_SELF, "glitchkit_usb", _verbs,
		"tools for USB fault injection");

