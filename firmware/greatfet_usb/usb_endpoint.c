/*
 * Copyright 2012 Jared Boone
 * Copyright 2013 Benjamin Vernoux
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

#include "usb_endpoint.h"

#include <usb_request.h>

#include "usb_device.h"

usb_endpoint_t usb0_endpoint_control_out = {
	.address = 0x00,
	.device = &usb0_device,
	.in = &usb0_endpoint_control_in,
	.out = &usb0_endpoint_control_out,
	.setup_complete = usb_setup_complete,
	.transfer_complete = usb_control_out_complete,
};
USB_DEFINE_QUEUE(usb0_endpoint_control_out, 4);

usb_endpoint_t usb0_endpoint_control_in = {
	.address = 0x80,
	.device = &usb0_device,
	.in = &usb0_endpoint_control_in,
	.out = &usb0_endpoint_control_out,
	.setup_complete = 0,
	.transfer_complete = usb_control_in_complete,
};
static USB_DEFINE_QUEUE(usb0_endpoint_control_in, 4);

// NOTE: Endpoint number for IN and OUT are different. I wish I had some
// evidence that having BULK IN and OUT on separate endpoint numbers was
// actually a good idea. Seems like everybody does it that way, but why?

usb_endpoint_t usb0_endpoint_bulk_in = {
	.address = 0x81,
	.device = &usb0_device,
	.in = &usb0_endpoint_bulk_in,
	.out = 0,
	.setup_complete = 0,
	.transfer_complete = usb_queue_transfer_complete
};
static USB_DEFINE_QUEUE(usb0_endpoint_bulk_in, 1);

usb_endpoint_t usb0_endpoint_bulk_out = {
	.address = 0x02,
	.device = &usb0_device,
	.in = 0,
	.out = &usb0_endpoint_bulk_out,
	.setup_complete = 0,
	.transfer_complete = usb_queue_transfer_complete
};
static USB_DEFINE_QUEUE(usb0_endpoint_bulk_out, 1);

/* USB1 */
usb_endpoint_t usb1_endpoint_control_out = {
	.address = 0x00,
	.device = &usb1_device,
	.in = &usb1_endpoint_control_in,
	.out = &usb1_endpoint_control_out,
	.setup_complete = 0,
	.transfer_complete = 0,
};
USB_DEFINE_QUEUE(usb1_endpoint_control_out, 4);

usb_endpoint_t usb1_endpoint_control_in = {
	.address = 0x80,
	.device = &usb1_device,
	.in = &usb1_endpoint_control_in,
	.out = &usb1_endpoint_control_out,
	.setup_complete = 0,
	.transfer_complete = 0,
};
static USB_DEFINE_QUEUE(usb1_endpoint_control_in, 4);

//
// Define endpoint structures & queues for each of the GreatDancer
// endpoints, just in case we want to use them.
// TODO: macroize these?
//

usb_endpoint_t usb1_endpoint1_in = {
	.address = 0x81,
	.device = &usb1_device,
	.in = &usb1_endpoint1_in,
	.out = 0,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint1_in, 1);

usb_endpoint_t usb1_endpoint1_out = {
	.address = 0x01,
	.device = &usb1_device,
	.in = 0,
	.out = &usb1_endpoint1_out,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint1_out, 1);


usb_endpoint_t usb1_endpoint2_in = {
	.address = 0x82,
	.device = &usb1_device,
	.in = &usb1_endpoint2_in,
	.out = 0,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint2_in, 1);

usb_endpoint_t usb1_endpoint2_out = {
	.address = 0x02,
	.device = &usb1_device,
	.in = 0,
	.out = &usb1_endpoint2_out,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint2_out, 1);


usb_endpoint_t usb1_endpoint3_in = {
	.address = 0x83,
	.device = &usb1_device,
	.in = &usb1_endpoint3_in,
	.out = 0,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint3_in, 1);

usb_endpoint_t usb1_endpoint3_out = {
	.address = 0x03,
	.device = &usb1_device,
	.in = 0,
	.out = &usb1_endpoint3_out,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint3_out, 1);


