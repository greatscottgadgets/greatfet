/*
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2013 Benjamin Vernoux <titanmkd@gmail.com>
 * Copyright 2015 Dominic Spill <dominicgs@gmail.com>
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
#include <libopencm3/lpc43xx/m4/nvic.h>

#include <greatfet_core.h>

#include "usb.h"
#include "usb_standard_request.h"

#include <rom_iap.h>
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_endpoint.h"
#include "usb_api_board.h"
#include "usb_api_spiflash.h"
#include "usb_api_adc.h"
#include "usb_api_spi.h"
#include "usb_api_i2c.h"
#include "usb_api_gpio.h"
#include "usb_api_logic_analyzer.h"
#include "usb_api_sdir.h"
#include "usb_api_greatdancer.h"
#include "usb_bulk_buffer.h"

usb_request_status_t usb_vendor_request_led_toggle(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		switch(endpoint->setup.value) {
			case 1:
				led_toggle(LED1);
				break;
			case 2:
				led_toggle(LED2);
				break;
			case 3:
				led_toggle(LED3);
				break;
			case 4:
				led_toggle(LED4);
				break;
		}
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

static const usb_request_handler_fn usb0_vendor_request_handler[] = {
	usb_vendor_request_init_spiflash,
	usb_vendor_request_write_spiflash,
	usb_vendor_request_read_spiflash,
	usb_vendor_request_erase_spiflash,
	usb_vendor_request_read_board_id,
	usb_vendor_request_read_version_string,
	usb_vendor_request_read_partid_serialno,
	NULL,
	usb_vendor_request_led_toggle,
	usb_vendor_request_register_gpio,
	usb_vendor_request_write_gpio,
	usb_vendor_request_init_spi,
	usb_vendor_request_spi_write,
	usb_vendor_request_spi_read,
	usb_vendor_request_spi_dump_flash,
	usb_vendor_request_i2c_start,
	usb_vendor_request_i2c_stop,
	usb_vendor_request_i2c_xfer,
	usb_vendor_request_i2c_response,
	usb_vendor_request_logic_analyzer_start,
	usb_vendor_request_logic_analyzer_stop,
	usb_vendor_request_reset,
	usb_vendor_request_adc_init,
	NULL, // ADC read
	NULL, // ADC stream
	usb_vendor_request_sdir_start,
	usb_vendor_request_sdir_stop,
	usb_vendor_request_sdir_tx,
	usb_vendor_request_greatdancer_connect,
	usb_vendor_request_greatdancer_disconnect,
	usb_vendor_request_greatdancer_bus_reset,
	usb_vendor_request_greatdancer_set_address,
	usb_vendor_request_greatdancer_set_up_endpoints,
	usb_vendor_request_greatdancer_get_status,
	usb_vendor_request_greatdancer_read_setup,
	usb_vendor_request_greatdancer_stall_endpoint,
  usb_vendor_request_greatdancer_send_on_endpoint,
  usb_vendor_request_greatdancer_clean_up_transfer,
  usb_vendor_request_greatdancer_start_nonblocking_read,
  usb_vendor_request_greatdancer_finish_nonblocking_read,
  usb_vendor_request_greatdancer_get_nonblocking_data_length,
};

static const uint32_t usb0_vendor_request_handler_count =
	sizeof(usb0_vendor_request_handler) / sizeof(usb0_vendor_request_handler[0]);

usb_request_status_t usb0_vendor_request(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage) {
	usb_request_status_t status = USB_REQUEST_STATUS_STALL;
	
	if( endpoint->setup.request < usb0_vendor_request_handler_count ) {
		usb_request_handler_fn handler = usb0_vendor_request_handler[endpoint->setup.request];
		if( handler ) {
			status = handler(endpoint, stage);
		}
	}
	return status;
}

const usb_request_handlers_t usb0_request_handlers = {
	.standard = usb_standard_request,
	.class = 0,
	.vendor = usb0_vendor_request,
	.reserved = 0,
};


void usb0_configuration_changed(usb_device_t* const device)
{
	if( device->configuration->number == 1 ) {
		cpu_clock_pll1_max_speed();
	} else {
		/* Configuration number equal 0 means usb bus reset. */
		cpu_clock_pll1_low_speed();
	}
}

void usb_set_descriptor_by_serial_number(void)
{
	iap_cmd_res_t iap_cmd_res;
	
	/* Read IAP Serial Number Identification */
	iap_cmd_res.cmd_param.command_code = IAP_CMD_READ_SERIAL_NO;
	iap_cmd_call(&iap_cmd_res);
		
	if (iap_cmd_res.status_res.status_ret == CMD_SUCCESS) {
		usb0_descriptor_string_serial_number[0] = USB_DESCRIPTOR_STRING_SERIAL_BUF_LEN;
		usb0_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;
		
		/* 32 characters of serial number, convert to UTF-16LE */
		for (size_t i=0; i<USB_DESCRIPTOR_STRING_SERIAL_LEN; i++) {
			const uint_fast8_t nibble = (iap_cmd_res.status_res.iap_result[i >> 3] >> (28 - (i & 7) * 4)) & 0xf;
			const char c = (nibble > 9) ? ('a' + nibble - 10) : ('0' + nibble);
			usb0_descriptor_string_serial_number[2 + i * 2] = c;
			usb0_descriptor_string_serial_number[3 + i * 2] = 0x00;
		}
	} else {
		usb0_descriptor_string_serial_number[0] = 8;
		usb0_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;
		usb0_descriptor_string_serial_number[2] = 'G';
		usb0_descriptor_string_serial_number[3] = 0x00;
		usb0_descriptor_string_serial_number[4] = 'S';
		usb0_descriptor_string_serial_number[5] = 0x00;
		usb0_descriptor_string_serial_number[6] = 'G';
		usb0_descriptor_string_serial_number[7] = 0x00;
	}
}

void init_usb0(void) {
	usb_set_descriptor_by_serial_number();

	usb_set_configuration_changed_cb(usb0_configuration_changed);

	usb_peripheral_reset(&usb0_device);
	usb_device_init(&usb0_device);

	usb_queue_init(&usb0_endpoint_control_out_queue);
	usb_queue_init(&usb0_endpoint_control_in_queue);
	usb_queue_init(&usb0_endpoint_bulk_out_queue);
	usb_queue_init(&usb0_endpoint_bulk_in_queue);

	usb_endpoint_init(&usb0_endpoint_control_out);
	usb_endpoint_init(&usb0_endpoint_control_in);

	usb_endpoint_init(&usb0_endpoint_bulk_in);

	nvic_set_priority(NVIC_USB0_IRQ, 254);

	usb_run(&usb0_device);
}


int main(void) {
	pin_setup();
	cpu_clock_init();
	led_off(LED2);
	led_off(LED3);
	led_off(LED4);

	init_usb0();
	init_greatdancer_api();
	
	while(true) {
		if(start_gpio_monitor)
			gpio_monitor_mode();
		if(logic_analyzer_enabled) {
			logic_analyzer_mode();
		}
		if(sdir_enabled) {
			sdir_mode();
		}
		if(adc_mode_enabled) {
			adc_mode();
		}

		/* Blink LED1 to let us know we're alive */
		led_toggle(LED1);
		delay(10000000);
	}
	
	return 0;
}
