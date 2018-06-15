/*
 * This file is part of GreatFET
 */

#include "usb_api_msp430.h"
#include "usb_queue.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <jtag430.h>


usb_request_status_t usb_vendor_request_msp430_jtag(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		jtag430_start();
		jtag430_eraseflash(0xA506,0xFFFE,0x3000,0);
		// jtag430_haltcpu();
		// usb_transfer_schedule_block(endpoint->in, &spi_buffer,
		//  							endpoint->setup.length, NULL, NULL);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
