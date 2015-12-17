/*
 * Copyright 2010 - 2012 Michael Ossmann
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

#include <libopencm3/lpc43xx/gpio.h>
#include <libopencm3/lpc43xx/scu.h>

#include "hackrf_core.h"

#define SCU_PINMUX_AZALEA_LED1     (P7_6)  /* GPIO3[14] on P7_6 */
#define SCU_PINMUX_AZALEA_LED2     (P4_1)  /* GPIO2[1] on P4_1 */
#define SCU_PINMUX_AZALEA_LED3     (P7_5)  /* GPIO3[13] on P7_5 */
#define SCU_PINMUX_AZALEA_LED4     (P7_4)  /* GPIO3[12] on P7_4 */

#define PIN_AZALEA_LED1            (BIT14) /* GPIO3[14] on P7_6 */
#define PIN_AZALEA_LED2            (BIT1)  /* GPIO2[1] on P4_1 */
#define PIN_AZALEA_LED3            (BIT13) /* GPIO3[13] on P7_5 */
#define PIN_AZALEA_LED4            (BIT12) /* GPIO3[12] on P7_4 */

#define PORT_AZALEA_LED1_3_4       (GPIO3) /* PORT for LED1, 3, 4 */
#define PORT_AZALEA_LED2           (GPIO2) /* PORT for LED1, 3, 4 */

uint32_t boot0, boot1, boot2, boot3;

int main(void)
{
	int i;
	pin_setup();

	/* Configure SCU Pin Mux as GPIO */
	scu_pinmux(SCU_PINMUX_AZALEA_LED1, SCU_GPIO_NOPULL);
	scu_pinmux(SCU_PINMUX_AZALEA_LED2, SCU_GPIO_NOPULL);
	scu_pinmux(SCU_PINMUX_AZALEA_LED3, SCU_GPIO_NOPULL);
	scu_pinmux(SCU_PINMUX_AZALEA_LED4, SCU_GPIO_NOPULL);

	/* Configure GPIO2[1/2/8] (P4_1/2 P6_12) as output. */
	GPIO2_DIR |= PIN_AZALEA_LED2;
	GPIO3_DIR |= (PIN_AZALEA_LED1 | PIN_AZALEA_LED3 | PIN_AZALEA_LED4);

	/* enable all power supplies */
	enable_1v8_power();

	/* Blink LED1/2/3 on the board and Read BOOT0/1/2/3 pins. */
	while (1) 
	{
		boot0 = BOOT0_STATE;
		boot1 = BOOT1_STATE;
		boot2 = BOOT2_STATE;
		boot3 = BOOT3_STATE;

		gpio_set(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED4); /* LED4 off */
		gpio_clear(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED1); /* LED1 on */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");

		gpio_set(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED1); /* LED1 off */
		gpio_clear(PORT_AZALEA_LED2, (PIN_AZALEA_LED2)); /* LED2 on */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");

		gpio_set(PORT_AZALEA_LED2, (PIN_AZALEA_LED2)); /* LED2 off */
		gpio_clear(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED3); /* LED3 on */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");

		gpio_set(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED3); /* LED3 off */
		gpio_clear(PORT_AZALEA_LED1_3_4, PIN_AZALEA_LED4); /* LED4 on */
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");
	}

	return 0;
}
