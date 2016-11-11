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
#include "usb_bulk_buffer.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <libopencm3/lpc43xx/adc.h>
// #include <libopencm3/lpc43xx/m4/nvic.h>
// #include <libopencm3/cm3/vector.h>

volatile bool adc_mode_enabled = false;

usb_request_status_t usb_vendor_request_init_adc(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	usb_endpoint_init(&usb0_endpoint_bulk_in);
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		adc_mode_enabled = true;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

// int currentResult=0;

// //AD0 Interrupt Function
// void adc0_isr(void) {
// 	unsigned long dummyRead;
// 	unsigned long ACD0_GDR_Read = ADC0_GDR;
	
// 	//Extract Conversion Result
// 	currentResult = (ACD0_GDR_Read>>6) & 0x3FF;
// 	//Read to Clear Done flag , Also clears AD0 interrupt
// 	dummyRead = ADC0_DR0;
// 	//Signal that ISR has finished
// 	//VICVectAddr = 0x0;
// }

#define BLK_LEN 0x4000
void adc_mode(void) {
	uint8_t adc_num = 0;
	uint8_t pins = 1;
	uint8_t clkdiv = 45;
	uint8_t clks = 0x2;
	uint16_t i, j = 0;


// vector_table.irq[NVIC_ADC0_IRQ] = adc0_isr;
led_off(LED1);
led_off(LED2);
led_off(LED3);
led_off(LED4);

	// adc_init(adc_num, pins, clkdiv, clks);
	
led_on(LED2);
	ADC0_CR = ADC_CR_SEL((uint32_t) pins) | 
	    	ADC_CR_CLKDIV((uint32_t) clkdiv) | 
	    	ADC_CR_CLKS((uint32_t) clks) | 
			ADC_CR_PDN;
	ADC0_CR |= ADC_CR_START(1);
	while(adc_mode_enabled) {
led_toggle(LED1);
		//adc_start(adc_num);
		// adc_read_to_buffer(adc_num, pins, buf, 10);

		for(i=0; i<BLK_LEN; i++) {
			while(!(ADC0_DR0 & ADC_DR_DONE));
			ADC0_CR |= ADC_CR_START(1);
			usb_bulk_buffer[i+j] = (ADC0_DR0>>6) & 0x0ff;
		}
		usb_transfer_schedule_block(
				&usb0_endpoint_bulk_in,
				&usb_bulk_buffer[j], BLK_LEN, 0, 0);
		j = (j+BLK_LEN) % 0x8000;
	}
led_off(LED2);
}
