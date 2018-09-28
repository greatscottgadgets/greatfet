/*
 * This file is part of GreatFET
 */

#include <stddef.h>

#include <greatfet_core.h>
#include <usb_type.h>

#include <usb.h>
#include <usb_host.h>
#include <usb_queue_host.h>
#include <usb_registers.h>

#include <usb_host_stack.h>


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
	uint16_t value, uint16_t index, uint16_t length, void *buffer)
{
	int rc, read_length = 0;

	// By default, if we don't have a data stage, use an IN token for the ack.
	usb_token_t ack_token;
	usb_token_t data_token;

	// Compose the setup packet to the transmitted.
	usb_setup_t setup_packet = {
		.request_type = request_type,
		.request = request,
		.value = value,
		.index = index,
		.length = length
	};

	// If this is a DEVICE_TO_HOST, our data stage should be an IN.
	if (request_type & USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_DEVICE_TO_HOST) {
		data_token = USB_PID_TOKEN_IN;
		ack_token  = USB_PID_TOKEN_OUT;
	} else {
		data_token = USB_PID_TOKEN_OUT;
		ack_token  = USB_PID_TOKEN_IN;
	}

	// Send the setup packet...
	rc = usb_host_transfer(
			host,
			qh,
			USB_PID_TOKEN_SETUP,
			0,
			&setup_packet,
			sizeof(setup_packet)
		);
	if (rc < 0)
		return rc;

	// If this packet includes a request for data,
	// ask the device for the relevant data.
	if (length) {

		rc = usb_host_transfer(
				host,
				qh,
				data_token,
				1,
				buffer,
				length
			);
		if (rc < 0)
			return rc;

		// Store the length actually read.
		read_length = rc;
	}
	// If we had no data stage, the ACK is always following an OUT-like
	// SETUP request, and thus should be an IN request.
	else {
		ack_token = USB_PID_TOKEN_IN;
	}

	// Perform the acknowledgement stage
	rc = usb_host_transfer(
			host,
			qh,
			ack_token,
			1,
			NULL,
			0
		);
	if (rc < 0)
		return rc;

	return read_length;
}


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
	void *data, size_t length)
{
	return usb_host_transfer(
		host,
		endpoint,
		USB_PID_TOKEN_OUT,
		0,
		data,
		length
	);
}


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
	void *data, size_t length)
{
	return usb_host_transfer(
		host,
		endpoint,
		USB_PID_TOKEN_IN,
		0,
		data,
		length
	);
}


/**
 * Read the target device's device descriptor.
 *
 * @param host The USB peripheral to work with.
 * @param qh The endpoint object for the device's control endpoint.
 * @param descriptor_out Buffer to recieve the device descriptor.
 */
int usb_host_get_descriptor(usb_peripheral_t *host,
	ehci_queue_head_t *qh, uint8_t descriptor_type, uint8_t descriptor_index,
	uint16_t language_id, uint16_t max_length, void *descriptor_out)
{
	return usb_host_control_request(host, qh,
		USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_DEVICE_TO_HOST |
		USB_SETUP_REQUEST_TYPE_STANDARD |
		USB_SETUP_REQUEST_TYPE_RECIPIENT_DEVICE,
		USB_STANDARD_REQUEST_GET_DESCRIPTOR,
		descriptor_type << 8 | descriptor_index,
		language_id,
		max_length,
		descriptor_out
	);
}


/**
 * Read the target device's device descriptor.
 *
 * @param host The USB peripheral to work with.
 * @param qh The endpoint object for the device's control endpoint.
 * @param descriptor_out Buffer to recieve the device descriptor.
 */
int usb_host_read_device_descriptor(usb_peripheral_t *host,
	ehci_queue_head_t *qh, usb_descriptor_device_t *descriptor_out)
{
	return usb_host_get_descriptor(host, qh,
			USB_DESCRIPTOR_TYPE_DEVICE, 0, 0, sizeof(*descriptor_out), descriptor_out);
}


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
    uint16_t max_length)
{
	return usb_host_get_descriptor(host, qh, USB_DESCRIPTOR_TYPE_CONFIGURATION,
            0, 0, max_length, descriptor_out);
}


/**
 * Read the target device's device descriptor.
 *
 * @param host The USB peripheral to work with.
 * @param qh The endpoint object for the device's control endpoint.
 * @param descriptor_out Buffer to recieve the device descriptor.
 */
int usb_host_switch_configuration(usb_peripheral_t *host,
	ehci_queue_head_t *qh, uint8_t configuration_number)
{
	return usb_host_control_request(host, qh,
		USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_HOST_TO_DEVICE |
		USB_SETUP_REQUEST_TYPE_STANDARD |
		USB_SETUP_REQUEST_TYPE_RECIPIENT_DEVICE,
		USB_STANDARD_REQUEST_SET_CONFIGURATION,
		configuration_number,
		0,
		0,
		NULL
	);
}


/**
 * Set the device's working address.
 *
 * @param host The USB peripheral to work with.
 * @param qh The endpoint object for the device's control endpoint.
 * @param descriptor_out Buffer to recieve the device descriptor.
 */
int usb_host_set_address(usb_peripheral_t *host,
	ehci_queue_head_t *qh, uint16_t address)
{
	return usb_host_control_request(host, qh,
		USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_HOST_TO_DEVICE |
		USB_SETUP_REQUEST_TYPE_STANDARD |
		USB_SETUP_REQUEST_TYPE_RECIPIENT_DEVICE,
		USB_STANDARD_REQUEST_SET_ADDRESS,
		address,
		0,
		0,
		NULL
	);
}
