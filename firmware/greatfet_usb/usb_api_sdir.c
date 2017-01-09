/*
 * Copyright 2016 Jared Boone <jared@sharebrained.com>
 * Copyright 2017 Dominic Spill <dominicgs@gmail.com>
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

#include "usb_api_sdir.h"

#include "sgpio.h"
#include "sgpio_isr.h"

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"
#include "usb_bulk_buffer.h"

#include <greatfet_core.h>
#include <gpio_lpc.h>
#include <gpio.h>
#include <pins.h>

#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/lpc43xx/sgpio.h>
#include <libopencm3/cm3/vector.h>

volatile bool sdir_enabled = false;

static const sgpio_config_t sgpio_config = {
	.slice_mode_multislice = true,
	.clock_divider = 20,
};

static void sdir_sgpio_start() {
	sgpio_configure_pin_functions(&sgpio_config);
	sgpio_configure(&sgpio_config, SGPIO_DIRECTION_INPUT);
	sgpio_clock_out_configure(20);

	/* Enable Gladiolus parts */
	scu_pinmux(SCU_PINMUX_GPIO5_3, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_PINMUX_GPIO5_5, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
	struct gpio_t gladiolus_powerdown = GPIO(5, 3);
	struct gpio_t gladiolus_enable = GPIO(5, 5);
	gpio_output(&gladiolus_enable);
	gpio_output(&gladiolus_powerdown);
	gpio_write(&gladiolus_enable, 1);
	gpio_write(&gladiolus_powerdown, 0);

	vector_table.irq[NVIC_SGPIO_IRQ] = sgpio_isr_input;

	nvic_set_priority(NVIC_SGPIO_IRQ, 0);
	nvic_enable_irq(NVIC_SGPIO_IRQ);
	SGPIO_SET_EN_1 = (1 << SGPIO_SLICE_A);
}

static void sdir_sgpio_stop() {
	SGPIO_CLR_EN_1 = (1 << SGPIO_SLICE_A);

	nvic_disable_irq(NVIC_SGPIO_IRQ);
}

void sdir_mode(void) {
	usb_endpoint_init(&usb0_endpoint_bulk_in);

	sdir_sgpio_start();

	unsigned int phase = 1;
	while(sdir_enabled) {
 		if ( usb_bulk_buffer_offset >= 0x4000
 		     && phase == 1) {
 			usb_transfer_schedule_block(
 				&usb0_endpoint_bulk_in,
 				&usb_bulk_buffer[0x0000],
 				0x4000, 0, 0
 				);
 			phase = 0;
 		}
 		// Set up IN transfer of buffer 1.
 		if ( usb_bulk_buffer_offset < 0x4000
 		     && phase == 0) {
 			usb_transfer_schedule_block(
 				&usb0_endpoint_bulk_in,
 				&usb_bulk_buffer[0x4000],
 				0x4000, 0, 0
 			);
 			phase = 1;
 		}
	}

	sdir_sgpio_stop();

	// usb_endpoint_disable(&usb0_endpoint_bulk_in);
}

usb_request_status_t usb_vendor_request_sdir_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		sdir_enabled = true;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_sdir_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		sdir_enabled = false;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
