/*
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

#include "usb_api_heartbeat.h"
#include <libopencm3/lpc43xx/rtc.h>

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"

#include "pins.h"

volatile bool heartbeat_mode_enabled = true;

#ifdef BOARD_CAPABILITY_RTC

/* Poll current RTC second and toggle the heartbeat LED if it has changed.
   This is called from the main loop.  It's polled instead of an ISR so that
   the LED will stop blinking if the main loop gets stuck. */
void heartbeat_mode(void) {
  static volatile uint32_t oldsec = 0;
  volatile uint32_t newsec = RTC_SEC;

  if (newsec != oldsec)
  {
    led_toggle(HEARTBEAT_LED);
    oldsec = newsec;
  }
}

#else
	/* Fall back to the old LED behavior for devices that don't support an RTC. */
	void heartbeat_mode(void) {
		led_toggle(HEARTBEAT_LED);
		delay(10000000);
	}
#endif

usb_request_status_t usb_vendor_request_heartbeat_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		heartbeat_mode_enabled = true;
    led_off(HEARTBEAT_LED);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_heartbeat_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		heartbeat_mode_enabled = false;
    led_off(HEARTBEAT_LED);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
