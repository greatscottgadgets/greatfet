/*
 * Copyright 2016 Dominic Spill <dominicgs@gmail.com>
 * Copyright 2017 Mike Naberezny <mike@naberezny.com>
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
#include <gpio_scu.h>
#include <libopencm3/lpc43xx/scu.h>

static struct gpio_t gpio_in[80];   /* Registry of GPIO pins set as inputs */
static struct gpio_t gpio_out[80];  /* Registry of GPIO pins set as outputs */
static uint8_t gpio_in_count = 0;   /* Count of elements used in gpio_in */
static uint8_t gpio_out_count = 0;  /* Count of elements used in gpio_out */
static uint16_t gpio_params[80];    /* Buffer used for USB transfers */


/* Configure the SCU pinmux to GPIO mode for a pin */
static void set_scu_pinmux_for_gpio(uint8_t gpio_port, uint8_t gpio_pin)
{
	scu_grp_pin_t scu_pin = get_scu_pin_for_gpio(gpio_port, gpio_pin);
	uint32_t scu_func = get_scu_func_for_gpio(gpio_port, gpio_pin);
	/* TODO: allow pull-up to be configured */
	scu_pinmux(scu_pin, SCU_GPIO_NOPULL | scu_func);
}


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
usb_request_status_t usb_vendor_request_gpio_register(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t total_pin_count = endpoint->setup.length / 2;
	uint8_t input_pin_count = endpoint->setup.value;
	uint8_t i;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->out, &gpio_params,
									endpoint->setup.length, NULL, NULL);

	} else if (stage == USB_TRANSFER_STAGE_DATA) {

		for (i=0; i < total_pin_count; i++) {
			uint8_t port = (gpio_params[i] >> 8) & 0xFF;
			uint8_t pin = gpio_params[i] & 0xFF;

			if (i < input_pin_count) {
				/* Configure pin as input */
				GPIO_SET(gpio_in[gpio_in_count], port, pin);
				gpio_input(&gpio_in[gpio_in_count]);
				set_scu_pinmux_for_gpio(port, pin);
				gpio_in_count++;
			} else {
				/* Configure pin as output */
				GPIO_SET(gpio_out[gpio_out_count], port, pin);
				gpio_output(&gpio_out[gpio_out_count]);
				gpio_clear(&gpio_out[gpio_out_count]);
				set_scu_pinmux_for_gpio(port, pin);
				gpio_out_count++;
			}
		}

		usb_transfer_schedule_ack(endpoint->in);
	}

	return USB_REQUEST_STATUS_OK;
}


/* Write to GPIO output pins
   Setup packet:
		 wLength: count of all pins in the data packet * 2 for uint16_t
	 Data packet:
     consists of one uint16_t for each GPIO output pin to change:
		   high byte = index into gpio_out array
			 low byte = 0=make pin low, nonzero=make pin high
*/
usb_request_status_t usb_vendor_request_gpio_write(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t total_pin_count = endpoint->setup.length / 2;
	uint8_t i;
	uint8_t gpio_index;
	struct gpio_t gpio;
	bool state;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->out, &gpio_params,
									endpoint->setup.length, NULL, NULL);

	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		for (i=0; i<total_pin_count; i++) {
			gpio_index = (gpio_params[i] >> 8) & 0xFF;
			state = (gpio_params[i] & 0xFF) != 0;

			if (gpio_index < gpio_out_count) {
				gpio = gpio_out[gpio_index];
				gpio_write(&gpio, state);
			}
		}

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/* Reset any registered GPIO pins back to their default state and
 * clear all registrations.
 */
usb_request_status_t usb_vendor_request_gpio_reset(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t i;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		/* change any outputs back to inputs */
		for (i=0; i<gpio_out_count; i++) {
			gpio_input(&gpio_out[i]);
		}

		/* unregister all */
		gpio_in_count = 0;
		gpio_out_count = 0;

		usb_transfer_schedule_ack(endpoint->in);
	}

	return USB_REQUEST_STATUS_OK;
}
