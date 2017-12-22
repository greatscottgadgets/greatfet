/*
 * This file is part of GreatFET
 */

#ifndef __USB_QUEUE_HOST_H__
#define __USB_QUEUE_HOST_H__

#include <libopencm3/lpc43xx/usb.h>

#include "usb_type.h"
#include "usb_queue.h"

// TODO: tune these?
#define USB_HOST_MAX_QUEUE_HEADS 8
#define USB_HOST_MAX_TRANSFER_DESCRIPTORS 8

/**
 * Structure that transperently wraps ECHI transfers descriptors and which
 * contains our metadat.
 */
typedef struct {

	// Pointer to the next element, typically in the freelist.
	ehci_link_t horizontal;

	// Core transfer descriptor. Should be automatically aligned.
	ehci_transfer_descriptor_t td;

	// Queue head on which this transfer exists.
	struct ehci_queue_head_t *qh;

	// Optional callback for completion of the given transfer.
	// FIXME: Not yet supported?
	transfer_completion_cb completion_cb;

	// User data to be passed to the given callback.
	void* user_data;

} ehci_transfer_t;


/**
 * Initializes the storage pools for USB objects. This essentially reclaims all
 * of the existing structures, so it should only be called when all of them are
 * free, or the controller hasn't been initialized.
 *
 * Must be called before using the USB host APIs.
 *
 * It's recommended to use this only on device initialization.
 */
void usb_host_initialize_storage_pools(void);


/**
 * Allocates a queue head from the pool of available queue heads, if possible.
 *
 * @return A queue head which should be freed back to the pool when no longer
 *    used with usb_host_free_queue_head.
 */
ehci_queue_head_t * usb_host_allocate_queue_head(void);


/**
 * Frees a queue head, returning it to the pool of available queue heads.
 *
 * @param to_free The queue head to be freed.
 */
void usb_host_free_queue_head(ehci_queue_head_t *to_free);


/**
 * Allocates a transfer descriptor from the pool of available transfer descriptors, if possible.
 *
 * @return A transfer descriptor which should be freed back to the pool when no longer
 *    used with usb_host_free_queue_head.
 */
ehci_transfer_t * usb_host_allocate_transfer(void);


/**
 * Frees a transfer descriptor, returning it to the pool of available transfer descriptors.
 *
 * @param to_free The transfer descriptor to be freed.
 */
void usb_host_free_transfer(ehci_transfer_t *to_free);

/**
 * Sets up an endpoint for use in issuing USB transactions. This can be used
 * for any endpoint on the asynchronous queue (e.g. not interrupt or iso).
 *
 * Intended to be used internally to the endpoint API, but accessible for
 * low-level access if e.g. Host APIs require.
 *
 * @param device_address The address of the downstream device.
 * @param endpoint_number The endpoint number of the endpoint being configurd,
 *		_not_ including the direction bit.
 * @param endpoint_speed The speed of the endpoint. Should match the speed of
 *		the attached device.
 * @param is_control_endpoint True iff the endpoint is a control endpoint.
 * @param max_packet_size The maximum packet size transmissable on the endpoint;
 *		up to 1024.
 */
ehci_queue_head_t * set_up_asynchronous_endpoint_queue(usb_peripheral_t *host, uint8_t device_address,
		uint8_t endpoint_number, usb_speed_t endpoint_speed,
		bool is_control_endpoint, uint16_t max_packet_size);


/**
 * Schedule a USB transfer on the hosts's asynchronous queue.
 * This will execute as soon as the hardware can.
 *
 * This is the low-level version; you may want to call this using an endpoint
 * abstraction.
 *
 */
int usb_host_transfer_schedule(
	const ehci_queue_head_t *qh,
	const usb_token_t pid_code,

	void* const data,
	const uint32_t maximum_length,
	const transfer_completion_cb completion_cb,
	void* const user_data
);


#endif//__USB_QUEUE_HOST_H__
