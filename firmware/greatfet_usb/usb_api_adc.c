/*
 * This file is part of GreatFET
 */

#include "usb_api_adc.h"
#include <drivers/usb/lpc43xx/usb.h>
#include <drivers/usb/lpc43xx/usb_queue.h>
#include "usb_endpoint.h"
#include "usb_bulk_buffer.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <libopencm3/lpc43xx/adc.h>
// #include <libopencm3/lpc43xx/m4/nvic.h>
// #include <libopencm3/cm3/vector.h>

volatile bool adc_mode_enabled = false;

usb_request_status_t usb_vendor_request_adc_init(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	usb_endpoint_init(&usb0_endpoint_bulk_in);
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		adc_mode_enabled = true;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_adc_read(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	uint8_t pins = 1; 		// pins start counting from 1, this is pin 0
	uint8_t clkdiv = 45;	// divide 204MHz clock by 45 to get ~4MHz
	uint8_t clks = 0x2;
	uint16_t value;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		ADC0_CR = ADC_CR_SEL((uint32_t) pins) |	
		ADC_CR_CLKDIV((uint32_t) clkdiv) |
		ADC_CR_CLKS((uint32_t) clks) |
		ADC_CR_PDN;
		ADC0_CR |= ADC_CR_START(1);

		while(!(ADC0_DR0 & ADC_DR_DONE));
		value = (ADC0_DR0>>6) & 0x3ff;

		usb_transfer_schedule_block(endpoint->in, &value, 2, NULL, NULL);
		
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->out);
	}

	return USB_REQUEST_STATUS_OK;
}

#define BLK_LEN 0x4000

void adc_mode(void) {
	uint8_t pins = 1;
	uint8_t clkdiv = 45;
	uint8_t clks = 0x2;
	uint16_t i, j = 0;

// vector_table.irq[NVIC_ADC0_IRQ] = adc0_isr;
	// adc_init(adc_num, pins, clkdiv, clks);

	ADC0_CR = ADC_CR_SEL((uint32_t) pins) |
	    	ADC_CR_CLKDIV((uint32_t) clkdiv) |
	    	ADC_CR_CLKS((uint32_t) clks) |
			ADC_CR_PDN;
	ADC0_CR |= ADC_CR_START(1);
	while(adc_mode_enabled) {
		//adc_start(adc_num);
		// adc_read_to_buffer(adc_num, pins, buf, 10);

		for(i=0; i<BLK_LEN; i++) {
			while(!(ADC0_DR0 & ADC_DR_DONE));
			ADC0_CR |= ADC_CR_START(1);
			usb_bulk_buffer[i+j] = (ADC0_DR0>>8) & 0x0ff;
		}
		usb_transfer_schedule_block(
				&usb0_endpoint_bulk_in,
				&usb_bulk_buffer[j], BLK_LEN, 0, 0);
		j = (j+BLK_LEN) % 0x8000;
	}
}
