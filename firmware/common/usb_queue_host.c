/*
 * This file is part of GreatFET
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "usb.h"
#include "usb_host.h"
#include "usb_queue_host.h"

#include "greatfet_core.h"

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/sync.h>

// Storage pools for re-usable USB objects.
static ehci_link_t queue_head_freelist;
static ehci_link_t transfer_freelist;

static ehci_queue_head_t queue_head_pool[USB_HOST_MAX_QUEUE_HEADS];
static ehci_transfer_t transfer_pool[USB_HOST_MAX_TRANSFER_DESCRIPTORS];

// TODO: Figure out how to handle locking on the below, if we ever wind up
// in a situation where things can be allocated or freed from different contexts
// (e.g. the mainloop allocating things while an interrupt context cleans up).

/**
 * Initializes the storage pools for USB objects. This essentially reclaims all
 * of the existing structures, so it should only be called when all of them are
 * free, or the controller hasn't been initialized.
 *
 * It's recommended to use this only on device initialization.
 */
void usb_host_initialize_storage_pools(void)
{
	// Start off with all of the elements in the freelist.

	// The freelist should start with the first element...
	queue_head_freelist.ptr = &queue_head_pool->horizontal;
	transfer_freelist.ptr = &transfer_pool->horizontal;

	// ... link each of the elements together...
	for(int i = 0; i < USB_HOST_MAX_QUEUE_HEADS - 1; ++i) {
		queue_head_pool[i].horizontal.ptr = &(queue_head_pool[i+1].horizontal);
	}
	for(int i = 0; i < USB_HOST_MAX_TRANSFER_DESCRIPTORS - 1; ++i) {
		transfer_pool[i].horizontal.ptr = &(transfer_pool[i+1].horizontal);
	}
	
	// ... terminating at the end of the list.
	queue_head_pool[USB_HOST_MAX_QUEUE_HEADS - 1].horizontal.ptr = TERMINATING_LINK;
	transfer_pool[USB_HOST_MAX_QUEUE_HEADS - 1].horizontal.ptr = TERMINATING_LINK;
}

/**
 * Core allocator for the freelist/pool allocator. Simply grabs the first element
 * from a freelist-- keeping things simple.
 */
static ehci_link_t * usb_host_allocate_from_freelist(ehci_link_t *freelist_head)
{
	ehci_link_t *allocated;

	// If the terminate is set on the freelist head, we're out of things
	// to allocate. Return NULL.
	if(freelist_head->terminate) {
		return NULL;
	}

	// Otherwise, grab the first element in the freelist, and advance
	// the freelist.
	allocated = freelist_head->ptr;
	freelist_head->ptr = allocated->ptr;

	// For now, clear out the allocated element to ensure we don't re-use its
	// address.
	allocated->link = 0;
	allocated->terminate = 1;

	// Return the allocated element.
	return allocated;
}


/**
 * Core deallcoator for the freelist/pool allocator. Simply sticks the relevant
 * object at the head of the freelist.
 */
static void usb_host_add_to_list(ehci_link_t *list_head, ehci_link_t *link)
{
	// Take the given node, and point it to the next element on the freelist.
	*link = *list_head;

	// Next, point the freelist head to the given node.
	list_head->ptr = link;
}


/**
 * Allocates a queue head from the pool of available queue heads, if possible.
 *
 * @return A queue head which should be freed back to the pool when no longer
 *		used with usb_host_free_queue_head.
 */
ehci_queue_head_t * usb_host_allocate_queue_head(void)
{
	return (ehci_queue_head_t *)usb_host_allocate_from_freelist(&queue_head_freelist);
}


/**
 * Frees a queue head, returning it to the pool of available queue heads.
 *
 * @param to_free The queue head to be freed.
 */
void usb_host_free_queue_head(ehci_queue_head_t *to_free)
{
	usb_host_add_to_list(&queue_head_freelist, (ehci_link_t *)to_free);
}


/**
 * Allocates a transfer descriptor from the pool of available transfer descriptors, if possible.
 *
 * @return A transfer descriptor which should be freed back to the pool when no longer
 *		used with usb_host_free_queue_head.
 */
ehci_transfer_t * usb_host_allocate_transfer(void)
{
	// Perform the core allocation.
	ehci_transfer_t * transfer = (ehci_transfer_t *)usb_host_allocate_from_freelist(&transfer_freelist);

	// ... and clear out core of transfer, if we have one.
	memset(&transfer->td, 0, sizeof(transfer->td));

	return transfer;
}


/**
 * Frees a transfer descriptor, returning it to the pool of available transfer descriptors.
 *
 * @param to_free The transfer descriptor to be freed.
 */
void usb_host_free_transfer(ehci_transfer_t *to_free)
{
	usb_host_add_to_list(&transfer_freelist, (ehci_link_t*)to_free);
}


/**
 * Adds a given transfer to a list of pending transfers for the given host.
 */
void usb_host_add_transfer_to_pending_list(usb_peripheral_t *host, ehci_transfer_t *transfer)
{
	usb_host_add_to_list(&host->pending_transfers, (ehci_link_t*)transfer);
}


/**
 * Converts a usb_speed_t into the endpoint speed bits specified in the EHCI spec.
 */
static uint8_t _get_endpoint_speed_bits(usb_speed_t endpoint_speed)
{
	switch(endpoint_speed) {
		case USB_SPEED_FULL: return 0;
		case USB_SPEED_LOW:  return 1;
		case USB_SPEED_HIGH: return 2;
		default: return -1;
	}

	return -1;
}


static void usb_host_initialize_queue_head(ehci_queue_head_t *qh,
		uint8_t device_address, uint8_t endpoint_number, usb_speed_t endpoint_speed,
		bool is_control_endpoint, uint16_t max_packet_size)
{
	// Set up the parameters for the queue head.
	// See the documentation in docs, and the EHCI specification section 3.6.
	qh->device_address = device_address;
	qh->inactive_on_next_transaction = 0;
	qh->endpoint_number = endpoint_number;
	qh->endpoint_speed = _get_endpoint_speed_bits(endpoint_speed);
	qh->data_toggle_control = 0;
	qh->head_reclamation_flag = 0;
	qh->max_packet_length = max_packet_size & 0x7FF;

	if(endpoint_speed == USB_SPEED_HIGH) {
		qh->control_endpoint_flag = 0;
	} else {
		qh->control_endpoint_flag = is_control_endpoint ? 1 : 0;
	}

	qh->nak_count_reload = 0; // TODO: maybe fix me?

	// TODO: support periodic endpoints!
	qh->uframe_smask = 0;
	qh->uframe_cmask = 0;

	// TODO: Validate if we may want these to be otherwise in the future?
	qh->hub_address = 0;
	qh->port_number = 0;

	// FIXME: Support values here for high-speed!
	qh->mult = 0;
	qh->overlay.ping_state_err = 0;

	qh->overlay.next_dtd_pointer = (ehci_transfer_descriptor_t *)TERMINATING_LINK;
	qh->overlay.alternate_next_dtd_pointer = (ehci_transfer_descriptor_t *)TERMINATING_LINK;
}


/**
 * Sets up an endpoint for use in issuing USB transactions. This can be used
 * for any endpoint on the asynchronous queue (e.g. not interrupt or iso).
 *
 * Intended to be used internally to the endpoint API.
 *
 * @param device_address The address of the downstream device.
 * @param endpoint_number The endpoint number of the endpoint being configurd,
 *		_not_ including the direction bit.
 * @param endpoint_speed The speed of the endpoint. Should match the speed of
 *		the attached device.
 * @param is_control_endpoint True iff the endpoint is a control endpoint.
 * @param max_packet_size The maximum packet size transmissable on the endpoint;
 *		up to 1024.
 *
 * @return The queue set up for the given endpoint, or NULL if we couldn't set up a queue.
 */
ehci_queue_head_t * set_up_asynchronous_endpoint_queue(usb_peripheral_t *host, uint8_t device_address,
		uint8_t endpoint_number, usb_speed_t endpoint_speed,
		bool is_control_endpoint, uint16_t max_packet_size)
{
	ehci_queue_head_t *qh = usb_host_allocate_queue_head();

	if(!qh)
		return NULL;

	// Set up the Queue Head object for use...
	usb_host_initialize_queue_head(qh, device_address, endpoint_number,
			endpoint_speed, is_control_endpoint, max_packet_size);

	// Add the new queue head to the asynchronous queue, ensuring the schedule
	// is disabled as we're modifying its innards.
	usb_host_disable_asynchronous_schedule(host);
	qh->horizontal.link = host->async_queue_head.horizontal.link;
	host->async_queue_head.horizontal.ptr = &qh->horizontal;
	host->async_queue_head.horizontal.type = DESCRIPTOR_QH;
	usb_host_enable_asynchronous_schedule(host);

	return qh;
}

// TODO: code to tear down an asynchronous endpoint!

static inline bool dtd_link_is_nonterminating(volatile ehci_transfer_descriptor_t * link)
{
	return !((uintptr_t)link & 0x1);
}

/**
 * Schedule a USB transfer on the hosts's asynchronous queue.
 * This will execute as soon as the hardware can.
 *
 * FIXME: Possibly use an endpoint abstaction rather than passing around QHs?
 * @param qh The queue head to schedule the given transfer on.
 * @param pid_code The PID code to use for the given transfer. Sets direction.
 * @param data A pointer to the data buffer to be transmitted from or recieved into,
 *      per the PID code provided.
 * @param maximum_length The length of the data to be transmitted _or_ the maximum length
 *      to be recieved.
 *
 * @param completion_cb If non-NULL, this is a callback that will be executed
 *      (from interrupt context!) once the transfer is complete or stalled.
 * @param user_data A value to be provided to the completion_cb function.
 *
 * @return 0 on success, or an error code on failure.
 */
int usb_host_transfer_schedule(
	const usb_peripheral_t *host,
	const ehci_queue_head_t *qh,
	const usb_token_t pid_code,

	void* const data,
	const uint32_t maximum_length,
	const host_transfer_completion_cb completion_cb,
	void* const user_data
) {

	// Allocate a transfer object for the given transfer.
	ehci_transfer_t* const transfer = usb_host_allocate_transfer();

	// Fail out if we couldn't get a transfer.
	if (transfer == NULL) {
		return -1; // FIXME: error codes
	}

	// Get a reference to the core transfer descriptor used by the hardware.
	ehci_transfer_descriptor_t* const td = &transfer->td;

	// Populate it with the meta-data used to configure the hardware...
	td->next_dtd_pointer					 = (ehci_transfer_descriptor_t *)TERMINATING_LINK;
	td->alternate_next_dtd_pointer = (ehci_transfer_descriptor_t *)TERMINATING_LINK;
	td->total_bytes								 = maximum_length;
	td->active										 = 1;
	td->pid_code									 = pid_code;

	// Request an interrupt on complete. This allows us to clean things up and
	// execute the completion callback.
	td->int_on_complete						 = 1;

	// ... and provide the addresses the DMA controller will use to access the
	// data source or target.
	td->buffer_pointer_page[0] = (uint32_t)data;
	td->buffer_pointer_page[1] = ((uint32_t)data + 0x1000) & 0xfffff000;
	td->buffer_pointer_page[2] = ((uint32_t)data + 0x2000) & 0xfffff000;
	td->buffer_pointer_page[3] = ((uint32_t)data + 0x3000) & 0xfffff000;
	td->buffer_pointer_page[4] = ((uint32_t)data + 0x4000) & 0xfffff000;

	// Fill in the fields we'll use when the transfer completes.
	transfer->completion_cb  = completion_cb;
	transfer->user_data			 = user_data;
	transfer->maximum_length = maximum_length;

	// Finally, we're ready to add our transfer to the relevant queue head.
	ehci_transfer_descriptor_t *tail;

	// Add the transfer to our list of pending transfers.
	// This is what we'll use to know when to clean up the transfer.
	usb_host_add_transfer_to_pending_list(host, transfer);

	cm_disable_interrupts();

	// Iterate until we find a link that has the Terminate bit set, and then
	// add our new transfer descriptor there.
	tail = (ehci_transfer_descriptor_t *)&qh->overlay;
	while(dtd_link_is_nonterminating(tail->next_dtd_pointer)) {
			tail = (ehci_transfer_descriptor_t *)tail->next_dtd_pointer;
	}
	tail->next_dtd_pointer = td;


	cm_enable_interrupts();
	return 0;
}


/**
 * Determines the address of the next link in an EHCI-style list.
 *
 * @return The link, or NULL if there is no link following this one.
 */
static ehci_link_t *next_link(ehci_link_t *link)
{
	uint32_t raw_pointer;

	// If this is a terminating link, return NULL,
	// which has the same semantic meaning as our terminator. :)
	if(link->terminate) {
			return NULL;
	}

	// Get a manipulable reference to the next QH...
	raw_pointer = (uintptr_t)link->link;

	// ... mask away the non-pointer bits....
	raw_pointer &= ~0b11111;

	// ... and convert it back into a pointer.
	return (ehci_link_t *)raw_pointer;
}


/**
 * Handle completion of an asynchronous transfer. This is automatically called
 * from the default interrupt handler when a scheudled host transfer completes.
 */
void usb_host_handle_asynchronous_transfer_complete(usb_peripheral_t *host)
{
	// Iterate over each transfer type
	ehci_link_t *previous = &host->pending_transfers;
	ehci_link_t *link     = next_link(previous);

	// Iterate over each element on the pending list.
	while (link) {

		// Expand our pointer to get the full transfer.
		ehci_transfer_t *transfer = (ehci_transfer_t *)link;

		// If this transfer is complete...
		if(!transfer->td.active) {

			// If we have a completion callback, call it.
			if(transfer->completion_cb) {
				uint32_t bytes_transferred = transfer->maximum_length - transfer->td.total_bytes;
				transfer->completion_cb(transfer->user_data, bytes_transferred, transfer->td.halted, transfer->td.transaction_error);
			}

			// Remove it from the pending list...
			previous->link = transfer->horizontal.link;

			// ... move to the next element in the list...
			previous = link;
			link = next_link(link);

			// ... and free the transfer.
			usb_host_free_transfer(transfer);
		} 
		
		// Otherwise, continue iterating.
		else {
			previous = link;
			link = next_link(link);
		}

	}
}
