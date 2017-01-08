/*
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 *
 * This file is part of HackRF.
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

#ifndef __SGPIO_H__
#define __SGPIO_H__

#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/lpc43xx/sgpio.h>

#include "gpio.h"

typedef enum {
	SGPIO_DIRECTION_INPUT,
	SGPIO_DIRECTION_OUTPUT,
} sgpio_direction_t;

typedef struct sgpio_config_t {
	bool slice_mode_multislice;
	uint16_t clock_divider;
} sgpio_config_t;

void sgpio_configure_pin_functions(const sgpio_config_t* const config);
void sgpio_set_slice_mode(
	sgpio_config_t* const config,
	const bool multi_slice
);
void sgpio_configure(
	const sgpio_config_t* const config,
	const sgpio_direction_t direction
);
void sgpio_clock_out_configure(uint16_t clock_divider);

#endif//__SGPIO_H__
