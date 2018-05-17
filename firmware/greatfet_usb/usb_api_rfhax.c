/*
 * This file is part of GreatFET
 */

#include "usb_api_rfhax.h"

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"
#include <greatfet_core.h>

#define RFHAX_OFF 0
#define RFHAX_CW 1
#define RFHAX_ASK 2
#define RFHAX_FSK 3

static uint8_t mode;

usb_request_status_t usb_vendor_request_rfhax(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	mode = endpoint->setup.index;
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		switch(endpoint->setup.index) {
			case RFHAX_OFF:
				break;
			case RFHAX_CW:
				break;
			case RFHAX_ASK:
				break;
			case RFHAX_FSK:
				break;
		}
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
