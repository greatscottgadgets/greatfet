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

/* Callback type for completed host transfers. */
typedef void (*host_transfer_completion_cb)(void*, unsigned int, bool, bool);

/**
 * Structure that transperently wraps ECHI transfers descriptors and which
 * contains our metadat.
 */
typedef struct {

	// Pointer to the next element, either in a free or a pending list.
	ehci_link_t horizontal;

	// Core transfer descriptor. Should be automatically aligned.
	ehci_transfer_descriptor_t td;

	// Optional callback for completion of the given transfer.
	host_transfer_completion_cb completion_cb;

	// User data to be passed to the given callback.
	void* user_data;

	// Store the total data we requested to be transferred.
	uint32_t maximum_length;

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
 * @param host The host this endpoint queue is associated with.
 * @param device_address The address of the downstream device.
 * @param endpoint_number The endpoint number of the endpoint being configurd,
 *		_not_ including the direction bit.
 * @param endpoint_speed The speed of the endpoint. Should match the speed of
 *		the attached device.
 * @param is_control_endpoint True iff the endpoint is a control endpoint.
 * @param handle_data_toggle If set, the endpoint should handle data toggling 
 *		automatically; otherwise, it will use the values specified when calling
 *		usb_host_transfer_schedule.
 * @param max_packet_size The maximum packet size transmissable on the endpoint;
 *		up to 1024.
 */
ehci_queue_head_t * usb_host_set_up_asynchronous_endpoint_queue(
		usb_peripheral_t *host, volatile ehci_queue_head_t *qh, uint8_t device_address,
		uint8_t endpoint_number, usb_speed_t endpoint_speed,
		bool is_control_endpoint, bool handle_data_toggle, uint16_t max_packet_size);


/**
 * Schedule a USB transfer on the hosts's asynchronous queue.
 * This will execute as soon as the hardware can.
 *
 * FIXME: Possibly use an endpoint abstaction rather than passing around QHs?
 * @param host The host that will execute the given transfer.
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
	usb_peripheral_t *host,
	ehci_queue_head_t *qh,
	const usb_token_t pid_code,
  const int data_toggle,

	void* const data,
	const uint32_t maximum_length,
	const host_transfer_completion_cb completion_cb,
	void* const user_data
);


/**
 * Cleans up any comlpeted transfers on a given QH. Assumes the QH
 * cannot be modified while executing-- the caller is responsible for
 * ensuring this by e.g. ensuring that USB interrupts are disabled.
 *
 * If any completed transfers have associated completion callbacks, this
 * method will execute them.
 *
 * This is automatically called if you're using the default interrupt handler;
 * or you can call this manually if you e.g. don't want an interrupt-driven
 * transciever or have your own interrupt handler.
 *
 * @param qh The queue head for the queue to be cleaned up.
 */
void usb_host_clean_up_completed_transfers(ehci_queue_head_t *qh);


/**
 * Handle completion of an asynchronous transfer, cleaning up any relevant
 * transactions and calling any transaction callbacks.
 *
 * This is automatically called if you're using the default interrupt handler;
 * or you can call this manually if you e.g. don't want an interrupt-driven
 * transciever or have your own interrupt handler.
 */
void usb_host_handle_asynchronous_transfer_complete(usb_peripheral_t *host);


#endif//__USB_QUEUE_HOST_H__
