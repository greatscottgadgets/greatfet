/*
 * This file is part of GreatFET
 */

#include "usb_api_leds.h"

#include <drivers/usb/lpc43xx/usb.h>
#include <drivers/usb/lpc43xx/usb_queue.h>
#include "usb_endpoint.h"

/* Define a simple function pointer that accepts all LED modification functions. */
typedef void (*led_func)(led_t);

/**
 * Handlers for each of the LED requests.
 */
static const led_func led_request_handlers[] = {
	led_toggle,    // 0
	led_off,       // 1
	led_on         // 2
};

/**
 * Perform a simple operation on the LEDs. This is distinct from the GPIO
 * API, as it goes through the GreatFET's internal HAL.
 */
usb_request_status_t usb_vendor_request_set_leds(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		unsigned request_type = endpoint->setup.index;
		led_func handler;

		/* If we've been passed an invalid request number, stall. */
		if(request_type >= ARRAY_SIZE(led_request_handlers)) {
			usb_endpoint_stall(endpoint);
		}

		/* Otherwise, figure out the handler function for the given LED request. */
		handler = led_request_handlers[request_type];

		/* If we have a valid handler, execute it. */
		if (handler) {
				handler(endpoint->setup.value - 1);
		}

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
