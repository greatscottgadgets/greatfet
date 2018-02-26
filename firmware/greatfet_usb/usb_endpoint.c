/*
 * This file is part of GreatFET
 */

#include "usb_endpoint.h"

#include <usb_request.h>
#include <usb.h>

#include "usb_device.h"

usb_endpoint_t usb0_endpoint_control_out = {
	.address = 0x00,
	.device = usb_peripherals,
	.in = &usb0_endpoint_control_in,
	.out = &usb0_endpoint_control_out,
	.setup_complete = usb_setup_complete,
	.transfer_complete = usb_control_out_complete,
};
USB_DEFINE_QUEUE(usb0_endpoint_control_out, 4);

usb_endpoint_t usb0_endpoint_control_in = {
	.address = 0x80,
	.device = &usb_peripherals[0],
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
	.device = &usb_peripherals[0],
	.in = &usb0_endpoint_bulk_in,
	.out = 0,
	.setup_complete = 0,
	.transfer_complete = usb_queue_transfer_complete
};
static USB_DEFINE_QUEUE(usb0_endpoint_bulk_in, 1);

usb_endpoint_t usb0_endpoint_bulk_out = {
	.address = 0x02,
	.device = &usb_peripherals[0],
	.in = 0,
	.out = &usb0_endpoint_bulk_out,
	.setup_complete = 0,
	.transfer_complete = usb_queue_transfer_complete
};
static USB_DEFINE_QUEUE(usb0_endpoint_bulk_out, 1);

/* USB1 */
usb_endpoint_t usb1_endpoint_control_out = {
	.address = 0x00,
	.device = &usb_peripherals[1],
	.in = &usb1_endpoint_control_in,
	.out = &usb1_endpoint_control_out,
	.setup_complete = 0,
	.transfer_complete = 0,
};
USB_DEFINE_QUEUE(usb1_endpoint_control_out, 4);

usb_endpoint_t usb1_endpoint_control_in = {
	.address = 0x80,
	.device = &usb_peripherals[1],
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
	.device = &usb_peripherals[1],
	.in = &usb1_endpoint1_in,
	.out = 0,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint1_in, 1);

usb_endpoint_t usb1_endpoint1_out = {
	.address = 0x01,
	.device = &usb_peripherals[1],
	.in = 0,
	.out = &usb1_endpoint1_out,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint1_out, 1);


usb_endpoint_t usb1_endpoint2_in = {
	.address = 0x82,
	.device = &usb_peripherals[1],
	.in = &usb1_endpoint2_in,
	.out = 0,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint2_in, 1);

usb_endpoint_t usb1_endpoint2_out = {
	.address = 0x02,
	.device = &usb_peripherals[1],
	.in = 0,
	.out = &usb1_endpoint2_out,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint2_out, 1);


usb_endpoint_t usb1_endpoint3_in = {
	.address = 0x83,
	.device = &usb_peripherals[1],
	.in = &usb1_endpoint3_in,
	.out = 0,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint3_in, 1);

usb_endpoint_t usb1_endpoint3_out = {
	.address = 0x03,
	.device = &usb_peripherals[1],
	.in = 0,
	.out = &usb1_endpoint3_out,
	.setup_complete = 0,
	.transfer_complete = 0
};
static USB_DEFINE_QUEUE(usb1_endpoint3_out, 1);


