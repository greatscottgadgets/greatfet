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
#include <libopencm3/lpc43xx/timer.h>

#include <greatfet_core.h>

#include "usb.h"
#include "usb_standard_request.h"

#include <rom_iap.h>
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_endpoint.h"
#include "usb_api_board_info.h"
#include "usb_api_spiflash.h"
#include "usb_api_gpio.h"
//#include "usb_api_spiflash_spansion.h"
#include "usb_bulk_buffer.h"

#include <stdlib.h>

#include "gpio_lpc.h"


typedef enum {
        TRANSCEIVER_MODE_OFF = 0,
        TRANSCEIVER_MODE_RX = 1,
        TRANSCEIVER_MODE_TX = 2,
        TRANSCEIVER_MODE_SS = 3,
} transceiver_mode_t;

static volatile transceiver_mode_t _transceiver_mode = TRANSCEIVER_MODE_OFF;
static volatile bool led3_on = false;
static volatile int led_count = 0;

void timer2_isr_idle() {
	TIMER2_IR |= TIMER_IR_MR0INT;

	led_count++;
	if(led_count % 100 == 0) {
		led_on(LED1);
	}
	if(led_count % 30 == 0) {
		led_off(LED1);
	}

	if(led3_on) {
		led3_on = false;
		led_off(LED3);
	} else {
		led3_on = true;
		led_on(LED3);
	}

	//delay(5000000);
	led_off(LED4);
	led_off(LED2);

}

void timer2_isr_tx() {
	TIMER2_IR |= TIMER_IR_MR0INT;
	led_on(LED3);

}

void timer2_isr_rx() {
	TIMER2_IR |= TIMER_IR_MR0INT;
	led_on(LED4);
	led_off(LED3);

	led_count++;

	uint32_t gpio_bits = GPIO_LPC_W(1, 0);
	uint8_t byte0 = gpio_bits & 0xff;
	uint8_t byte1 = (gpio_bits >> 8) & 0xff;
	uint8_t byte2 = (gpio_bits >> 16) & 0xff;
	uint8_t byte3 = (gpio_bits >> 24) & 0xff;
	
	usb_bulk_buffer[usb_bulk_buffer_offset++] = byte0;
	if(usb_bulk_buffer_offset >= 0x8000) {
		usb_bulk_buffer_offset = 0;
	}
	usb_bulk_buffer[usb_bulk_buffer_offset++] = byte1;
	if(usb_bulk_buffer_offset >= 0x8000) {
		usb_bulk_buffer_offset = 0;
	}
	usb_bulk_buffer[usb_bulk_buffer_offset++] = byte2;
	if(usb_bulk_buffer_offset >= 0x8000) {
		usb_bulk_buffer_offset = 0;
	}
	usb_bulk_buffer[usb_bulk_buffer_offset++] = byte3;
	if(usb_bulk_buffer_offset >= 0x8000) {
		usb_bulk_buffer_offset = 0;
	}
}


void set_transceiver_mode(const transceiver_mode_t new_transceiver_mode) {
        //baseband_streaming_disable(&sgpio_config);

        usb_endpoint_disable(&usb0_endpoint_bulk_in);
        usb_endpoint_disable(&usb0_endpoint_bulk_out);

        _transceiver_mode = new_transceiver_mode;

        if( _transceiver_mode == TRANSCEIVER_MODE_RX ) {
                led_off(LED3);
                led_on(LED2);
                usb_endpoint_init(&usb0_endpoint_bulk_in);

                vector_table.irq[NVIC_TIMER2_IRQ] = timer2_isr_rx;
		timer_enable_counter(TIMER2);
        } else if (_transceiver_mode == TRANSCEIVER_MODE_TX) {
                led_off(LED2);
                led_on(LED3);
                usb_endpoint_init(&usb0_endpoint_bulk_out);

		vector_table.irq[NVIC_TIMER2_IRQ] = timer2_isr_tx;
		timer_enable_counter(TIMER2);
        } else {
                led_off(LED2);
                led_off(LED3);

		vector_table.irq[NVIC_TIMER2_IRQ] = timer2_isr_idle;
		timer_disable_counter(TIMER2);
        }

        if( _transceiver_mode != TRANSCEIVER_MODE_OFF ) {
                //baseband_streaming_enable(&sgpio_config);
        }
}

transceiver_mode_t transceiver_mode(void) {
        return _transceiver_mode;
}

usb_request_status_t usb_vendor_request_set_transceiver_mode(
        usb_endpoint_t* const endpoint,
        const usb_transfer_stage_t stage
) {
        if( stage == USB_TRANSFER_STAGE_SETUP ) {
                switch( endpoint->setup.value ) {
                case TRANSCEIVER_MODE_OFF:
                case TRANSCEIVER_MODE_RX:
                case TRANSCEIVER_MODE_TX:
                        set_transceiver_mode(endpoint->setup.value);
                        usb_transfer_schedule_ack(endpoint->in);
                        return USB_REQUEST_STATUS_OK;
                default:
                        return USB_REQUEST_STATUS_STALL;
                }
        } else {
                return USB_REQUEST_STATUS_OK;
        }
}

usb_request_status_t usb_vendor_request_enable_usb1(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

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
	NULL,
	usb_vendor_request_set_transceiver_mode,
	usb_vendor_request_erase_spiflash,
	usb_vendor_request_write_spiflash,
	usb_vendor_request_read_spiflash,
	usb_vendor_request_read_board_id,
	usb_vendor_request_read_version_string,
	usb_vendor_request_read_partid_serialno,
	usb_vendor_request_enable_usb1,
	usb_vendor_request_led_toggle,
	//usb_vendor_request_read_spiflash_spansion,
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

const usb_request_handlers_t usb1_request_handlers = {
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
	usb_endpoint_init(&usb0_endpoint_bulk_out);
	usb_endpoint_init(&usb0_endpoint_bulk_in);

	nvic_set_priority(NVIC_USB0_IRQ, 254);

	usb_run(&usb0_device);
}

void usb1_configuration_changed(usb_device_t* const device)
{
	if( device->configuration->number == 1 ) {
		led_on(LED1);
	}
}

void init_usb1(void) {
	usb_set_configuration_changed_cb(usb1_configuration_changed);

	usb_peripheral_reset(&usb1_device);

	usb_device_init(&usb1_device);

	usb_queue_init(&usb1_endpoint_control_out_queue);
	usb_queue_init(&usb1_endpoint_control_in_queue);

	usb_endpoint_init(&usb1_endpoint_control_out);
	usb_endpoint_init(&usb1_endpoint_control_in);

	nvic_set_priority(NVIC_USB1_IRQ, 255);
	usb_run(&usb1_device);
}


usb_request_status_t usb_vendor_request_enable_usb1(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		init_usb1();
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

void tim_setup(void)
{
	vector_table.irq[NVIC_TIMER2_IRQ] = timer2_isr_idle;

	/* Enable TIM2 interrupt. */
        nvic_set_priority(NVIC_TIMER2_IRQ, 10);
	nvic_enable_irq(NVIC_TIMER2_IRQ);

	timer_disable_counter(TIMER2);
	timer_reset(TIMER2);

	TIMER_MR3(TIMER2) = 250;	// match count

	TIMER_MCR(TIMER2) &= ~(TIMER_MCR_MR3I | TIMER_MCR_MR3R | TIMER_MCR_MR3S); // clear out mcr bits
	TIMER_MCR(TIMER2) |= TIMER_MCR_MR3R;	// enable counter reset on match
	TIMER_MCR(TIMER2) |= TIMER_MCR_MR3I;	// enable interrupt on match


//	timer_set_counter(TIMER2, 0xffffffff);

//	timer_set_mode(TIMER2, 0);

	timer_enable_counter(TIMER2);
}

int main(void) {
	pin_setup();
	led_on(LED1);
	cpu_clock_init();
	led_on(LED2);
	led_off(LED3);

	init_usb0();

	tim_setup();
	
	//int delay_time = 5000000;
	//unsigned long flash_counter = 0;
	bool tx_led_on = false;

        int phase = 0;
	//unsigned char cur_char = 0x01;

	while(usb_bulk_buffer_offset++ < 0x4000) {
		usb_bulk_buffer[usb_bulk_buffer_offset] = 0xa0;
	}

	while(true) {
		/* Blink LED4 to let us know we're alive */
/*
		led_off(LED1);
		led_on(LED2);

		delay(delay_time);

		led_on(LED1);
		led_off(LED2);

		delay(delay_time);
*/

                // Set up IN transfer of buffer 0.
                //if ( usb_bulk_buffer_offset >= 16384
                if (usb_bulk_buffer_offset >= 16384
                     && phase == 1
                     && transceiver_mode() != TRANSCEIVER_MODE_OFF) {
			/*
			usb_bulk_buffer_offset = 0;
			cur_char++;
			while(usb_bulk_buffer_offset++ < 0x2000) {
				usb_bulk_buffer[usb_bulk_buffer_offset] = cur_char;
			}
			*/

                        usb_transfer_schedule_block(
                                (transceiver_mode() == TRANSCEIVER_MODE_RX)
                                ? &usb0_endpoint_bulk_in : &usb0_endpoint_bulk_out,
                                &usb_bulk_buffer[0x0000],
                                0x4000,
                                NULL, NULL
                                );
                        phase = 0;

			if(tx_led_on) {
				tx_led_on = false;
				led_off(LED1);
				led_on(LED2);
			} else {
				tx_led_on = true;
				led_on(LED1);
				led_off(LED2);
			}
                }

                // Set up IN transfer of buffer 1.
                //if ( usb_bulk_buffer_offset < 16384
                if (usb_bulk_buffer_offset < 16384
                     && phase == 0
                     && transceiver_mode() != TRANSCEIVER_MODE_OFF) {
			/*
			usb_bulk_buffer_offset = 0x2000;
			while(usb_bulk_buffer_offset++ < 0x4000) {
				usb_bulk_buffer[usb_bulk_buffer_offset] = 0xc0;
			}
			*/

                        usb_transfer_schedule_block(
                                (transceiver_mode() == TRANSCEIVER_MODE_RX)
                                ? &usb0_endpoint_bulk_in : &usb0_endpoint_bulk_out,
                                &usb_bulk_buffer[0x4000],
                                0x4000,
                                NULL, NULL
                        );
                        phase = 1;
                }
/*
*/

	}
	
	return 0;
}
