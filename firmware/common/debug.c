/*
 * This file is part of GreatFET
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
