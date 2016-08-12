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
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		struct gpio_t this_gpio;
		GPIO_SET(this_gpio, (endpoint->setup.value >> 8) & 0xFF,
			endpoint->setup.value & 0xFF);

		gpio_output(&this_gpio);
		gpio_clear(&this_gpio);

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

/* Toggle out pins */
usb_request_status_t usb_vendor_request_write_gpio(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		struct gpio_t this_gpio;
		GPIO_SET(this_gpio, (endpoint->setup.value >> 8) & 0xFF,
			endpoint->setup.value & 0xFF);

		gpio_toggle(&this_gpio);

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
