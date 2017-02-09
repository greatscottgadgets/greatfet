/*
 * Copyright 2012 Jared Boone
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

#ifndef __USB_H__
#define __USB_H__

// TODO: Refactor to support high performance operations without having to
// expose usb_transfer_descriptor_t. Or usb_endpoint_prime(). Or, or, or...
#include <libopencm3/lpc43xx/usb.h>

#include "usb_type.h"

#define NUM_USB_CONTROLLERS 2
#define NUM_USB1_ENDPOINTS 4

void usb_peripheral_reset();

void usb_bus_reset(
	usb_device_t* const device
);

usb_queue_head_t* usb_queue_head(
	const uint_fast8_t endpoint_address,
	const usb_device_t* const device
);


usb_endpoint_t* usb_endpoint_from_address(
	const uint_fast8_t endpoint_address,
	const usb_device_t* const device
);

uint_fast8_t usb_endpoint_address(
	const usb_transfer_direction_t direction,
	const uint_fast8_t number
);

void usb_device_init(
	usb_device_t* const device
);

void usb_controller_reset(
	usb_device_t* const device
);


void usb_controller_run(
	const usb_device_t* const device
);

void usb_run(
	usb_device_t* const device
);

void usb_run_tasks(
	const usb_device_t* const device
);

usb_speed_t usb_speed(
	const usb_device_t* const device
);

uint32_t usb_get_status(
	const usb_device_t* const device
);

uint32_t usb_get_endpoint_setup_status(
	const usb_device_t* const device
);

void usb_clear_endpoint_setup_status(
	const uint32_t endpoint_setup_status,
	const usb_device_t* const device
);


uint32_t usb_get_endpoint_ready(
	const usb_device_t* const device
);

uint32_t usb_get_endpoint_complete(
	const usb_device_t* const device
);

void usb_clear_endpoint_complete(
	const uint32_t endpoint_complete,
	const usb_device_t* const device
);

void usb_set_address_immediate(
	const usb_device_t* const device,
	const uint_fast8_t address
);

void usb_set_address_deferred(
	const usb_device_t* const device,
	const uint_fast8_t address
);

void usb_endpoint_init_without_descriptor(
	const usb_endpoint_t* const endpoint,
  uint_fast16_t max_packet_size,
  usb_transfer_type_t transfer_type
);

void usb_endpoint_init(
	const usb_endpoint_t* const endpoint
);

void usb_endpoint_stall(
	const usb_endpoint_t* const endpoint
);

void usb_endpoint_disable(
	const usb_endpoint_t* const endpoint
);

void usb_endpoint_flush(
	const usb_endpoint_t* const endpoint
);

bool usb_endpoint_is_ready(
	const usb_endpoint_t* const endpoint
);

void usb_endpoint_prime(
	const usb_endpoint_t* const endpoint,
	usb_transfer_descriptor_t* const first_td
);

void usb_endpoint_schedule_wait(
	const usb_endpoint_t* const endpoint,
        usb_transfer_descriptor_t* const td
);

void usb_endpoint_schedule_append(
        const usb_endpoint_t* const endpoint,
        usb_transfer_descriptor_t* const tail_td,
        usb_transfer_descriptor_t* const new_td
);

#endif//__USB_H__
