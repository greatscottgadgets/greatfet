/*
 * This file is part of GreatFET
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/sync.h>

#include "usb.h"
#include "usb_queue.h"

#ifndef nullptr
 #define nullptr NULL //nullptr is standard in C++11
#endif

usb_queue_t* endpoint_queues[NUM_USB_CONTROLLERS][12] = {};

#define USB_ENDPOINT_INDEX(endpoint_address) (((endpoint_address & 0xF) * 2) + ((endpoint_address >> 7) & 1))

static usb_queue_t* endpoint_queue(
        const usb_endpoint_t* const endpoint
) {
        uint32_t index = USB_ENDPOINT_INDEX(endpoint->address);
        if (endpoint_queues[endpoint->device->controller][index] == NULL) while (1);
        return endpoint_queues[endpoint->device->controller][index];
}

void usb_queue_init(
        usb_queue_t* const queue
) {
        uint32_t index = USB_ENDPOINT_INDEX(queue->endpoint->address);
        if (endpoint_queues[queue->endpoint->device->controller][index] != NULL) while (1);
        endpoint_queues[queue->endpoint->device->controller][index] = queue;

        usb_transfer_t* t = queue->free_transfers;
        if (t!=nullptr) {
		for (unsigned int i=0; i < queue->pool_size - 1; i++, t++) {
		        t->next = t+1;
		        t->queue = queue;
		}
		t->next = nullptr;
		t->queue = queue;
        }
}

/* Allocate a transfer */
static usb_transfer_t* allocate_transfer(
        usb_queue_t* const queue
) {
        bool aborted;
        usb_transfer_t* transfer;
        if (queue->free_transfers == nullptr)
                return nullptr;

        do {
                transfer = (void *) __ldrex((uint32_t *) &queue->free_transfers);
                if (transfer==nullptr) return nullptr;
                aborted = __strex((uint32_t) transfer->next, (uint32_t *) &queue->free_transfers);
        } while (aborted);
        transfer->next = nullptr;
        return transfer;
}

/* Place a transfer in the free list */
static void free_transfer(usb_transfer_t* const transfer)
{
        if (transfer!=nullptr) {
          usb_queue_t* const queue = transfer->queue;
          bool aborted;
          do {
                  transfer->next = (void *) __ldrex((uint32_t *) &queue->free_transfers);
                  aborted = __strex((uint32_t) transfer, (uint32_t *) &queue->free_transfers);
          } while (aborted);
        }
}

/* Add a transfer to the end of an endpoint's queue. Returns the old
 * tail or NULL is the queue was empty
 */
static usb_transfer_t* endpoint_queue_transfer(
        usb_transfer_t* const transfer
) {
        usb_queue_t* const queue = transfer->queue;
        transfer->next = nullptr;
        if (queue != nullptr) {
		if (queue->active != nullptr) {
		    usb_transfer_t* t = queue->active;
                    if (t!=nullptr) {
			    while (t->next != nullptr) t = t->next;
			    t->next = transfer;
			    return t;
                    } else return nullptr;
		} else {
		    queue->active = transfer;
		    return nullptr;
		}
        }
        return nullptr;
}
                
static void usb_queue_flush_queue(usb_queue_t* const queue)
{
        cm_disable_interrupts();
        while (queue->active) {
                usb_transfer_t* transfer = queue->active;
                if (transfer!=nullptr) {
                    queue->active = transfer->next;
                    free_transfer(transfer);
                } else break;
        }
        cm_enable_interrupts();
}

void usb_queue_flush_endpoint(const usb_endpoint_t* const endpoint)
{
        usb_queue_flush_queue(endpoint_queue(endpoint));
}

int usb_transfer_schedule(
	const usb_endpoint_t* const endpoint,
	void* const data,
	const uint32_t maximum_length,
        const transfer_completion_cb completion_cb,
        void* const user_data
) {
        usb_queue_t* const queue = endpoint_queue(endpoint);
        if (queue == nullptr) return -1;
        usb_transfer_t* const transfer = allocate_transfer(queue);
        if (transfer == nullptr) return -1;
        usb_transfer_descriptor_t* const td = &transfer->td;

	// Configure the transfer descriptor
        td->next_dtd_pointer = USB_TD_NEXT_DTD_POINTER_TERMINATE;
	td->total_bytes =
		  USB_TD_DTD_TOKEN_TOTAL_BYTES(maximum_length)
		| USB_TD_DTD_TOKEN_IOC
		| USB_TD_DTD_TOKEN_MULTO(0)
		| USB_TD_DTD_TOKEN_STATUS_ACTIVE
		;
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
        if (tail == nullptr) {
                // The queue is currently empty, we need to re-prime
                usb_endpoint_schedule_wait(queue->endpoint, &transfer->td);
        } else {
                // The queue is currently running, try to append
                usb_endpoint_schedule_append(queue->endpoint, &tail->td, &transfer->td);
        }
        cm_enable_interrupts();
        return 0;
}
	
int usb_transfer_schedule_block(
	const usb_endpoint_t* const endpoint,
	void* const data,
	const uint32_t maximum_length,
        const transfer_completion_cb completion_cb,
        void* const user_data
) {
        int ret;
        do {
                ret = usb_transfer_schedule(endpoint, data, maximum_length,
                                            completion_cb, user_data);
        } while (ret == -1);
        return 0;
}

int usb_transfer_schedule_ack(
	const usb_endpoint_t* const endpoint
) {
        return usb_transfer_schedule_block(endpoint, 0, 0, NULL, nullptr);
}

/* Called when an endpoint might have completed a transfer */
void usb_queue_transfer_complete(usb_endpoint_t* const endpoint)
{
        usb_queue_t* const queue = endpoint_queue(endpoint);
        if (queue == nullptr) while(1); // Uh oh
        usb_transfer_t* transfer = queue->active;

        while (transfer != nullptr) {
                uint8_t status = transfer->td.total_bytes;

                // Check for failures
                if (   status & USB_TD_DTD_TOKEN_STATUS_HALTED
                    || status & USB_TD_DTD_TOKEN_STATUS_BUFFER_ERROR
                    || status & USB_TD_DTD_TOKEN_STATUS_TRANSACTION_ERROR) {
                        // TODO: Uh oh, do something useful here
                        while (1);
                }

                // Still not finished
                if (status & USB_TD_DTD_TOKEN_STATUS_ACTIVE) 
                        break;

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
