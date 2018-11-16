/*
 * This file is part of GreatFET
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <debug.h>

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/sync.h>

#include <drivers/usb/lpc43xx/usb.h>
#include <drivers/usb/lpc43xx/usb_queue.h>

// FIXME abstract:
#define USB_ALLOC_TIMEOUT_DEFAULT_US (1000000UL)

usb_queue_t* endpoint_queues[NUM_USB_CONTROLLERS][12] = {};

#define USB_ENDPOINT_INDEX(endpoint_address) (((endpoint_address & 0xF) * 2) + ((endpoint_address >> 7) & 1))

static usb_queue_t* endpoint_queue(
		const usb_endpoint_t* const endpoint
) {
		uint32_t index = USB_ENDPOINT_INDEX(endpoint->address);
		if (endpoint_queues[endpoint->device->controller][index] == NULL) {
			pr_error("usb error: could not find a Queue for endpoint %d!", endpoint->address);
			return NULL;
		}
		return endpoint_queues[endpoint->device->controller][index];
}

void usb_queue_init(
		usb_queue_t* const queue
) {
		uint32_t index = USB_ENDPOINT_INDEX(queue->endpoint->address);
		if (endpoint_queues[queue->endpoint->device->controller][index] != NULL) {
			pr_error("usb error: could not initialize queue for endpoint %d!", queue->endpoint->address);
			return;
		}
		endpoint_queues[queue->endpoint->device->controller][index] = queue;

		usb_transfer_t* t = queue->free_transfers;
		for (unsigned int i=0; i < queue->pool_size - 1; i++, t++) {
				t->next = t+1;
				t->queue = queue;
		}
		t->next = NULL;
		t->queue = queue;
}

/* Allocate a transfer */
static usb_transfer_t* allocate_transfer(
		usb_queue_t* const queue
) {
		bool aborted;
		usb_transfer_t* transfer;
		if (queue->free_transfers == NULL)
				return NULL;

		do {
				transfer = (void *) __ldrex((uint32_t *) &queue->free_transfers);
				aborted = __strex((uint32_t) transfer->next, (uint32_t *) &queue->free_transfers);
		} while (aborted);

		transfer->next = NULL;
		return transfer;
}

/* Place a transfer in the free list */
static void free_transfer(usb_transfer_t* const transfer)
{
		usb_queue_t* const queue = transfer->queue;
		bool aborted;
		do {
				transfer->next = (void *) __ldrex((uint32_t *) &queue->free_transfers);
				aborted = __strex((uint32_t) transfer, (uint32_t *) &queue->free_transfers);
		} while (aborted);
}

/* Add a transfer to the end of an endpoint's queue. Returns the old
 * tail or NULL is the queue was empty
 */
static usb_transfer_t* endpoint_queue_transfer(
		usb_transfer_t* const transfer
) {
		usb_queue_t* const queue = transfer->queue;
		transfer->next = NULL;
		if (queue->active != NULL) {
			usb_transfer_t* t = queue->active;
			while (t->next != NULL) t = t->next;
			t->next = transfer;
			return t;
		} else {
			queue->active = transfer;
			return NULL;
		}
}

int usb_transfer_schedule(
	const usb_endpoint_t* const endpoint,
	void* const data,
	const uint32_t maximum_length,
		const transfer_completion_cb completion_cb,
		void* const user_data
) {
		usb_queue_t* const queue = endpoint_queue(endpoint);
		usb_transfer_t* const transfer = allocate_transfer(queue);
		if (transfer == NULL)
			return ENOSPC;

		usb_transfer_descriptor_t* const td = &transfer->td;

	// Configure the transfer descriptor
		td->next_dtd_pointer = USB_TD_NEXT_DTD_POINTER_TERMINATE;
	td->total_bytes =
		  USB_TD_DTD_TOKEN_TOTAL_BYTES(maximum_length)
		| USB_TD_DTD_TOKEN_IOC
		| USB_TD_DTD_TOKEN_MULTO(0)
		| USB_TD_DTD_TOKEN_STATUS_ACTIVE ;
	td->buffer_pointer_page[0] =  (uint32_t)data;
	td->buffer_pointer_page[1] = ((uint32_t)data + 0x1000) & 0xfffff000;
	td->buffer_pointer_page[2] = ((uint32_t)data + 0x2000) & 0xfffff000;
	td->buffer_pointer_page[3] = ((uint32_t)data + 0x3000) & 0xfffff000;
	td->buffer_pointer_page[4] = ((uint32_t)data + 0x4000) & 0xfffff000;

		// Fill in transfer fields
		transfer->maximum_length = maximum_length;
		transfer->completion_cb = completion_cb;
		transfer->user_data = user_data;

		cm_disable_interrupts();
		usb_transfer_t* tail = endpoint_queue_transfer(transfer);
		if (tail == NULL) {
				// The queue is currently empty, we need to re-prime
				usb_endpoint_schedule_wait(queue->endpoint, &transfer->td);
		} else {
				// The queue is currently running, try to append
				usb_endpoint_schedule_append(queue->endpoint, &tail->td, &transfer->td);
		}
		cm_enable_interrupts();
		return 0;
}

int usb_transfer_schedule_wait(
	const usb_endpoint_t* const endpoint,
	void* const data,
	const uint32_t maximum_length,
	const transfer_completion_cb completion_cb,
	void* const user_data,
	uint32_t timeout
)
{
		uint32_t time_base = get_time();
		int ret = -1;

		while (ret) {

			// Repeatedly try to schedule a transfer until one succeeds or we time out.
			ret = usb_transfer_schedule(endpoint,
					data, maximum_length, completion_cb, user_data);

			// And enforce our timeout.
			if (get_time_since(time_base) > timeout) {
				return ETIMEDOUT;
			}
		}
		return 0;
}


int usb_transfer_schedule_block(
	const usb_endpoint_t* const endpoint,
	void* const data,
	const uint32_t maximum_length,
	const transfer_completion_cb completion_cb,
	void* const user_data
)
{
	return usb_transfer_schedule_wait(endpoint, data, maximum_length,
			completion_cb, user_data, USB_ALLOC_TIMEOUT_DEFAULT_US);
}

int usb_transfer_schedule_ack(
	const usb_endpoint_t* const endpoint
) {
		return usb_transfer_schedule_block(endpoint, 0, 0, NULL, NULL);
}



/* Called when an endpoint might have completed a transfer */
static void usb_queue_clean_up_transfers(usb_endpoint_t const * endpoint, bool include_active)
{
		usb_queue_t* const queue = endpoint_queue(endpoint);
		if (queue == NULL) {
			pr_error("usb error: tried to clean up an endpoint (%d) with no queue!\n", endpoint->address);
		}

		usb_transfer_t* transfer = queue->active;

		while (transfer != NULL) {
				uint8_t status = transfer->td.total_bytes;
				bool aborting = false;

				bool td_is_active = (status & USB_TD_DTD_TOKEN_STATUS_ACTIVE);

				// Check for failures
				if (status & USB_TD_DTD_TOKEN_STATUS_HALTED) {
					pr_error("usb error:transaction reports halted status! aborting.\n");
					aborting = true;
				}
				if (status & USB_TD_DTD_TOKEN_STATUS_BUFFER_ERROR) {
					pr_error("usb error:transaction reports buffer error! aborting.\n");
					aborting = true;
				}
				if (status & USB_TD_DTD_TOKEN_STATUS_TRANSACTION_ERROR) {
					pr_error("usb error:transaction reports transaction error! aborting.\n");
					aborting = true;
				}

				// TODO: check for timeout, or NAK count, or etc?

				// If we're aborting due to an error, the TD is effetively non-active.
				if (aborting) {
					td_is_active = false;
				}

				// If this in active, non-error'd transaction, ignore it.
				if (td_is_active) {
					if (!include_active && td_is_active) {
						break;
					} else {
						// TODO: abstract me
						int number = endpoint->address & 0x7f;
						const char *direction = endpoint->address & 0x80 ? "IN" : "OUT";

						pr_info("usb stack: discarding an active transcation on EP%d:%s!\n", number, direction);
						pr_info("usb stack: (discard was likely due to overriding SETUP or STALL)\n");
					}
				}

				// FIXME: add in an error callback, which should do the below instead of us
				// in the event of an error!

				// Advance the head. We need to do this before invoking the completion
				// callback as it might attempt to schedule a new transfer
				queue->active = transfer->next;
				usb_transfer_t* next = transfer->next;

				// Invoke completion callback
				unsigned int total_bytes = (transfer->td.total_bytes & USB_TD_DTD_TOKEN_TOTAL_BYTES_MASK) >> USB_TD_DTD_TOKEN_TOTAL_BYTES_SHIFT;
				unsigned int transferred = transfer->maximum_length - total_bytes;
				if (transfer->completion_cb)
						transfer->completion_cb(transfer->user_data, transferred);

				// Advance head and free transfer
				free_transfer(transfer);
				transfer = next;
		}
}


void usb_queue_transfer_complete(usb_endpoint_t* const endpoint)
{
	usb_queue_clean_up_transfers(endpoint, false);
}

void usb_queue_flush_endpoint(const usb_endpoint_t* const endpoint)
{
		cm_disable_interrupts();
		usb_queue_clean_up_transfers(endpoint, true);
		cm_enable_interrupts();
}

