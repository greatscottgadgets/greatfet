/**
 * Simple predecessor to the pipe API; currently being used until a few speed issues with the pipe API are fixed.
 * (We need to generate more machine code; yay.)
 *
 * This file is part of greatfet.
 */

#include <drivers/comms.h>

#include <drivers/sgpio.h>
#include <drivers/usb/usb.h>
#include <drivers/usb/usb_queue.h>
#include <drivers/platform_clock.h>

#include "usb_streaming.h"
#include "usb_bulk_buffer.h"
#include "usb_endpoint.h"


#include <errno.h>
#include <debug.h>
#include <toolchain.h>


static bool usb_streaming_enabled = false;
static unsigned int phase = 1;

static uint32_t *volatile position_in_buffer;
static uint32_t *volatile data_in_buffer;
volatile uint32_t debug_data;

uint32_t read_position;


// XXX
static inline void cm_enable_interrupts(void)
{
        __asm__("CPSIE I\n");
}

static inline void cm_disable_interrupts(void)
{
        __asm__("CPSID I\n");
}


/**
 * Schedules transmission of a completed logic-analyzer buffer.
 */
static int streaming_schedule_usb_transfer_in(int buffer_number)
{
	// We reach the overrun threshold if the logic analyzer has captured enough data to fill
	// every available buffer -- that is, every buffer except the one we're actively using to transmit.
	unsigned overrun_threeshold = (USB_STREAMING_NUM_BUFFERS- 1) * USB_STREAMING_BUFFER_SIZE;

	// If we don't have a full buffer of data to transmit, we can't send anything yet. Bail out.
	if (data_in_buffer && (*data_in_buffer < USB_STREAMING_BUFFER_SIZE)) {
		return EAGAIN;
	}

	// Otherwise, transmit the relevant (complete) buffer...
	usb_transfer_schedule_wait(
		&usb0_endpoint_bulk_in,
		&usb_bulk_buffer[buffer_number * USB_STREAMING_BUFFER_SIZE],
		USB_STREAMING_BUFFER_SIZE, 0, 0, 0);

	// ... and mark those samples as no longer pending transfer.
	if (data_in_buffer) {
		cm_disable_interrupts();
		*data_in_buffer -= USB_STREAMING_BUFFER_SIZE;
		cm_enable_interrupts();

		// Basic overrun detection: if we have more than our threshold remaining after
		// consuming a buffer (really, passing it to the USB hardware for transmission),
		// then we overran.
		if (*data_in_buffer > overrun_threeshold) {
			pr_error("logic analyzer: overrun detected (%u data writes to buffer)!\n", *data_in_buffer);
			usb_endpoint_stall(&usb0_endpoint_bulk_in);
		}
	}

	return 0;
}


static void service_usb_streaming_in(void)
{
	static unsigned int transfers = 0;
	int rc;

	if ((*position_in_buffer >= USB_STREAMING_BUFFER_SIZE) && phase == 1) {
		rc = streaming_schedule_usb_transfer_in(0);
		if(rc) {
			return;
		}

		phase = 0;

		++transfers;
	}

	if ((*position_in_buffer < USB_STREAMING_BUFFER_SIZE) && phase == 0) {
		rc = streaming_schedule_usb_transfer_in(1);
		if(rc) {
			return;
		}
		phase = 1;

		++transfers;
	}

	// Toggle the LED a bit to indicate progress.
	if ((transfers % 100) == 0) {
		led_toggle(LED4);
	}
}


/**
 * Core USB streaming service routine: ferries data to or from the host.
 */
void service_usb_streaming(void)
{
	if(!usb_streaming_enabled) {
		return;
	}

	// TODO: support USB streaming out, too
	service_usb_streaming_in();
}

/**
 * Sets up a task thread that will rapidly stream data to/from a USB host.
 */
void usb_streaming_start_streaming_to_host(uint32_t *volatile user_position_in_buffer,
	uint32_t *volatile user_data_in_buffer)
{
	usb_endpoint_init(&usb0_endpoint_bulk_in);

	// Store our references to the user variables to be updated.
	position_in_buffer = user_position_in_buffer;
	data_in_buffer     = user_data_in_buffer;

	phase = (*position_in_buffer > USB_STREAMING_BUFFER_SIZE) ? 0 : 1;

	// And enable USB streaming.
	// FIXME: support out streaming, too
	usb_streaming_enabled = true;
}



/**
 * Sets up a task thread that will rapidly stream data to/from a USB host.
 */
void usb_streaming_stop_streaming_to_host()
{
	usb_streaming_enabled = false;
	usb_endpoint_disable(&usb0_endpoint_bulk_in);

	pr_info("Position in buffer: %08x\n", *position_in_buffer);

	led_off(LED4);
}
