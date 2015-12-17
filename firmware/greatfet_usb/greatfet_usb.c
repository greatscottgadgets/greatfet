/*
 * Copyright 2012 Jared Boone
 * Copyright 2013 Benjamin Vernoux
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

#include <stddef.h>

#include <libopencm3/cm3/vector.h>

#include <libopencm3/lpc43xx/gpio.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/lpc43xx/scu.h>

#include "greatfet_core.h"

#include "usb.h"
#include "usb_standard_request.h"

#include <rom_iap.h>
#include "usb_descriptor.h"

#include "usb_device.h"
#include "usb_endpoint.h"
#include "usb_api_board_info.h"
#include "usb_api_spiflash.h"

#include "sgpio_isr.h"
#include "usb_bulk_buffer.h"

#define SCU_PINMUX_AZALEA_LED1     (P7_6)  /* GPIO3[14] on P7_6 */
#define SCU_PINMUX_AZALEA_LED2     (P4_1)  /* GPIO2[1] on P4_1 */
#define SCU_PINMUX_AZALEA_LED3     (P7_5)  /* GPIO3[13] on P7_5 */
#define SCU_PINMUX_AZALEA_LED4     (P7_4)  /* GPIO3[12] on P7_4 */

#define PIN_AZALEA_LED1            (BIT14) /* GPIO3[14] on P7_6 */
#define PIN_AZALEA_LED2            (BIT1)  /* GPIO2[1] on P4_1 */
#define PIN_AZALEA_LED3            (BIT13) /* GPIO3[13] on P7_5 */
#define PIN_AZALEA_LED4            (BIT12) /* GPIO3[12] on P7_4 */

#define PORT_AZALEA_LED1_3_4       (GPIO3) /* PORT for LED1, 3, 4 */
#define PORT_AZALEA_LED2           (GPIO2) /* PORT for LED1, 3, 4 */
 
static const usb_request_handler_fn vendor_request_handler[] = {
	NULL,
	usb_vendor_request_erase_spiflash,
	usb_vendor_request_write_spiflash,
	usb_vendor_request_read_spiflash,
	usb_vendor_request_read_board_id,
	usb_vendor_request_read_version_string,
	usb_vendor_request_read_partid_serialno
};

static const uint32_t vendor_request_handler_count =
	sizeof(vendor_request_handler) / sizeof(vendor_request_handler[0]);

usb_request_status_t usb_vendor_request(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage
) {
	usb_request_status_t status = USB_REQUEST_STATUS_STALL;
	
	if( endpoint->setup.request < vendor_request_handler_count ) {
		usb_request_handler_fn handler = vendor_request_handler[endpoint->setup.request];
		if( handler ) {
			status = handler(endpoint, stage);
		}
	}
	
	return status;
}

const usb_request_handlers_t usb_request_handlers = {
	.standard = usb_standard_request,
	.class = 0,
	.vendor = usb_vendor_request,
	.reserved = 0,
};

void usb_configuration_changed(
	usb_device_t* const device
) {
	/* Reset transceiver to idle state until other commands are received */
	if( device->configuration->number == 1 ) {
		// transceiver configuration
		cpu_clock_pll1_max_speed();
		gpio_set(PORT_LED1_3, PIN_LED1);
	} else {
		/* Configuration number equal 0 means usb bus reset. */
		cpu_clock_pll1_low_speed();
		gpio_clear(PORT_LED1_3, PIN_LED1);
	}
}

void usb_set_descriptor_by_serial_number(void)
{
	iap_cmd_res_t iap_cmd_res;
	
	/* Read IAP Serial Number Identification */
	iap_cmd_res.cmd_param.command_code = IAP_CMD_READ_SERIAL_NO;
	iap_cmd_call(&iap_cmd_res);
	
	if (iap_cmd_res.status_res.status_ret == CMD_SUCCESS) {
		usb_descriptor_string_serial_number[0] = USB_DESCRIPTOR_STRING_SERIAL_BUF_LEN;
		usb_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;
		
		/* 32 characters of serial number, convert to UTF-16LE */
		for (size_t i=0; i<USB_DESCRIPTOR_STRING_SERIAL_LEN; i++) {
			const uint_fast8_t nibble = (iap_cmd_res.status_res.iap_result[i >> 3] >> (28 - (i & 7) * 4)) & 0xf;
			const char c = (nibble > 9) ? ('a' + nibble - 10) : ('0' + nibble);
			usb_descriptor_string_serial_number[2 + i * 2] = c;
			usb_descriptor_string_serial_number[3 + i * 2] = 0x00;
		}
	} else {
		usb_descriptor_string_serial_number[0] = 2;
		usb_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;
	}
}

#define DGS_REMOVE_ME 0
int main(void) {
	pin_setup();

	/* Configure SCU Pin Mux as GPIO */
	scu_pinmux(SCU_PINMUX_AZALEA_LED1, SCU_GPIO_NOPULL);
	scu_pinmux(SCU_PINMUX_AZALEA_LED2, SCU_GPIO_NOPULL);
	scu_pinmux(SCU_PINMUX_AZALEA_LED3, SCU_GPIO_NOPULL);
	scu_pinmux(SCU_PINMUX_AZALEA_LED4, SCU_GPIO_NOPULL);

	/* Configure GPIO2[1/2/8] (P4_1/2 P6_12) as output. */
	GPIO2_DIR |= PIN_AZALEA_LED2;
	GPIO3_DIR |= (PIN_AZALEA_LED1 | PIN_AZALEA_LED3 | PIN_AZALEA_LED4);

	gpio_clear(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED4); /* LED4 on */

	cpu_clock_init();

	usb_set_descriptor_by_serial_number();
	gpio_set(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED4); /* LED4 off */

	usb_set_configuration_changed_cb(usb_configuration_changed);
	usb_peripheral_reset();
	
	usb_device_init(0, &usb_device);
	
	usb_queue_init(&usb_endpoint_control_out_queue);
	usb_queue_init(&usb_endpoint_control_in_queue);
	usb_queue_init(&usb_endpoint_bulk_out_queue);
	usb_queue_init(&usb_endpoint_bulk_in_queue);

	usb_endpoint_init(&usb_endpoint_control_out);
	usb_endpoint_init(&usb_endpoint_control_in);
	
	nvic_set_priority(NVIC_USB0_IRQ, 255);

	usb_run(&usb_device);
	
	//ssp1_init();

	unsigned int i = 0;
	while(true) {
		gpio_set(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED4); /* LED4 off */
		gpio_clear(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED1); /* LED1 on */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");

		gpio_set(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED1); /* LED1 off */
		gpio_clear(PORT_AZALEA_LED2, (PIN_AZALEA_LED2)); /* LED2 on */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");

		gpio_set(PORT_AZALEA_LED2, (PIN_AZALEA_LED2)); /* LED2 off */
		gpio_clear(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED3); /* LED3 on */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");

		gpio_set(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED3); /* LED3 off */
		gpio_clear(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED4); /* LED4 on */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");
	}
	
	return 0;
}
