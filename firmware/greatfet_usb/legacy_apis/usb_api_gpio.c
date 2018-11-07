/*
 * This file is part of GreatFET
 */

#include "usb_api_gpio.h"
#include <drivers/usb/lpc43xx/usb_queue.h>

#include <stddef.h>
#include <greatfet_core.h>
#include <gpio.h>
#include <gpio_lpc.h>
#include <gpio_scu.h>
#include <libopencm3/lpc43xx/scu.h>

static struct gpio_t gpio_in[80];   /* Registry of GPIO pins set as inputs */
static struct gpio_t gpio_out[80];  /* Registry of GPIO pins set as outputs */
static uint8_t gpio_in_count = 0;   /* Count of elements used in gpio_in */
static uint8_t gpio_out_count = 0;  /* Count of elements used in gpio_out */
static uint8_t gpio_buffer[80 * 2]; /* Buffer used for USB transfers */


/* Configure the SCU pinmux to GPIO mode for a pin */
static void set_scu_pinmux_for_gpio(uint8_t gpio_port, uint8_t gpio_pin)
{
	scu_grp_pin_t scu_pin = get_scu_pin_for_gpio(gpio_port, gpio_pin);
	uint32_t scu_func = get_scu_func_for_gpio(gpio_port, gpio_pin);
	/* TODO: allow pull-up to be configured */
	scu_pinmux(scu_pin, SCU_GPIO_NOPULL | scu_func);
}


/* Set GPIO pin definitions and directions
   Setup packet:
	   wValue:  count of pins in the data packet that will be inputs
		 wLength: count of all pins in the data packet * 2
	 Data packet:
	 	 consists of a pair of uint8_t for each LPC GPIO to set up:
		   {port, pin}
     ordering in packet determines pin direction:
		   inputs first (determined by wValue), then outputs
*/
usb_request_status_t usb_vendor_request_gpio_register(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t total_pairs_count = endpoint->setup.length;
	uint8_t input_pairs_count = endpoint->setup.value * 2;
	uint8_t i;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->out, &gpio_buffer,
									endpoint->setup.length, NULL, NULL);

	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		for (i=0; i < total_pairs_count; i+=2) {
			uint8_t port = gpio_buffer[i];
			uint8_t pin = gpio_buffer[i+1];

			if (i < input_pairs_count) {
				/* Configure pin as input */
				GPIO_SET(gpio_in[gpio_in_count], port, pin);
				gpio_input(&gpio_in[gpio_in_count]);
				set_scu_pinmux_for_gpio(port, pin);
				gpio_in_count++;
			} else {
				/* Configure pin as output */
				GPIO_SET(gpio_out[gpio_out_count], port, pin);
				gpio_output(&gpio_out[gpio_out_count]);
				gpio_clear(&gpio_out[gpio_out_count]);
				set_scu_pinmux_for_gpio(port, pin);
				gpio_out_count++;
			}
		}

		usb_transfer_schedule_ack(endpoint->in);
	}

	return USB_REQUEST_STATUS_OK;
}


/* Write to GPIO output pins
   Setup packet:
		 wLength: count of all GPIO pins in the data packet * 2
	 Data packet:
     consists of a pair of uint8_t for each GPIO output pin to change:
		   {index into gpio_out array, 0=low or nonzero=high}
*/
usb_request_status_t usb_vendor_request_gpio_write(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t total_pairs_count = endpoint->setup.length;
	uint8_t i;
	uint8_t gpio_index;
	struct gpio_t gpio;
	bool state;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->out, &gpio_buffer,
									endpoint->setup.length, NULL, NULL);

	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		for (i=0; i<total_pairs_count; i+=2) {
			gpio_index = gpio_buffer[i];
			state = gpio_buffer[i+1];

			if (gpio_index < gpio_out_count) {
				gpio = gpio_out[gpio_index];
				gpio_write(&gpio, state);
			}
		}

		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


/* Read GPIO input pins
 	 Setup packet:
	 	 wLength: max count of bytes to return (host typically sends 0xFF)
	 Data packet:
	 	 array of uint8_t where GPIO input states are packed as follows:
			 byte 0, bit 7: gpio_in[0]
			 byte 0, bit 6: gpio_in[1]
			 ...
			 byte 0, bit 0: gpio_in[7]
			 byte 1, bit 7: gpin_in[8]
			 ...
 */
usb_request_status_t usb_vendor_request_gpio_read(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t gpio_index;
	uint8_t buffer_index;
	uint8_t buffer_count;
	uint8_t bit_mask;
	struct gpio_t gpio;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		buffer_index = 0;
		gpio_buffer[buffer_index] = 0;
		bit_mask = 0x80;

		for (gpio_index=0; gpio_index<gpio_in_count; gpio_index++) {
			gpio = gpio_in[gpio_index];
			if (gpio_read(&gpio)) {
				gpio_buffer[buffer_index] |= bit_mask;
			}

			if (bit_mask == 0x01) {
				bit_mask = 0x80;
				gpio_buffer[++buffer_index] = 0;
			} else {
				bit_mask >>= 1;
			}
		}

		buffer_count = gpio_in_count / 8;
		if (gpio_in_count % 8 != 0) {
			buffer_count++;
		}

		if (buffer_count > endpoint->setup.length) {
			buffer_count = endpoint->setup.length;
		}

		usb_transfer_schedule_block(endpoint->in, gpio_buffer, buffer_count, NULL, NULL);
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}


/* Reset any registered GPIO pins back to their default state and
 * clear all registrations.
 */
usb_request_status_t usb_vendor_request_gpio_reset(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t i;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		/* change any outputs back to inputs */
		for (i=0; i<gpio_out_count; i++) {
			gpio_input(&gpio_out[i]);
		}

		/* unregister all */
		gpio_in_count = 0;
		gpio_out_count = 0;

		usb_transfer_schedule_ack(endpoint->in);
	}

	return USB_REQUEST_STATUS_OK;
}
