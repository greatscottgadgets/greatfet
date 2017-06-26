/*
 * Copyright 2015 Dominic Spill <dominicgs@gmail.com>
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
