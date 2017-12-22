/*
 * This file is part of GreatFET
 */

#include "usb_api_spi.h"

#include <stddef.h>
#include "usb_api_usbhost.h"

#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/m4/nvic.h>

#include <greatfet_core.h>

#include "usb.h"
#include "usb_standard_request.h"
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_endpoint.h"
#include "usb_request.h"
#include "usb_host.h"
#include "usb_queue_host.h"
#include "usb_registers.h"

typedef char packet_buffer[512];

// To save storage, use the same buffers as GreatDancer, for now.
// For now, we shouldn't have both host and GreatDancer running at the
// same time. This _could_ change in the future.
//
extern packet_buffer endpoint_buffer[NUM_USB1_ENDPOINTS];
extern uint32_t total_received_data[NUM_USB1_ENDPOINTS];

// State for the USB read-and-write APIS.
// -Upper 16 bits: set if the given endpoint has completed the last
//  scheduled read/write.
// -Lower 16 bits: set if the last read/write on the given endpoint stalled.
static volatile uint32_t usb_host_read_status  = 0;
static volatile uint32_t usb_host_write_status = 0;


/**
 * Enumeration describing each of the possible Index values for GET_STATUS
 * requests.
 */ 
enum greatdancer_status_request {
	GET_PORTSC1 = 0,
	GET_READ_STATUS = 1,
	GET_WRITE_STATUS = 2
};
typedef enum greatdancer_status_request greatdancer_status_request_t;


/**
 * Command used for issuing an endpoint setup.
 */
struct _endpoint_setup_command_t {
	uint8_t endpoint_schedule;
	uint8_t device_address;
	uint8_t endpoint_number;
	uint8_t endpoint_speed;
	uint8_t is_control_endpoint;
	uint16_t max_packet_size;
} __attribute__((packed));
typedef struct _endpoint_setup_command_t endpoint_setup_command_t;



/**
 * Start-of-day setup for the USBHost API.
 */
void init_usbhost_api(void)
{
	usb_host_initialize_storage_pools();
}


/**
 * Sets up the GreatFET to act as a USB host, including enabling the GreatFET
 * to provide USB1 VBUS.
 */
usb_request_status_t usb_vendor_request_usbhost_connect(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {

		// Set up the device in host mode...
		usb_controller_reset(&usb_peripherals[1]);
		usb_host_init(&usb_peripherals[1]);

		// Provide VBUS to the target, if possible.
		// TODO: maybe this should be its own vendor request
		usb_provide_vbus(&usb_peripherals[1]);

		// Enable the USB controller-- this will allow start the point
		// where interrupts can be issued.
		usb_run(&usb_peripherals[1]);

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Handle requests for the status of recent reads.
 * Clears any pending Stall or Complet notifications.
 */
static uint32_t usbhost_get_read_status()
{
	// Read the currently pending events, and clear them.
	return __sync_fetch_and_and(&usb_host_read_status, 0);
}

/**
 * Handle requests for the status of recent writes.
 * Clears any pending Stall or Complet notifications.
 */
static uint32_t usbhost_get_write_status()
{
	// Read the currently pending events, and clear them.
	return __sync_fetch_and_and(&usb_host_write_status, 0);
}


/**
 * Retrieves the value of a the stauts register corresponding to a given
 * greatfet_status_request_t.
 *
 * @param index Selects the status register to be queried.
 * @param device The device for which the register is to be queried.
 * @return The value of the status register.
 */
static uint32_t get_status_register(uint16_t index,
		const usb_peripheral_t* const host)
{
	switch(index) {
		case GET_PORTSC1:      return USB_REG(host->controller)->PORTSC1;
		case GET_READ_STATUS:  return usbhost_get_read_status();
		case GET_WRITE_STATUS: return usbhost_get_write_status();
	}

	// TODO: more meaningfull error handling, here?
	return -1;
}


/**
 * Queries the GreatDancer for any events that need to be processed.
 *
 * The index value is used to select which status section we're looking for:
 *
 *	0 = port status (PORTSC1 register)
 *
 *	Always transmits a 4-byte word back to the host.
 */
usb_request_status_t usb_vendor_request_usbhost_get_status(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		uint32_t status = get_status_register(endpoint->setup.index, &usb_peripherals[1]);
		usb_transfer_schedule_block(endpoint->in, (void * const)&status, sizeof(status), NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Issue a bus reset request to the downstream device.
 */
usb_request_status_t usb_vendor_request_usbhost_bus_reset(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_host_reset_device(&usb_peripherals[1]);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Sets up an endpoint on the USB host for use in comms.
 *
 * Expects a data stage with an _endpoint_setup_command_t packet
 * describing the endpoint to be initialized. See its documentation above.
 *
 * Setup fields not used.
 */
usb_request_status_t usb_vendor_request_usbhost_set_up_endpoint(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	static endpoint_setup_command_t command;

	if (stage == USB_TRANSFER_STAGE_SETUP) {

		if(endpoint->setup.length != sizeof(endpoint_setup_command_t)) {
			return USB_REQUEST_STATUS_STALL;
		}

		// Read the command from the host.
		usb_transfer_schedule_block(endpoint->out, &command, endpoint->setup.length, NULL, NULL);

	} else if(stage == USB_TRANSFER_STAGE_DATA) {

		// ... and process it.

		// Acknowledge early, as this can take a bit. :)
		usb_transfer_schedule_ack(endpoint->in);

		// TODO: Support schedules other than asynchronous.
		// TODO: Use an endpoint API, rather than the queue set up directly.

		ehci_queue_head_t *qh = set_up_asynchronous_endpoint_queue(&usb_peripherals[1], command.device_address,
				command.endpoint_number, command.endpoint_speed, command.is_control_endpoint,
				command.max_packet_size);

		// FIXME: this is temporary placeholder code
		usb_peripherals[1].control_endpoint_queue = qh;

	}
	return USB_REQUEST_STATUS_OK;
}


/**
 * Callback that's executed each time a nonblocking read completes.
 * Stores the number of bytes transferred when the read executed.
 */
static void handle_write_complete_callback(void * const user_data,
		unsigned int transferred, bool stalled, bool error)
{
		// This is horrible, but we've shoehorned a small integer via
		// the user-data field. Convert it back to an integer.
		int endpoint_number = (int)user_data;

		// TODO: possibly handle short packet transmissions,
		// if there's any reason on our side they could be enshortened
		(void)transferred;

		// Create a new write status word that contains:
		//	- A one in the appropriate bit to indicate a read has completed.
		//	- If appropriate, a 1 to incidate the read stalled.
		uint32_t new_write_status = (1 << (endpoint_number + 16));
		if(stalled || error) {
			new_write_status |= (1 << endpoint_number);
		}

		// Or the new read status into the existing write status register.
		__sync_fetch_and_or(&usb_host_write_status, new_write_status);
}


/**
 * Sends a packet from the host on a given endpoint. Can be used to send
 * an OUT or SETUP packet.
 *
 * Setup parameters:
 *		index: the endpoint to send on
 *		value: the type of PID token to accompany the transaction;
 *				we don't validate this so FaceDancer can potentially be horrible
 * Data stage:
 *		the data to be transmitted
 *
 * Expects a populated usb_setup_t with the data to be transmitted.
 */
usb_request_status_t usb_vendor_request_usbhost_send_on_endpoint(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	int endpoint_number = endpoint->setup.index;
	int pid_token       = endpoint->setup.value;

	// FIXME: Grab the endpoint queue for the given endpoint, rather than always
	// assuming EP0.
	ehci_queue_head_t *endpoint_queue = usb_peripherals[1].control_endpoint_queue;

	if (stage == USB_TRANSFER_STAGE_SETUP) {

		// If this isn't a valid endpoint, stall the request.
		if(endpoint_number >= NUM_USB1_ENDPOINTS) {
				return USB_REQUEST_STATUS_STALL;
		}

		// If we're requesting a ZLP issue it immediately.
		if(endpoint->setup.length == 0) {

			// Send the setup packet...
			usb_host_transfer_schedule(
					&usb_peripherals[1],
					endpoint_queue,
					pid_token,
					NULL,
					0,
					handle_write_complete_callback,
					(void *)endpoint_number
				);

			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		}

		// Read the command from the host into the relevant endpoint buffer.
		usb_transfer_schedule_block(endpoint->out, &endpoint_buffer[endpoint_number], endpoint->setup.length, NULL, NULL);

	}
	else if(stage == USB_TRANSFER_STAGE_DATA) {

		// Send the setup packet...
		usb_host_transfer_schedule(
				&usb_peripherals[1],
				endpoint_queue,
				pid_token,
				&endpoint_buffer[endpoint_number],
				endpoint->setup.length,
				handle_write_complete_callback,
				(void *)endpoint_number
			);

		// ... and ack the send to the host.
		usb_transfer_schedule_ack(endpoint->in);
	}

	return USB_REQUEST_STATUS_OK;
}


/**
 * Callback that's executed each time a nonblocking read completes.
 * Stores the number of bytes transferred when the read executed.
 */
static void handle_read_complete_callback(void * const user_data,
		unsigned int transferred, bool stalled, bool error)
{
		// This is horrible, but we've shoehorned a small integer via
		// the user-data field. Convert it back to an integer.
		int endpoint_number = (int)user_data;

		// Store the total amount waiting to be read.
		total_received_data[endpoint_number] = transferred;

		// Create a new read status word that contains:
		//	- A one in the appropriate bit to indicate a read has completed.
		//	- If appropriate, a 1 to incidate the read stalled.
		uint32_t new_read_status = (1 << (endpoint_number + 16));
		if(stalled || error) {
			new_read_status |= (1 << endpoint_number);
		}

		// Or the new read status into the existing read status register.
		__sync_fetch_and_or(&usb_host_read_status, new_read_status);
}




/**
 * Primes the USB controller to recieve data on a particular endpoint, but
 * does not wait for a transfer to complete. The transfer's status can be
 * checked with get_transfer_status and then read with finish_nonblocking_read.
 *
 * The index parameter specifies which endpoint we should be reading from.
 * The value parameter specifies the maximum length that shoud be read during
 * this transaction.
 */
usb_request_status_t usb_vendor_request_usbhost_start_nonblocking_read(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		int endpoint_number = endpoint->setup.index;
		int total_data = endpoint->setup.value;

		// FIXME: Grab the endpoint queue for the given endpoint, rather than always
		// assuming EP0.
		ehci_queue_head_t *endpoint_queue = usb_peripherals[1].control_endpoint_queue;

		// ... and start a nonblocking transfer.
		usb_host_transfer_schedule(
				&usb_peripherals[1],
				endpoint_queue,
				USB_PID_TOKEN_IN,
				&endpoint_buffer[endpoint_number],
				sizeof(endpoint_buffer[endpoint_number]),
				handle_read_complete_callback,
				(void *)endpoint_number);

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

/**
 * Queries an endpoint to determine how much data is available. Should only be
 * used after a nonblocking read was primed with START_NONBLOCKING_READ and
 * completed by the USB hardware. Completion can be checked by [how?].
 *
 * Response is valid unless a transfer has been initiated with
 * START_NONBLOCKING_READ and then has completed.
 *
 * index: The endpoint to check.
 */
usb_request_status_t usb_vendor_request_usbhost_get_nonblocking_data_length(
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
usb_request_status_t usb_vendor_request_usbhost_finish_nonblocking_read(
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
