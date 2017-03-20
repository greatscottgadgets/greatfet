/*
 * Copyright 2010 - 2012 Michael Ossmann
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

#include "greatfet_core.h"

extern void blinky_ratchet(void (*f1)(), void (*f2)(), void (*f3)(), void (*f4)());

void led0_on() {
    led_on(LED1);
}

void led0_off() {
    led_off(LED1);
}

void led1_on() {
    led_on(LED2);
}

void led1_off() {
    led_off(LED2);
}


int main(void)
{
	int i;

	pin_setup();
    led0_off();

	/* Blink LED1/2/3 on the board. */
	while (1)
	{

        blinky_ratchet(&led0_on,
                       &led0_off,
                       &led1_on,
                       &led1_off
                       );
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");
	}

	return 0;
}
