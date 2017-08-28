/*
 * This file is part of GreatFET
 */

#include "greatfet_core.h"

int main(void)
{
	cpu_clock_init();
	cpu_clock_pll1_max_speed();
	rtc_init();
	pin_setup();

	/* Blink LED1/2/3/4 on the board. */
	while (1)
	{
		led_on(LED1);
		led_off(LED2);
		led_on(LED3);
		led_off(LED4);
		delay(5000000);

		led_off(LED1);
		led_on(LED2);
		led_off(LED3);
		led_on(LED4);
		delay(5000000);
	}

	return 0;
}
