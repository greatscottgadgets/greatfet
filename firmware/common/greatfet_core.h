/*
 * Copyright 2012 Michael Ossmann <mike@ossmann.com>
 * Copyright 2012 Benjamin Vernoux <titanmkd@gmail.com>
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
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

#ifndef __GREATFET_CORE_H
#define __GREATFET_CORE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#include "spi_ssp.h"
#include "spiflash.h"

/* hardware identification number */
#define BOARD_ID_AZALEA  0

#ifdef GREATFET_AZALEA
#define BOARD_ID BOARD_ID_AZALEA
#endif

void delay(uint32_t duration);

/* TODO: Hide these configurations */
extern const ssp_config_t ssp_config_spi;
extern spi_bus_t spi_bus_ssp0;
extern spi_bus_t spi_bus_ssp1;

//extern spiflash_driver_t spi_flash;

void cpu_clock_init(void);
void cpu_clock_pll1_low_speed(void);
void cpu_clock_pll1_max_speed(void);

void pin_setup(void);

void enable_1v8_power(void);
void disable_1v8_power(void);

typedef enum {
	LED1 = 0,
	LED2 = 1,
	LED3 = 2,
	LED4 = 3,
} led_t;

void led_on(const led_t led);
void led_off(const led_t led);
void led_toggle(const led_t led);

void debug_led(uint8_t val);

#ifdef __cplusplus
}
#endif

#endif /* __GREATFET_CORE_H */
