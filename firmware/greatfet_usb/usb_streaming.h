
/**
 * Simple predecessor to the pipe API; currently being used until a few speed issues with the pipe API are fixed.
 * (We need to generate more machine code; yay.)
 *
 * This file is part of greatfet.
 */


#include <toolchain.h>
#include "usb_bulk_buffer.h"

#ifndef __GREATFET_USB_STREAMING_H__
#define __GREATFET_USB_STREAMING_H__

enum {
	USB_STREAMING_NUM_BUFFERS = 2,

	// FIXME: halve this when doing both in and out
	USB_STREAMING_BUFFER_SIZE    = 0x4000,
	USB_STREAMING_MIN_TRANSFER_SIZE = 32,

	USB_STREAMING_IN_ADDRESS  = 0x81,
	USB_STREAMING_OUT_ADDRESS = 0x02,
};

/**
 * Core USB streaming service routine: ferries data to or from the host.
 */
void service_usb_streaming(void);


/**
 * Sets up a task thread that will rapidly stream data to/from a USB host.
 */
void usb_streaming_start_streaming_to_host(uint32_t *volatile user_position_in_buffer,
	uint32_t *volatile user_data_in_buffer);


/**
 * Sets up a task thread that will rapidly stream data to/from a USB host.
 */
void usb_streaming_stop_streaming_to_host(void);

#endif
