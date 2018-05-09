/*
 * This file is part of GreatFET
 */

#include "usb_api_1-Wire.h"
#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <gpio.h>
#include <pins.h>
#include <gpio_lpc.h>

#include <libopencm3/lpc43xx/scu.h>

static struct gpio_t dq = GPIO(5, 8);

usb_request_status_t usb_vendor_request_1wire_init(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		scu_pinmux(SCU_PINMUX_GPIO5_8, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
		gpio_output(&dq);
        gpio_write(&dq, 1);
        usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_1wire_read(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {

	if (stage == USB_TRANSFER_STAGE_SETUP) {
        
		usb_transfer_schedule_ack(endpoint->in);
	}

	return USB_REQUEST_STATUS_OK;
}