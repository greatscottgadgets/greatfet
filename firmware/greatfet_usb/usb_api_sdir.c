/*
 * Copyright 2016 Jared Boone <jared@sharebrained.com>
 * Copyright 2017 Dominic Spill <dominicgs@gmail.com>
 * Copyright 2017 Schuyler St. Leger <schuyler.st.leger@gmail.com>
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
#include <libopencm3/lpc43xx/timer.h>

volatile bool sdir_enabled = false;
volatile bool sdir_tx_enabled = false;

static const sgpio_config_t sgpio_config = {
	.slice_mode_multislice = true,
	.clock_divider = 20,
};

static struct gpio_t ir_tx_pin = GPIO(1, 3);
static uint8_t ir_tx_buf[0x100U];
static uint16_t tx_buf_len;
static uint16_t tx_buf_idx = 0;

uint32_t samplerate = 0;

void sdir_tx_isr() {
	TIMER1_IR = 1;
	led_toggle(LED3);
	gpio_write(&ir_tx_pin, (ir_tx_buf[tx_buf_idx]!=0x00));
	// gpio_write(&ir_tx_pin, 1);
	if(++tx_buf_idx != tx_buf_len) {
		timer_reset(TIMER1);
	} else {
		gpio_write(&ir_tx_pin, 0);
		timer_disable_counter(TIMER1);
		led_toggle(LED4);		
	}
}

#define TIMER_CLK_SPEED 204000000
#define TIMER_PRESCALER 1

void sdir_tx_mode(uint32_t samplerate) {
led_off(LED2);
led_off(LED3);
led_off(LED4);


	/* GPIO Tx pin */	
	scu_pinmux(SCU_PINMUX_SD_DAT1, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	gpio_output(&ir_tx_pin);


	/* Timer to update Tx pin */
	vector_table.irq[NVIC_TIMER1_IRQ] = sdir_tx_isr;
	nvic_set_priority(NVIC_TIMER1_IRQ, 0);
	nvic_enable_irq(NVIC_TIMER1_IRQ);
	led_toggle(LED2);
	TIMER1_MCR = 1;
	TIMER1_MR0 = (TIMER_CLK_SPEED / (TIMER_PRESCALER + 1)) / samplerate;
	timer_set_prescaler(TIMER1, TIMER_PRESCALER);
	timer_set_mode(TIMER1, TIMER_CTCR_MODE_TIMER);
	timer_reset(TIMER1);
	tx_buf_idx = 0;
	timer_enable_counter(TIMER1);
}

static void sdir_sgpio_start() {
	sgpio_configure_pin_functions(&sgpio_config);
	sgpio_configure(&sgpio_config, SGPIO_DIRECTION_INPUT);
	sgpio_clock_out_configure(20);

	/* Enable Gladiolus parts */
#ifndef NXP_XPLORER
	scu_pinmux(SCU_PINMUX_GPIO5_3, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_PINMUX_GPIO5_5, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
#endif


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

usb_request_status_t usb_vendor_request_sdir_tx(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->out, &ir_tx_buf[0],
									endpoint->setup.length, NULL, NULL);
		samplerate = ((uint32_t)endpoint->setup.value) << 16 
		             | endpoint->setup.index;
		tx_buf_len = endpoint->setup.length;
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		sdir_tx_mode(samplerate);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
