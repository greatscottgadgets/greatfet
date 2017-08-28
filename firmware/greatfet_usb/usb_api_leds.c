/*
 * This file is part of GreatFET
 */

#include "usb_api_leds.h"

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"

usb_request_status_t usb_vendor_request_led_toggle(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		switch(endpoint->setup.value) {
			case 1:
				led_toggle(LED1);
				break;
			case 2:
				led_toggle(LED2);
				break;
			case 3:
				led_toggle(LED3);
				break;
			case 4:
				led_toggle(LED4);
				break;
		}
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
