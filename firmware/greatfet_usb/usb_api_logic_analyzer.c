/*
 * Copyright 2016 Jared Boone <jared@sharebrained.com>
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

#include "usb_api_logic_analyzer.h"

#include "sgpio.h"
#include "sgpio_isr.h"

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"
#include "usb_bulk_buffer.h"

#include <greatfet_core.h>

#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/lpc43xx/sgpio.h>
#include <libopencm3/cm3/vector.h>

volatile bool logic_analyzer_enabled = false;

static const sgpio_config_t sgpio_config = {
	.slice_mode_multislice = true,
	.clock_divider = 4,
};

static void logic_analyzer_sgpio_start() {
	sgpio_configure_pin_functions(&sgpio_config);
	sgpio_configure(&sgpio_config, SGPIO_DIRECTION_INPUT);

	vector_table.irq[NVIC_SGPIO_IRQ] = sgpio_isr_input;

	nvic_set_priority(NVIC_SGPIO_IRQ, 0);
	nvic_enable_irq(NVIC_SGPIO_IRQ);
	SGPIO_SET_EN_1 = (1 << SGPIO_SLICE_A);
}

static void logic_analyzer_sgpio_stop() {
	SGPIO_CLR_EN_1 = (1 << SGPIO_SLICE_A);

	nvic_disable_irq(NVIC_SGPIO_IRQ);
}

void logic_analyzer_mode(void) {
led_off(LED1);
led_off(LED2);
led_off(LED3);
led_off(LED4);

	usb_endpoint_init(&usb0_endpoint_bulk_in);

	logic_analyzer_sgpio_start();

led_on(LED1);
	unsigned int phase = 0;
	while(logic_analyzer_enabled) {
		if ( usb_bulk_buffer_offset >= 16384
		     && phase == 1) {
led_on(LED2);
led_off(LED3);
			usb_transfer_schedule_block(
				&usb0_endpoint_bulk_in,
				&usb_bulk_buffer[0x0000],
				0x4000, 0, 0
				);
			phase = 0;
		}

		// Set up IN transfer of buffer 1.
		if ( usb_bulk_buffer_offset < 16384
		     && phase == 0) {
led_on(LED3);
led_off(LED2);
			usb_transfer_schedule_block(
				&usb0_endpoint_bulk_in,
				&usb_bulk_buffer[0x4000],
				0x4000, 0, 0
			);
			phase = 1;
		}
	}
led_off(LED1);
led_off(LED2);
led_off(LED3);
led_off(LED4);

	logic_analyzer_sgpio_stop();

	usb_endpoint_disable(&usb0_endpoint_bulk_in);
}

usb_request_status_t usb_vendor_request_logic_analyzer_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		logic_analyzer_enabled = true;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_logic_analyzer_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		logic_analyzer_enabled = false;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
