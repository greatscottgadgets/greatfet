/*
 * This file is part of GreatFET
 */

#include "usb_api_spi.h"
#include "usb_queue.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <i2c_bus.h>
#include <debug.h>

// TODO: don't keep this independent buffer around;
// instead either convert this to a scatter-gather for
// USB transmission or otherwise transmit directly from
// the buffer
static char shuttle_buffer[2048];

/**
 * Vendor request to read a section of the device's debug ringbuffer.
 *
 * @param value: if 0, the data won't be cleared from the ringbuffer
 *      any other value clears the data as it's read
 */
usb_request_status_t usb_vendor_request_read_dmesg(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		unsigned int length = debug_ring_read(shuttle_buffer, sizeof(shuttle_buffer), endpoint->setup.value);
		usb_transfer_schedule_block(endpoint->in, shuttle_buffer, length, NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}
