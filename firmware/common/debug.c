/*
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

#include <pins.h>
#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/uart.h>
#include "debug.h"

void log_init(void) {
	scu_pinmux(SCU_PINMUX_GPIO3_9, SCU_UART_RX_TX | SCU_CONF_FUNCTION6);
	scu_pinmux(SCU_PINMUX_GPIO3_10, SCU_UART_RX_TX | SCU_CONF_FUNCTION6);
    uart_init(UART2, UART_DATABIT_8, UART_STOPBIT_1, UART_PARITY_NONE,
              10625, 0, 1);
	    // uint16_t uart_divisor, uint8_t uart_divaddval, uint8_t uart_mulval);
}

void log_text(char* text, uint8_t length) {
    int i;
    for(i=0; i<length; i++) {
        uart_write(UART2, text[i]);
    }
}
