/*
 * This file is part of GreatFET
 */

#include "usb_api_msp430.h"
#include <drivers/usb/lpc43xx/usb_queue.h>

#include <stddef.h>
#include <greatfet_core.h>
#include <jtag430.h>


usb_request_status_t usb_vendor_request_msp430_jtag(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		uint8_t jtagid = jtag430_do_a_thing();
		// These values are taken from jtag430.c
		// In the future we'll have a sensible API for this
		// jtag430_eraseflash(0xA506,0xFFFE,0x3000,0);
		// jtag430_stop();
		// usb_transfer_schedule_ack(endpoint->in);
		usb_transfer_schedule_block(endpoint->in, &jtagid, 1, NULL, NULL);
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}
