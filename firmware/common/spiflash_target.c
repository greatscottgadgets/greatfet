/*
 * This file is part of GreatFET
 */

#include "spiflash.h"

#include <libopencm3/lpc43xx/scu.h>
#include "pins.h"

/* TODO: Why is SSEL being controlled manually when SSP0 could do it
 * automatically?
 */

void spiflash_target_init(spi_target_t* const target) {
	/* Init SPIFI GPIO to Normal GPIO */
	scu_pinmux(P3_3, (SCU_SSP_IO | SCU_CONF_FUNCTION2));    // P3_3 SPIFI_SCK => SSP0_SCK
	scu_pinmux(P3_4, (SCU_GPIO_FAST | SCU_CONF_FUNCTION0)); // P3_4 SPIFI SPIFI_SIO3 IO3 => GPIO1[14]
	scu_pinmux(P3_5, (SCU_GPIO_FAST | SCU_CONF_FUNCTION0)); // P3_5 SPIFI SPIFI_SIO2 IO2 => GPIO1[15]
	scu_pinmux(P3_6, (SCU_GPIO_FAST | SCU_CONF_FUNCTION0)); // P3_6 SPIFI SPIFI_MISO IO1 => GPIO0[6]
	scu_pinmux(P3_7, (SCU_GPIO_FAST | SCU_CONF_FUNCTION4)); // P3_7 SPIFI SPIFI_MOSI IO0 => GPIO5[10]
	scu_pinmux(P3_8, (SCU_GPIO_FAST | SCU_CONF_FUNCTION4)); // P3_8 SPIFI SPIFI_CS => GPIO5[11]
	
	/* configure SSP pins */
	scu_pinmux(SCU_SSP0_MISO, (SCU_SSP_IO | SCU_CONF_FUNCTION5));
	scu_pinmux(SCU_SSP0_MOSI, (SCU_SSP_IO | SCU_CONF_FUNCTION5));
	scu_pinmux(SCU_SSP0_SCK,  (SCU_SSP_IO | SCU_CONF_FUNCTION2));

	/* configure GPIO pins */
	scu_pinmux(SCU_FLASH_HOLD, SCU_GPIO_FAST);
	scu_pinmux(SCU_FLASH_WP, SCU_GPIO_FAST);
	scu_pinmux(SCU_SSP0_SSEL, (SCU_GPIO_FAST | SCU_CONF_FUNCTION4));

	/* drive SSEL, HOLD, and WP pins high */
	gpio_set(target->gpio_hold);
	gpio_set(target->gpio_wp);

	/* Set GPIO pins as outputs. */
	gpio_output(target->gpio_hold);
	gpio_output(target->gpio_wp);
}
