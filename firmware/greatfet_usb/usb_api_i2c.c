/*
 * This file is part of GreatFET
 */

#include "usb_api_spi.h"
#include "usb_queue.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <i2c_bus.h>

uint8_t i2c_tx_buffer[255];
uint8_t i2c_rx_buffer[255];
uint16_t duty_cycle_count;

usb_request_status_t usb_vendor_request_i2c_start(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		if (endpoint->setup.value == 0) {
			duty_cycle_count = 255;
		} else {
			duty_cycle_count = endpoint->setup.value;
		}
		i2c_bus_start(&i2c0, duty_cycle_count);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_i2c_stop(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		i2c_bus_stop(&i2c0);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

/* wValue = slave address, wIndex = response length */
usb_request_status_t usb_vendor_request_i2c_xfer(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	if (stage == USB_TRANSFER_STAGE_SETUP)  {
		if (endpoint->setup.length > 0) {
			// Send data to the I2C device, so we need data from the host.
			usb_transfer_schedule_block(endpoint->out, i2c_tx_buffer,
										endpoint->setup.length, NULL, NULL);
		} else {
			// We are only reading from the I2C device so do everything here
			i2c_bus_transfer(&i2c0, endpoint->setup.value & 0xff, NULL, 0,
							 i2c_rx_buffer, endpoint->setup.index);
			usb_transfer_schedule_ack(endpoint->in);
		}
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		i2c_bus_transfer(&i2c0, endpoint->setup.value & 0xff, i2c_tx_buffer,
						 endpoint->setup.length, i2c_rx_buffer,
						 endpoint->setup.index);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_i2c_response(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->in, i2c_rx_buffer,
									endpoint->setup.length, NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}
