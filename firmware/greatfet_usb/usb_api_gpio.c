/*
 * Copyright 2016 Dominic Spill
 *
 * This file is part of GreatFET.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "usb_api_gpio.h"
#include "usb_queue.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <gpio.h>
#include <gpio_lpc.h>

static struct gpio_t gpio_in[80];   /* Registry of GPIO pins set as inputs */
static struct gpio_t gpio_out[80];  /* Registry of GPIO pins set as outputs */
static uint8_t gpio_in_count = 0;   /* Count of elements used in gpio_in */
static uint8_t gpio_out_count = 0;  /* Count of elements used in gpio_out */
static uint16_t gpio_params[80];    /* Buffer used for USB transfers */

/* Set GPIO pin definitions and directions
   Setup packet:
	   wValue:  count of pins in the data packet that will be inputs
		 wLength: count of all pins in the data packet * 2 for uint16_t
	 Data packet:
     consists of one uint16_t for each LPC GPIO to set up:
		   high byte = port, low byte = pin
     ordering in packet determines pin direction:
		   inputs first (determined by wValue), then outputs
*/
usb_request_status_t usb_vendor_request_register_gpio(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t total_pin_count = endpoint->setup.length / 2;
	uint8_t input_pin_count = endpoint->setup.value;
	uint8_t i;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->out, &gpio_params,
									endpoint->setup.length, NULL, NULL);

	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		/* Configure input pins */
		for(i=0; i<input_pin_count; i++) {
			GPIO_SET(gpio_in[gpio_in_count],
					 (gpio_params[i] >> 8) & 0xFF, /* port */
					  gpio_params[i] & 0xFF		     /* pin */
					);
			gpio_input(&gpio_in[gpio_in_count]);
			/* TODO scu pinmux must be configured to make pin gpio */
			gpio_in_count++;
		}

		/* Configure output pins */
		for(; i<total_pin_count; i++) {
			GPIO_SET(gpio_out[gpio_out_count],
					 (gpio_params[i]>>8) & 0xFF, /* port */
					  gpio_params[i] & 0xFF		   /* pin */
					);
			gpio_output(&gpio_out[gpio_out_count]);
			gpio_clear(&gpio_out[gpio_out_count]);
			/* TODO scu pinmux must be configured to make pin gpio */
			gpio_out_count++;
		}

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

/* Toggle out pins */
usb_request_status_t usb_vendor_request_write_gpio(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		/* TODO finish me */
		return USB_REQUEST_STATUS_OK;
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		/* TODO finish me */
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
