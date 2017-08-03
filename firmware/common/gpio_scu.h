/*
 * Copyright (C) 2017 Mike Naberezny <mike@naberezny.com>
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

#ifndef __GPIO_SCU_H__
#define __GPIO_SCU_H__

#include <libopencm3/lpc43xx/scu.h>

#define GPIO_MAX_PORTS 6
#define GPIO_MAX_PORT_BITS 20

scu_grp_pin_t get_scu_pin_for_gpio(uint8_t gpio_port, uint8_t gpio_pin);
uint32_t get_scu_func_for_gpio(uint8_t gpio_port, uint8_t gpio_pin);

#endif /*__GPIO_SCU_H__*/
