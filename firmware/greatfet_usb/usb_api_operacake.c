/*
 * This file is part of GreatFET
 */

#include "usb_api_operacake.h"
#include "usb_queue.h"

#include <operacake.h>

usb_request_status_t usb_vendor_request_operacake(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		operacake_init();
		operacake_gpio();
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
