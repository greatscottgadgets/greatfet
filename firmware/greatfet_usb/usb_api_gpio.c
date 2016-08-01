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

volatile bool start_gpio_monitor = false;
static uint8_t gpio_in_count = 0;
static uint8_t gpio_out_count = 0;
static struct gpio_t gpio_in[80];
static struct gpio_t gpio_out[80];
static uint16_t gpio_params[80];

/* Set pin definitions, directions, etc */
usb_request_status_t usb_vendor_request_register_gpio(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t in_count, i;
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->out, &gpio_params,
									endpoint->setup.length/2, NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		in_count = endpoint->setup.value;
		for(i=0; i<in_count; i++) {
			GPIO_SET(gpio_in[gpio_in_count], (gpio_params[i]>>8)&0xFF, gpio_params[i]&0xFF);
			gpio_set(&gpio_in[gpio_in_count]);
			gpio_in_count++;
		}
		for(; i<endpoint->setup.length; i++) {
			GPIO_SET(gpio_out[gpio_out_count], (gpio_params[i]>>8)&0xFF, gpio_params[i]&0xFF);
			gpio_set(&gpio_out[gpio_out_count]);
			gpio_out_count++;
		}
		led_on(LED2);
		usb_transfer_schedule_ack(endpoint->in);
		if(gpio_out_count)
			start_gpio_monitor = true;
		led_on(LED3);
	}
	return USB_REQUEST_STATUS_OK;
}

/* Toggle out pins */
usb_request_status_t usb_vendor_request_write_gpio(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		;
		return USB_REQUEST_STATUS_OK;
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

void gpio_monitor_mode(void) {
	led_off(LED4);
	while(true) {
		led_off(LED3);
		delay(10000000);
		led_on(LED3);
		delay(10000000);
	}
}