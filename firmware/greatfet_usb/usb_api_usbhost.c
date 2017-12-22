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

static void usbhost_usb_isr(void);

typedef char packet_buffer[512];

// To save storage, use the same buffers as GreatDancer, for now.
// For now, we shouldn't have both host and GreatDancer running at the
// same time. This _could_ change in the future.
//
extern packet_buffer endpoint_buffer[NUM_USB1_ENDPOINTS];
extern uint32_t total_received_data[NUM_USB1_ENDPOINTS];


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

		// Set up our IRQ handler and enable the USB controller.
		// From this point forward, the greatdancer_usb_isr can be generated.
		usb_set_irq_handler(&usb_peripherals[1], usbhost_usb_isr);
		usb_run(&usb_peripherals[1]);

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
static uint32_t get_status_register(uint16_t index,
		const usb_peripheral_t* const host)
{
	return USB_REG(host->controller)->PORTSC1;
}


/**
 * Queries the GreatDancer for any events that need to be processed.
 *
 * The index value is used to select which status section we're looking for:
 *
 *  0 = port status (PORTSC1 register)
 *
 *  Always transmits a 4-byte word back to the host.
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
 * FIXME: drop the 's' in the name
 */
usb_request_status_t usb_vendor_request_usbhost_set_up_endpoints(
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
 * Sends a SETUP packet to the given host.
 *
 * TODO: accept arguments to specify the endpoint it should be sent on
 *
 * Expects a populated usb_setup_t with the data to be transmitted.
 */
usb_request_status_t usb_vendor_request_usbhost_send_setup_packet(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	static usb_setup_t command;

	if (stage == USB_TRANSFER_STAGE_SETUP) {

		if(endpoint->setup.length != sizeof(usb_setup_t)) {
			return USB_REQUEST_STATUS_STALL;
		}

		// Read the command from the host.
		usb_transfer_schedule_block(endpoint->out, &command, endpoint->setup.length, NULL, NULL);

	} else if(stage == USB_TRANSFER_STAGE_DATA) {

    // Send the setup packet...
    usb_host_transfer_schedule(
        usb_peripherals[1].control_endpoint_queue,
        USB_PID_TOKEN_SETUP,
        &command,
        sizeof(usb_setup_t),
        NULL,
        NULL
      );

    // ... and ack the send to the host.
		usb_transfer_schedule_ack(endpoint->in);
	}

	return USB_REQUEST_STATUS_OK;
}



/**
 * Handle interrupts for the Greatdancer's USB controller.
 */
static void usbhost_usb_isr(void) {
	const uint32_t status = usb_get_status(&usb_peripherals[1]);
}
