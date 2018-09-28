/*
 * This file is part of GreatFET
 */

#include "usb_api_dac.h"
#include <drivers/usb/lpc43xx/usb.h>
#include <drivers/usb/lpc43xx/usb_queue.h>
#include "usb_endpoint.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <dac.h>
#include <pins.h>

usb_request_status_t usb_vendor_request_dac_set(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	usb_endpoint_init(&usb0_endpoint_bulk_in);
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		
		// set J2_P5 up as a DAC pin
		scu_pinmux(SCU_PINMUX_GPIO2_3, SCU_GPIO_FAST | SCU_GPIO_PUP | SCU_CONF_FUNCTION0);
		static struct gpio_t dac_pin = GPIO(2, 3);
		gpio_input(&dac_pin); 

		DAC_CR = DAC_CR_VALUE(endpoint->setup.value) & DAC_CR_VALUE_MASK;
		DAC_CTRL = DAC_CTRL_DMA_ENA;

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
