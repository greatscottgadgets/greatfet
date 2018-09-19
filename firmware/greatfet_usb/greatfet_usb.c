/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <stdio.h>

/* FIXME: only include this from libgreat */
/* FIXME: replace me with a better name */
#include <toolchain.h>

#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/m4/nvic.h>

#include <greatfet_core.h>

#include "drivers/usb/lpc43xx/usb.h"

#include "usb_request_handlers.h"
#include "usb_api_sdir.h"
#include "usb_api_usbhost.h"
#include "usb_api_logic_analyzer.h"
#include "usb_api_adc.h"
#include "usb_api_heartbeat.h"
#include "usb_api_greatdancer.h"
#include "glitchkit.h"

#include <rom_iap.h>
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_endpoint.h"

#include "usb_bulk_buffer.h"

#include "debug.h"

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
	usb_peripheral_reset(&usb_peripherals[0]);
	usb_device_init(&usb_peripherals[0]);

	usb_queue_init(&usb0_endpoint_control_out_queue);
	usb_queue_init(&usb0_endpoint_control_in_queue);
	usb_queue_init(&usb0_endpoint_bulk_out_queue);
	usb_queue_init(&usb0_endpoint_bulk_in_queue);

	usb_endpoint_init(&usb0_endpoint_control_out);
	usb_endpoint_init(&usb0_endpoint_control_in);

	usb_endpoint_init(&usb0_endpoint_bulk_in);

	nvic_set_priority(NVIC_USB0_IRQ, 254);

	usb_run(&usb_peripherals[0]);
}



int main(void) {
	debug_init();

	cpu_clock_init();
	cpu_clock_pll1_max_speed();
	pin_setup();
	if(heartbeat_mode_enabled) {
		heartbeat_init();
	}

	init_usb0();
	init_greatdancer_api();
	init_usbhost_api();

	while(true) {
		if(logic_analyzer_enabled) {
			logic_analyzer_mode();
		}
		if(sdir_rx_enabled) {
			sdir_rx_mode();
		}
		if(sdir_tx_enabled) {
			sdir_tx_mode();
		}
		if(adc_mode_enabled) {
			adc_mode();
		}
		if(heartbeat_mode_enabled) {
			heartbeat_mode();
		}

		service_glitchkit();
	}

	return 0;
}
