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

#include "usb_api_spi.h"
#include "usb_queue.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <i2c_bus.h>

uint8_t i2c_tx_buffer[255];
uint8_t i2c_rx_buffer[255];

usb_request_status_t usb_vendor_request_i2c_start(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	if ((stage == USB_TRANSFER_STAGE_SETUP)) {
		
		i2c_bus_start(&i2c0, &i2c_config_slow_clock);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_i2c_stop(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	if ((stage == USB_TRANSFER_STAGE_SETUP)) {
		
		i2c_bus_stop(&i2c0);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

/* wValue = slave address, wIndex = response length */
usb_request_status_t usb_vendor_request_i2c_xfer(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP)
	{
		usb_transfer_schedule_block(endpoint->out, i2c_tx_buffer,
									endpoint->setup.length, NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		i2c_bus_transfer(&i2c0, endpoint->setup.value & 0xff, i2c_tx_buffer,
						 endpoint->setup.length, i2c_rx_buffer,
						 endpoint->setup.index & 0xff);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_i2c_response(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP)
	{
		usb_transfer_schedule_block(endpoint->out, i2c_rx_buffer,
									endpoint->setup.length, NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
