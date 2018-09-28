/*
 * This file is part of GreatFET
 * High-level APIs for accessing USB devices via the USB host driver.
 */

#ifndef __USB_HOST_STACK_H__
#define __USB_HOST_STACK_H__

#include <usb_type.h>

/**
 * Issues a control request to the device.
 *
 * @param host The USB host to use for transfers.
 * @param qh The endpoint object for the control endpoint.
 * @param request_type The Request Type object, composed of the request_type
 * 	flags from usb_type.h.
 * @param request The control request number.
 * @param value, index Argumenst to the control request.
 * @param length The length transmitted (for an HOST_TO_DEVICE request)
 * 	or maximum data we're willing to recieve (for a DEVICE_TO_HOST).
 * @param buffer The data to be transmitted (HOST_TO_DEVICE) or buffer to
 * 	recieve response (DEVICE_TO_HOST).
 *
 * @return length transferred on success, or a negative error code on failure
 * 	-EIO indicate a I/O error; -EPIPE indicates a stall
 */
int usb_host_control_request(usb_peripheral_t *host,
	ehci_queue_head_t *qh, usb_setup_request_type_t request_type, uint8_t request,
	uint16_t value, uint16_t index, uint16_t length, void *buffer);


/**
 * Convenience function that sends data on a given endpoint.
 *
 * @param host The USB host to use for transmission.
 * @param endpoint The endpoint to send on.
 * @param data The data to be sent.
 * @param length The length of the data to send.
 *
 * @return length transferred on success, or a negative error code on failure
 * 	-EIO indicate a I/O error; -EPIPE indicates a stall
 */
int usb_host_send_on_endpoint(usb_peripheral_t *host, ehci_queue_head_t *endpoint,
	void *data, size_t length);


/**
 * Convenience function that sends data on a given endpoint.
 *
 * @param host The USB host to use for transmission.
 * @param endpoint The endpoint to send on.
 * @param data Buffer to recieve data.
 * @param length The maximum length we're willing to recieve.
 *
 * @return length transferred on success, or a negative error code on failure
 * 	-EIO indicate a I/O error; -EPIPE indicates a stall
 */
int usb_host_read_from_endpoint(usb_peripheral_t *host, ehci_queue_head_t *endpoint,
	void *data, size_t length);


/**
 * Read the target device's device descriptor.
 *
 * @param host The USB peripheral to work with.
 * @param qh The endpoint object for the device's control endpoint.
 * @param descriptor_out Buffer to recieve the device descriptor.
 */
int usb_host_get_descriptor(usb_peripheral_t *host,
	ehci_queue_head_t *qh, uint8_t descriptor_type, uint8_t descriptor_index,
	uint16_t language_id, uint16_t max_length, void *descriptor_out);


/**
 * Read the target device's configuration descriptor and subordinate
 *  descriptors.
 *
 * @param host The USB peripheral to work with.
 * @param qh The endpoint object for the device's control endpoint.
 * @param max_length The maximum length of descriptor to be read.
 * @param descriptor_out Buffer to recieve the device descriptor.
 */
int usb_host_read_configuration_desriptors(usb_peripheral_t *host,
	ehci_queue_head_t *qh, void *descriptor_out,
    uint16_t max_length);

/**
 * Read the target device's device descriptor.
 *
 * @param host The USB peripheral to work with.
 * @param qh The endpoint object for the device's control endpoint.
 * @param descriptor_out Buffer to recieve the device descriptor.
 */
int usb_host_read_device_descriptor(usb_peripheral_t *host,
	ehci_queue_head_t *qh, usb_descriptor_device_t *descriptor_out);


/**
 * Read the target device's device descriptor.
 *
 * @param host The USB peripheral to work with.
 * @param qh The endpoint object for the device's control endpoint.
 * @param descriptor_out Buffer to recieve the device descriptor.
 */
int usb_host_switch_configuration(usb_peripheral_t *host,
	ehci_queue_head_t *qh, uint8_t configuration_number);


int usb_host_set_address(usb_peripheral_t *host,
	ehci_queue_head_t *qh, uint16_t address);

#endif
