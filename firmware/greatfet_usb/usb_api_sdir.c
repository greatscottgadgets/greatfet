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

static struct gpio_t ir_tx[] = {
	GPIO(1, 0),
	GPIO(1, 1),
	GPIO(1, 2),
	GPIO(1, 3),
	GPIO(1, 4),
	GPIO(1, 5),
	GPIO(1, 6),
	GPIO(1, 7)
};

static struct gpio_t ir_tx_sleep = GPIO(2, 9);
static struct gpio_t ir_tx_clk = GPIO(1, 9);
static struct gpio_t ir_tx_cmode = GPIO(2, 11);
static struct gpio_t ir_tx_mode = GPIO(2, 12);
static struct gpio_t ir_tx_pin = GPIO(2, 14);
static struct gpio_t ir_tx_refio = GPIO(1, 8);

static uint8_t ir_tx_value = 0;
uint32_t samplerate = 0;

void sdir_tx_isr() {
	TIMER2_IR = TIMER_IR_MR0INT;
	ir_tx_value++;
	gpio_write_multiple(&ir_tx[0], ir_tx_value);
}

#define TIMER_CLK_SPEED 204000000
#define TIMER_PRESCALER 0

void sdir_tx_mode(uint32_t samplerate) {
led_off(LED2);
led_off(LED3);
led_off(LED4);

	/* GPIO Tx pin */	
#ifndef NXP_XPLORER
	int i;
	scu_pinmux(SCU_PINMUX_GPIO1_0, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_1, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_2, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_3, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_4, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_5, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_6, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_7, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);

	scu_pinmux(SCU_PINMUX_GPIO1_8, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_9, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_9, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_11, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_12, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_14, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	for(i=0; i<8; i++)
		gpio_output(&ir_tx[i]);

	gpio_output(&ir_tx_sleep);
	gpio_input(&ir_tx_clk);

	gpio_input(&ir_tx_cmode);
	gpio_input(&ir_tx_mode);
	gpio_input(&ir_tx_pin);
	gpio_input(&ir_tx_refio);
#endif

	gpio_write(&ir_tx_sleep, 0);
	/*
	 * TIMER1 produces the TX DAC clock signal
	 * TIMER2 is derived from TIMER1 and triggers DAC data signals
	 */
	timer_disable_counter(TIMER1);
	timer_disable_counter(TIMER2);
	scu_pinmux(P5_4, (SCU_GPIO_FAST | SCU_CONF_FUNCTION5));
	vector_table.irq[NVIC_TIMER2_IRQ] = sdir_tx_isr;
	nvic_set_priority(NVIC_TIMER2_IRQ, 0);
	nvic_enable_irq(NVIC_TIMER2_IRQ);
	led_toggle(LED2);
	TIMER1_MCR = TIMER_MCR_MR0R;
	TIMER1_MR0 = (TIMER_CLK_SPEED / (2*(TIMER_PRESCALER + 1))) / samplerate;
	TIMER1_MR3 = (TIMER_CLK_SPEED / (2*(TIMER_PRESCALER + 1))) / samplerate;
	TIMER1_EMR = (TIMER_EMR_EMC_TOGGLE << TIMER_EMR_EMC0_SHIFT) | (TIMER_EMR_EMC_TOGGLE << TIMER_EMR_EMC3_SHIFT);
	timer_set_prescaler(TIMER1, TIMER_PRESCALER);
	timer_set_mode(TIMER1, TIMER_CTCR_MODE_TIMER);
	timer_reset(TIMER1);
	/* T1_MAT3 is connected to T2_CAP3 through the GIMA by default */
	TIMER2_MCR = (TIMER_MCR_MR0I | TIMER_MCR_MR0R);
	TIMER2_MR0 = 2;
	TIMER2_CCR = 0;
	timer_set_prescaler(TIMER2, TIMER_PRESCALER);
	timer_set_mode(TIMER2, (TIMER_CTCR_MODE_COUNTER_BOTH | TIMER_CTCR_CINSEL_CAPN_3));
	timer_reset(TIMER2);
	timer_enable_counter(TIMER2);
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
		samplerate = ((uint32_t)endpoint->setup.value) << 16 
		             | endpoint->setup.index;
		sdir_tx_mode(samplerate);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
