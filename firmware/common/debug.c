/*
 * Copyright 2017 Dominic Spill <dominicgs@gmail.com>
 * Copyright 2017 Mike Naberezny <mike@naberezny.com>
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

#include <pins.h>
#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/uart.h>
#include "debug.h"

void debug_init(void) {
	// UART2 TX on P7_1
	scu_pinmux(SCU_PINMUX_GPIO3_9, SCU_UART_RX_TX | SCU_CONF_FUNCTION6);
	// UART2 RX on P7_2
	scu_pinmux(SCU_PINMUX_GPIO3_10, SCU_UART_RX_TX | SCU_CONF_FUNCTION6);
	// 9600-N-8-1
	// Note: UART clock is derived from PLL1; these settings assume that PLL1
	// has been set to maximum (204 MHz) by calling cpu_clock_pll1_max_speed()
  uart_init(UART2, UART_DATABIT_8, UART_STOPBIT_1, UART_PARITY_NONE,
              /* uart_divisor */   1328,
              /* uart_divaddval */ 0,
              /* uart_mulval */    1);
}

void debug_log(char *str) {
  while (*str != '\0')
  {
    uart_write(UART2, *str);
    str++;
  }
}
