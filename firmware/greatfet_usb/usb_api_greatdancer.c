/*
 * Copyright 2017 Kyle J. Temkin <kyle@ktemkin.com>
 *
 * This file is part of GreatFET.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "usb_api_spi.h"
#include "usb_queue.h"

#include <stddef.h>
#include "usb_api_greatdancer.h"

#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/m4/nvic.h>

#include <greatfet_core.h>

#include "usb.h"
#include "usb_standard_request.h"
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_endpoint.h"
#include "usb_request.h"

struct _endpoint_setup_command_t {
	uint8_t address;
	uint16_t max_packet_size;
	uint8_t transfer_type;
} __attribute__((packed));
typedef struct _endpoint_setup_command_t endpoint_setup_command_t;

typedef char packet_buffer[512];

packet_buffer endpoint_buffer[NUM_USB1_ENDPOINTS];
uint32_t total_received_data[NUM_USB1_ENDPOINTS];

/**
 * When using the GreatDancer, all events are generated
 * and handled on the host side, so we don't need to generate
 * anything.
 */
const usb_request_handlers_t usb1_request_handlers = {
	.standard = NULL,
	.class = NULL,
	.vendor = NULL,
	.reserved = NULL,
};


/**
 * Perform all of the one-time initializations of the GreatDancer API.
 */
void init_greatdancer_api(void) {
	// Initialize all of our queues, so they're ready
	// if the GreatDancer application decides to use them.
	usb_queue_init(&usb1_endpoint_control_out_queue);
	usb_queue_init(&usb1_endpoint_control_in_queue);
	usb_queue_init(&usb1_endpoint1_out_queue);
	usb_queue_init(&usb1_endpoint1_in_queue);
	usb_queue_init(&usb1_endpoint2_out_queue);
	usb_queue_init(&usb1_endpoint2_in_queue);
	usb_queue_init(&usb1_endpoint3_out_queue);
	usb_queue_init(&usb1_endpoint3_in_queue);
}


/**
 * Performs the per-run initialization of the GreatDancer device.
 * Should be between successive executions of the facedancer.
 */
static void set_up_greatdancer(void) {
	usb_peripheral_reset(&usb1_device);
	usb_device_init(&usb1_device);

	// Set up the control endpoint. The application will request setup
	// for all of the non-standard channels on connection.
	usb_endpoint_init(&usb1_endpoint_control_out);
	usb_endpoint_init(&usb1_endpoint_control_in);
}


/**
 * Finds the endpoint object associated with a given address.
 * This version can be used even before the USB controller is initialized,
 * but is less flexible.
 *
 * @param address The address for which an endpoint should be located.:w
 */
static usb_endpoint_t *usb_preinit_endpoint_from_address(uint8_t address)
{
	switch(address) {
		case 0x80: return &usb1_endpoint_control_in;
		case 0x00: return &usb1_endpoint_control_out;

		case 0x81: return &usb1_endpoint1_in;
		case 0x01: return &usb1_endpoint1_out;

		case 0x82: return &usb1_endpoint2_in;
		case 0x02: return &usb1_endpoint2_out;

		case 0x83: return &usb1_endpoint3_in;
		case 0x03: return &usb1_endpoint3_out;
	}

	return NULL;
}


/**
 * Sets up the GreatDancer to make a USB connection, resetting the device
 * if necessary. Enables USB pull-ups to begin the enumeration process.
 *
 * Setup arguments not used.
 */
usb_request_status_t usb_vendor_request_greatdancer_connect(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {

		usb_controller_reset(&usb1_device);
		set_up_greatdancer();

		// Note that we call usb_controller_run and /not/ usb_run.
		// This in particular leaves all interrupts masked in the NVIC
		// so we can poll them manually from the host side.
		usb_controller_run(&usb1_device);

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Sets up the GreatDancer to make a USB connection.
 *
 * Expects zero or more triplets describing how the device's endpoints should
 * be initialied. Each triplet should contain:
 *
 * - One byte of endpoint address
 * - Two bytes describing the maximum packet size on the endpoint
 * - One byte describing the endpoint type
 */
usb_request_status_t usb_vendor_request_greatdancer_set_up_endpoints(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {

		// Read the data to be transmitted from the host.
		// Note that we use endpoint buffer zero, as it's the only buffer that's been set up.
		usb_transfer_schedule_block(endpoint->out, &endpoint_buffer[0], endpoint->setup.length, NULL, NULL);

	} else if(stage == USB_TRANSFER_STAGE_DATA) {
		int i;
		endpoint_setup_command_t *command = NULL;

		// Iterate over each of the provided "endpoint setup" packets...
		for(i = 0; i < endpoint->setup.length; i += 4) {
			usb_endpoint_t* target_endpoint;
			command = (endpoint_setup_command_t *)&endpoint_buffer[0][i];

			// Endpoint zero is always the control endpoint, and can't be configured.
			// Ignore any attempt to configure the control endpoint.
			if((command->address & 0x7F) == 0x00) {
					continue;
			}

			// Ignore any endpoint configurations that try and set up transactions
			// we won't be able to handle.
			if(command->max_packet_size > sizeof(packet_buffer)) {
					continue;
			}

			// Fetch the endpoint to be configured, and configure it.
			target_endpoint = usb_preinit_endpoint_from_address(command->address);
			if(!target_endpoint) {
					continue;
			}

			// And initialize the endpoint.
			usb_endpoint_init_without_descriptor(target_endpoint, command->max_packet_size, command->transfer_type);
		}

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Terminates all existing communication and shuts down the GreatDancer USB.
 */
usb_request_status_t usb_vendor_request_greatdancer_disconnect(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_controller_reset(&usb1_device);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Retrieves the value of a the stauts register corresponding to a given
 * greatfet_status_request_t.
 *
 * @param index Selects the status register to be queried.
 * @param device The device for which the register is to be queried.
 * @return The value of the status register.
 */
static uint32_t get_status_register(greatdancer_status_request_t index,
		const usb_device_t* const device)
{
	switch(index) {
		case GET_USBSTS: return usb_get_status(device);
		case GET_ENDPTSETUPSTAT: return usb_get_endpoint_setup_status(device);
		case GET_ENDPTCOMPLETE: return usb_get_endpoint_complete(device);
		case GET_ENDPTSTATUS: return usb_get_endpoint_ready(device);
	}

	// TODO: more meaningfull error handling, here?
	return -1;
}


/**
 * Queries the GreatDancer for any events that need to be processed.
 *
 * The index value is used to select which status section we're looking for:
 *
 *  0 = pending interrupts (USBSTS register)
 *  1 = setup status for all endpoints (ENDPTSETUPSTAT)
 *  2 = endpoint completion status (ENDPTCOMPLETE)
 *  3 = endpoint primed status (ENDPTSTATUS)
 *
 *  Always transmits a 4-byte word back to the host.
 */
usb_request_status_t usb_vendor_request_greatdancer_get_status(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		uint32_t status = get_status_register(endpoint->setup.index, &usb1_device);
		usb_transfer_schedule_block(endpoint->in, (void * const)&status, sizeof(status), NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Reads a setup packet from the GreatDancer port and relays it to the host.
 * The index parameter specifies which endpoint we should be reading from.
 *
 * Always transmits an 8-byte setup packet back to the host. If no setup packet
 * is waiting, the results of this vendor request are unspecified.
 */
usb_request_status_t usb_vendor_request_greatdancer_read_setup(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		int endpoint_number = endpoint->setup.index;

		// Figure out the endpoint we're reading setup data from...
		uint_fast8_t address = usb_endpoint_address(USB_TRANSFER_DIRECTION_OUT, endpoint_number);
		usb_endpoint_t* const target_endpoint = usb_endpoint_from_address(address, &usb1_device);

		// ... and find its setup data.
		uint8_t * const setup_data =
			(uint8_t * const)usb_queue_head(target_endpoint->address, target_endpoint->device)->setup;

		// Transmit the setup data back ...
		usb_transfer_schedule_block(endpoint->in, setup_data, 8, NULL, NULL);

		// ... and mark that packet as handled.
		usb_clear_endpoint_setup_status(1 << endpoint_number, &usb1_device);

	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Callback that's executed each time a nonblocking read completes.
 * Stores the number of bytes transferred when the read executed.
 */
static void store_transfer_count_callback(void * const user_data, unsigned int transferred)
{
		unsigned int * total_data = (unsigned int*)user_data;

		if(total_data) {
			*total_data = transferred;
		}
}


/**
 * Primes the USB controller to recieve data on a particular endpoint, but
 * does not wait for a transfer to complete. The transfer's status can be
 * checked with get_transfer_status and then read with finish_nonblocking_read.
 *
 * The index parameter specifies which endpoint we should be reading from.
 */
usb_request_status_t usb_vendor_request_greatdancer_start_nonblocking_read(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		int endpoint_number = endpoint->setup.index;

		// Figure out the endpoint we're reading setup data from...
		uint_fast8_t address = usb_endpoint_address(USB_TRANSFER_DIRECTION_OUT, endpoint_number);
		usb_endpoint_t* const target_endpoint = usb_endpoint_from_address(address, &usb1_device);

		// ... and start a nonblocking transfer.
		usb_transfer_schedule(target_endpoint, &endpoint_buffer[endpoint_number], sizeof(packet_buffer), store_transfer_count_callback, &total_received_data[endpoint_number]);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Queries an endpoint to determine how much data is available. Should only be
 * used after a nonblocking read was primed with START_NONBLOCKING_READ and
 * completed by the USB hardware. Completion can be checked by reading the
 * ENDPTCOMPLETE register with GET_STATUS.
 *
 * Response is valid unless a transfer has been initiated with
 * START_NONBLOCKING_READ and then has completed.
 *
 * index: The endpoint to check.
 */
usb_request_status_t usb_vendor_request_greatdancer_get_nonblocking_data_length(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		// Get the total data read, and send it back to the host.
		int endpoint_number = endpoint->setup.index;
		usb_transfer_schedule_block(endpoint->in, (void * const)&total_received_data[endpoint_number], sizeof(total_received_data[endpoint_number]), NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Finishes a non-blocking read by returning the read data back to the host.
 * Should only be used after determining that a transfer is complete with
 * the get_transfer_status request and reading the relevant length with
 * get_nonblocking_data_length.
 *
 * index: The endpoint number to request data on.
 * length: The total amount of bytes to read, which must be <= the value returned
 *		by a get_nonblocking_data_lengthr request.
 */
usb_request_status_t usb_vendor_request_greatdancer_finish_nonblocking_read(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		int endpoint_number = endpoint->setup.index;

		// If we've requested more than will fit in our buffer, or
		// more than we've read, we can't handle this transaction. Stall the endpoint.
		// This should never happen in proper operation, as apps should always provide
		// a max_packet_size that's less than sizeof(packet_buffer).
		if(endpoint->setup.length > sizeof(packet_buffer) ||
				(endpoint->setup.length > total_received_data[endpoint_number])) {
			usb_endpoint_stall(endpoint);
		}

		// Transmit the data back.
		usb_transfer_schedule_block(endpoint->in, &endpoint_buffer[endpoint_number], endpoint->setup.length, NULL, NULL);

	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Reads data from the GreatFET host and sends on a provided GreatDancer endpoint.
 * The index parameter specifies which endpoint we should be reading from.
 *
 * index: The endpoint to be transmitted.
 * The OUT request should contain a data stage containing all data to be sent.
 */
usb_request_status_t usb_vendor_request_greatdancer_send_on_endpoint(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	int endpoint_number = endpoint->setup.index;

	// Figure out the endpoint we're reading setup data from...
	uint_fast8_t address = usb_endpoint_address(USB_TRANSFER_DIRECTION_IN, endpoint_number);
	usb_endpoint_t* const target_endpoint = usb_endpoint_from_address(address, &usb1_device);

	if (stage == USB_TRANSFER_STAGE_SETUP) {

		// If we've requested more than will fit in our buffer, or
		// more than we've read, we can't handle this transaction. Stall the endpoint.
		if(endpoint->setup.length > sizeof(packet_buffer)) {
			usb_endpoint_stall(endpoint);
		}

		// If we have a zero-length packet, handle it immediately.
		if(endpoint->setup.length == 0 ) {
			usb_transfer_schedule_ack(target_endpoint);
			usb_transfer_schedule_ack(endpoint->in);
		} else {
			// Read the data to be transmitted from the host.
			usb_transfer_schedule_block(endpoint->out, &endpoint_buffer[endpoint_number], endpoint->setup.length, NULL, NULL);
		}

	}
	else if (stage == USB_TRANSFER_STAGE_DATA) {
		if(endpoint->setup.length > 0 ) {
			// Send the data on the endpoint.
			usb_transfer_schedule(target_endpoint, &endpoint_buffer[endpoint_number], endpoint->setup.length, NULL, NULL);
			usb_transfer_schedule_ack(endpoint->in);
		}
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Sets the address of the GreatDancer.
 *
 * value: The USB address that the GreatDancer device should assume.
 */
usb_request_status_t usb_vendor_request_greatdancer_set_address(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		uint_fast8_t address = endpoint->setup.value_l;

		// Currently, this is executed the status stage of the GreatDancer
		// request is compleed, so we use _immediate. If this ever changes,
		// this will need to be re-evaluated to see if we should use deferred.:w
		//
		usb_set_address_immediate(&usb1_device, address);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Stimulate a bus reset on the GreatDancer side.
 */
usb_request_status_t usb_vendor_request_greatdancer_bus_reset(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_bus_reset(&usb1_device);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Temporarily stalls the relevant USB endpoint.
 *
 * index: The endpoint to be stalled.
 */
usb_request_status_t usb_vendor_request_greatdancer_stall_endpoint(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		int endpoint_number = endpoint->setup.index;

		// Figure out the endpoint we're reading setup data from...
		uint_fast8_t address = usb_endpoint_address(USB_TRANSFER_DIRECTION_OUT, endpoint_number);
		usb_endpoint_t* const target_endpoint = usb_endpoint_from_address(address, &usb1_device);

		usb_endpoint_stall(target_endpoint);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Should be called whenever a transfer is complete; cleans up any transfer
 * descriptors assocaited with that transfer.
 *
 * index: The endpoint on which the transfer should be cleaned up.
 * value: The direction; matches the USB spec. (1 for IN)
 */
usb_request_status_t usb_vendor_request_greatdancer_clean_up_transfer(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		int endpoint_number = endpoint->setup.index;
		int direction = (endpoint->setup.value_l) ? USB_TRANSFER_DIRECTION_IN : USB_TRANSFER_DIRECTION_OUT;

		// Figure out the endpoint we're reading setup data from...
		uint_fast8_t address = usb_endpoint_address(direction, endpoint_number);
		usb_endpoint_t* const target_endpoint = usb_endpoint_from_address(address, &usb1_device);

		// Clear the "transfer complete" bit.
		if(direction == USB_TRANSFER_DIRECTION_IN) {
			usb_clear_endpoint_complete(USB1_ENDPTCOMPLETE_ETCE(1 << endpoint_number), &usb1_device);
		} else {
			usb_clear_endpoint_complete(USB1_ENDPTCOMPLETE_ERCE(1 << endpoint_number), &usb1_device);
		}

		// Clean up any transfers that are complete on the given endpoint.
		usb_queue_transfer_complete(target_endpoint);

		// Send the acknolwedgement for the control channel...
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
