/*
 * Copyright 2016 Dominic Spill
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

#include "usb_api_adc.h"
#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <greatfet_pins.h>
#include <libopencm3/lpc43xx/adc.h>

volatile bool adc_mode_enabled = false;

usb_request_status_t usb_vendor_request_init_adc(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	int i;
	usb_endpoint_init(&usb0_endpoint_bulk_in);
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		adc_mode_enabled = true;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

#define BUF_LEN 10
void adc_mode(void) {
	uint8_t adc_num = 0;
	uint8_t pins = 1;
	uint8_t clkdiv = 45;
	uint8_t clks = 0x2;
	uint8_t i, j = 0;
	uint8_t buf[2][BUF_LEN];

led_off(LED1);
led_off(LED2);
led_off(LED3);
led_off(LED4);

	// adc_init(adc_num, pins, clkdiv, clks);
	
led_on(LED2);
	while(adc_mode_enabled) {
led_toggle(LED1);
		//adc_start(adc_num);
		// adc_read_to_buffer(adc_num, pins, buf, 10);
		for(i=0; i<BUF_LEN; i++) {
			ADC0_CR = ADC_CR_SEL((uint32_t) pins) | 
	        	ADC_CR_CLKDIV((uint32_t) clkdiv) | 
	        	ADC_CR_CLKS((uint32_t) clks) | 
				ADC_CR_PDN;
			ADC0_CR |= ADC_CR_START(1);

			while(!(ADC0_DR0 & ADC_DR_DONE));
			buf[j][i] = (ADC0_DR0>>6) & 0x0ff;
		}
		usb_transfer_schedule_block(
				&usb0_endpoint_bulk_in,
				&buf[j][0], BUF_LEN, 0, 0);
		j = (j+1) % 2;
		//delay(10000000);
	}
led_off(LED2);
}
