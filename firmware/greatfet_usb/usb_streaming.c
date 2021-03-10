/**
 * Simple predecessor to the pipe API; currently being used until a few speed issues with the pipe API are fixed.
 * (We need to generate more machine code; yay.)
 *
 * This file is part of greatfet.
 */


#include <errno.h>
#include <debug.h>
#include <string.h>
#include <toolchain.h>

#include <drivers/comms.h>

#include <drivers/sgpio.h>
#include <drivers/usb/usb.h>
#include <drivers/usb/usb_queue.h>
#include <drivers/platform_clock.h>

#include "usb_streaming.h"
#include "usb_bulk_buffer.h"
#include "usb_endpoint.h"

#include "greatfet_core.h"



typedef void (*streaming_handler_t)(void);


static bool usb_streaming_enabled = false;
static unsigned int phase = 1;

static volatile uint32_t *position_in_buffer;
static volatile uint32_t *data_in_buffer;
volatile uint32_t debug_data;

// Abstract storage for buffer metadata. Used for functionality in which we manage the buffer,
// rather than the caller.
volatile uint32_t read_position;
volatile uint32_t write_position;
volatile uint32_t buffer_content_count;

// Timer objects used by our "periodic upload" functionality.
hw_timer_t periodic_event_timer;


// XXX
static inline void cm_enable_interrupts(void)
{
        __asm__("CPSIE I\n");
}

static inline void cm_disable_interrupts(void)
{
        __asm__("CPSID I\n");
}


static int streaming_detect_overrun(void)
{
	// We reach the overrun threshold if the logic analyzer has captured enough data to fill
	// every available buffer -- that is, every buffer except the one we're actively using to transmit.
	uint32_t overrun_threeshold = USB_STREAMING_NUM_BUFFERS * USB_STREAMING_BUFFER_SIZE;

	if (!data_in_buffer) {
		return 0;
	}


	// Basic overrun detection: if we have more than our threshold remaining after
	// consuming a buffer (really, passing it to the USB hardware for transmission),
	// then we overran.
	if (*data_in_buffer > overrun_threeshold) {
		pr_debug("streaming: debug: host isn't reading from us (possible overflow) -- stalling endpoint\n");
		usb_endpoint_stall(&usb0_endpoint_bulk_in);

		// Tentative: stop streaming if we ever overrun.
		usb_streaming_stop_streaming_to_host();
		return EOVERFLOW;
	}

	return 0;
}


/**
 * Schedules transmission of a completed logic-analyzer buffer.
 */
static int streaming_schedule_usb_transfer_in(int buffer_number)
{
	int rc;


	// If we don't have a full buffer of data to transmit, we can't send anything yet. Bail out.
	if (data_in_buffer && (*data_in_buffer < USB_STREAMING_BUFFER_SIZE)) {
		return EAGAIN;
	}


	if (streaming_detect_overrun()) {
		return EOVERFLOW;
	}


	// Otherwise, transmit the relevant (complete) buffer...
	rc = usb_transfer_schedule(
		&usb0_endpoint_bulk_in,
		&usb_bulk_buffer[buffer_number * USB_STREAMING_BUFFER_SIZE],
		USB_STREAMING_BUFFER_SIZE, 0, 0);
	if (rc) {
		return EAGAIN;
	}

	// ... and mark those samples as no longer pending transfer.
	if (data_in_buffer) {
		cm_disable_interrupts();
		*data_in_buffer -= USB_STREAMING_BUFFER_SIZE;
		cm_enable_interrupts();

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
	else if ((*position_in_buffer < USB_STREAMING_BUFFER_SIZE) && phase == 0) {
		rc = streaming_schedule_usb_transfer_in(1);
		if(rc) {
			return;
		}
		phase = 1;

		++transfers;
	}
	else {
		streaming_detect_overrun();
		return;
	}


	// Toggle the LED a bit to indicate progress.
	if ((transfers % 200) == 0) {
		led_toggle(LED4);
	}
}


/**
 * Sets up a task thread that will rapidly stream data to/from a USB host.
 */
void usb_streaming_start_streaming_to_host(volatile uint32_t *user_position_in_buffer,
	volatile uint32_t *user_data_in_buffer)
{
	usb_endpoint_init(&usb0_endpoint_bulk_in);
	usb_endpoint_clear_stall(&usb0_endpoint_bulk_in);

	// Store our references to the user variables to be updated.
	position_in_buffer = user_position_in_buffer;
	data_in_buffer     = user_data_in_buffer;

	phase = (*position_in_buffer > USB_STREAMING_BUFFER_SIZE) ? 0 : 1;

	// And enable USB streaming.
	// FIXME: support out streaming, too
	usb_streaming_enabled = true;
}


/**
 * Sets up a task thread that will periodically call a callback, and then deliver the collected
 * data to the host.
 */
uint32_t usb_streaming_start_periodic_data_gathering(uint32_t frequency, timer_callback_t callback,
	void *callback_argument)
{
	// Allocate a periodic event timer...
	int rc = acquire_timer(&periodic_event_timer);
	if (rc) {
		pr_error("error: streaming: could not allocate a timer for periodic events (%u)!\n", rc);
		return rc;
	}

	// ... set up our USB streaming ...
	read_position = 0;
	write_position = 0;
	buffer_content_count = 0;
	usb_endpoint_init(&usb0_endpoint_bulk_in);

	// ... and enable data gathering.
	call_function_periodically(&periodic_event_timer, frequency, callback, callback_argument);
	return 0;
}



/**
 * Sets up a task thread that will rapidly stream data to/from a USB host.
 */
void usb_streaming_stop_streaming_to_host()
{
	usb_streaming_enabled = false;
	usb_endpoint_disable(&usb0_endpoint_bulk_in);

	led_off(LED4);
}


/**
 * Halts a periodic data gathering request.
 */
void usb_streaming_stop_periodic_gathering(void)
{
	cancel_periodic_function_calls(&periodic_event_timer);
	release_timer(&periodic_event_timer);
}



static uint32_t usb_streaming_submit_data(void *data_in, uint32_t count)
{
	uint32_t original_position_in_buffer = write_position;

	uint8_t *data_in_window = data_in;
	uint32_t data_after_start = 0;

	// Compute the vital statistics of our buffer.
	const uint32_t buffer_size = sizeof(usb_bulk_buffer);

	// Figure out how much data we want to place directly follwing the current
	// write pointer.
	uint32_t space_remaining_to_end = buffer_size - write_position;
	uint32_t data_after_pointer = count;

	// If we'd wrap around...
	if (count > space_remaining_to_end) {

		//... copy from the write pointer to the end...
		data_after_pointer = space_remaining_to_end;
		count -= space_remaining_to_end;

		//  and copy the remaining data to the beginning.
		data_after_start = count;
	}


	// Update our write pointer...
	write_position = (write_position + count) % buffer_size;
	buffer_content_count += count;

	// ... and perform the copy itself.
	memcpy(&usb_bulk_buffer[original_position_in_buffer], data_in, data_after_pointer);
	memcpy(&usb_bulk_buffer[0], &data_in_window[data_after_pointer], data_after_start);

	return 0;
}


/**
 * Submit data into the user buffer for streaming, and schedule a USB transfer to gather
 * the relevant data iff a transfer is available.
 */
uint32_t usb_streaming_send_data(void *data_in, uint32_t count)
{
	int rc;

	uint32_t data_to_send;
	uint32_t bytes_to_end_of_buffer;

	// Add the data to our buffer...
	usb_streaming_submit_data(data_in, count);

	// Send either the amount of data we have available, or the bytes to the end of the buffer, whichever is less.
	bytes_to_end_of_buffer = sizeof(usb_bulk_buffer) - read_position;
	data_to_send = buffer_content_count < bytes_to_end_of_buffer ? buffer_content_count : bytes_to_end_of_buffer;

	// Try to schedule a USB transfer.
	rc = usb_transfer_schedule(
		&usb0_endpoint_bulk_in,
		&usb_bulk_buffer[read_position],
		data_to_send, NULL, NULL);


	// If we succeeded in scheduling a transfer, advance the read pointer.
	if (!rc) {
		read_position = (read_position + data_to_send) % sizeof(usb_bulk_buffer);
		buffer_content_count -= data_to_send;
	}

	return 0;
}



/**
 * Core USB streaming service routine: ferries data to or from the host.
 */
void task_usb_streaming(void)
{
	if(!usb_streaming_enabled) {
		return;
	}

	// TODO: support USB streaming out, too
	service_usb_streaming_in();
}

DEFINE_TASK(task_usb_streaming);
