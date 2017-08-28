/*
 * This file is part of GreatFET
 */

#include "gpio_scu.h"
#include <libopencm3/lpc43xx/scu.h>


/*
 gpio_scu

 Functions to map GPIO (port, pin) to SCU pinmux values.  These allow
 a pin to be configured for GPIO mode if only the GPIO (port, pin) are known.

 XXX Only GPIO lines used by the GreatFET headers are supported.
*/


/* Table used internally by the functions below it */
static const uint32_t gpio_to_scu_pin[GPIO_MAX_PORTS][GPIO_MAX_PORT_BITS] = {
  /* GPIO0 */
  {
  P0_0,   /* GPIO0[0]   J1_4  */
  P0_1,   /* GPIO0[1]   J1_6  */
  P1_15,  /* GPIO0[2]   J1_28 */
  P1_16,  /* GPIO0[3]   J1_30 */
  P1_0,   /* GPIO0[4]   J1_7  */
  P6_6,   /* GPIO0[5]   J2_34 */
  P3_6,   /* GPIO0[6]   J2_38 */
  P2_7,   /* GPIO0[7]   J2_14 */
  P1_1,   /* GPIO0[8]   J1_10 */
  P1_2,   /* GPIO0[9]   J1_12 */
  P1_3,   /* GPIO0[10]  J1_40 */
  P1_4,   /* GPIO0[11]  J1_39 */
  P1_17,  /* GPIO0[12]  J1_32 */
  P1_18,  /* GPIO0[13]  J1_31 */
  P2_10,  /* GPIO0[14]  J7_14 */
  P1_20,  /* GPIO0[15]  J1_37 */
  0,      /* GPIO0[16]        */
  0,      /* GPIO0[17]        */
  0,      /* GPIO0[18]        */
  0,      /* GPIO0[19]        */
  },
  /* GPIO1 */
  {
  P1_7,   /* GPIO1[0]   J1_15 */
  P1_8,   /* GPIO1[1]   J1_18 */
  P1_9,   /* GPIO1[2]   J1_17 */
  P1_10,  /* GPIO1[3]   J1_20 */
  P1_11,  /* GPIO1[4]   J1_22 */
  P1_12,  /* GPIO1[5]   J1_21 */
  P1_13,  /* GPIO1[6]   J1_26 */
  P1_14,  /* GPIO1[7]   J1_25 */
  P1_5,   /* GPIO1[8]   J1_13 */
  P1_6,   /* GPIO1[9]   J1_16 */
  P2_9,   /* GPIO1[10]  J7_6  */
  P2_11,  /* GPIO1[11]  J7_13 */
  P2_12,  /* GPIO1[12]  J7_7  */
  P2_13,  /* GPIO1[13]  J7_8  */
  P3_4,   /* GPIO1[14]  J2_28 */
  P3_5,   /* GPIO1[15]  J2_37 */
  0,      /* GPIO1[16]        */
  0,      /* GPIO1[17]        */
  0,      /* GPIO1[18]        */
  0,      /* GPIO1[19]        */
  },
  /* GPIO2 */
  {
  P4_0,   /* GPIO2[0]   J2_4  */
  0,      /* GPIO2[1]         */
  P4_2,   /* GPIO2[2]   J2_8  */
  P4_3,   /* GPIO2[3]   J2_9  */
  P4_4,   /* GPIO2[4]   J2_7  */
  P4_5,   /* GPIO2[5]   J2_6  */
  P4_6,   /* GPIO2[6]   J2_10 */
  P5_7,   /* GPIO2[7]   J1_29 */
  0,      /* GPIO2[8]         */
  P5_0,   /* GPIO2[9]   J1_8  */
  P5_1,   /* GPIO2[10]  J1_9  */
  P5_2,   /* GPIO2[11]  J1_14 */
  P5_3,   /* GPIO2[12]  J1_19 */
  P5_4,   /* GPIO2[13]  J1_24 */
  P5_5,   /* GPIO2[14]  J1_23 */
  P5_6,   /* GPIO2[15]  J1_27 */
  0,      /* GPIO2[16]        */
  0,      /* GPIO2[17]        */
  0,      /* GPIO2[18]        */
  0,      /* GPIO2[19]        */
  },
  /* GPIO3 */
  {
  P6_1,   /* GPIO3[0]   J7_18 */
  P6_2,   /* GPIO3[1]   J7_17 */
  P6_3,   /* GPIO3[2]   J2_36 */
  P6_4,   /* GPIO3[3]   J7_2  */
  P6_5,   /* GPIO3[4]   J7_3  */
  P6_9,   /* GPIO3[5]   J7_16 */
  P6_10,  /* GPIO3[6]   J7_15 */
  0,      /* GPIO3[7]         */
  P7_0,   /* GPIO3[8]   J2_27 */
  P7_1,   /* GPIO3[9]   J2_25 */
  P7_2,   /* GPIO3[10]  J2_23 */
  0,      /* GPIO3[11]        */
  0,      /* GPIO3[12]        */
  0,      /* GPIO3[13]        */
  0,      /* GPIO3[14]        */
  P7_7,   /* GPIO3[15]  J2_16 */
  0,      /* GPIO3[16]        */
  0,      /* GPIO3[17]        */
  0,      /* GPIO3[18]        */
  0,      /* GPIO3[19]        */
  },
  /* GPIO4 */
  {
  0,      /* GPIO4[0]         */
  0,      /* GPIO4[1]         */
  0,      /* GPIO4[2]         */
  0,      /* GPIO4[3]         */
  0,      /* GPIO4[4]         */
  0,      /* GPIO4[5]         */
  0,      /* GPIO4[6]         */
  0,      /* GPIO4[7]         */
  0,      /* GPIO4[8]         */
  0,      /* GPIO4[9]         */
  0,      /* GPIO4[10]        */
  P9_6,   /* GPIO4[11]  J1_34 */
  0,      /* GPIO4[12]        */
  0,      /* GPIO4[13]        */
  0,      /* GPIO4[14]        */
  0,      /* GPIO4[15]        */
  0,      /* GPIO4[16]        */
  0,      /* GPIO4[17]        */
  0,      /* GPIO4[18]        */
  0,      /* GPIO4[19]        */
  },
  /* GPIO5 */
  {
  P2_0,   /* GPIO5[0]   J1_35 */
  P2_1,   /* GPIO5[1]   J2_35 */
  P2_2,   /* GPIO5[2]   J2_33 */
  P2_3,   /* GPIO5[3]   J2_20 */
  P2_4,   /* GPIO5[4]   J2_19 */
  P2_5,   /* GPIO5[5]   J2_18 */
  P2_6,   /* GPIO5[6]   J2_15 */
  P2_8,   /* GPIO5[7]   J2_13 */
  P3_1,   /* GPIO5[8]   J2_24 */
  P3_2,   /* GPIO5[9]   J2_22 */
  P3_7,   /* GPIO5[10]  J2_30 */
  0,      /* GPIO5[11]        */
  P4_8,   /* GPIO5[12]  J2_3  */
  P4_9,   /* GPIO5[13]  J1_3  */
  P4_10,  /* GPIO5[14]  J1_5  */
  P6_7,   /* GPIO5[15]  J2_31 */
  P6_8,   /* GPIO5[16]  J2_29 */
  0,      /* GPIO5[17]        */
  P9_5,   /* GPIO5[18]  J1_33 */
  0,      /* GPIO5[19]        */
  },
};


/*
 Get the SCU pin constant that corresponds to a GPIO port and pin.
 Example: GPIO5[7] -> P2_8
 */
scu_grp_pin_t get_scu_pin_for_gpio(uint8_t gpio_port, uint8_t gpio_pin)
{
	scu_grp_pin_t scu_pin = 0;
	if ((gpio_port < GPIO_MAX_PORTS) && (gpio_pin < GPIO_MAX_PORT_BITS)) {
		scu_pin = gpio_to_scu_pin[gpio_port][gpio_pin];
	}
	return scu_pin;
}


/*
 Get the SCU function required to set GPIO mode for a GPIO port and pin.
 Example: GPIO5[7] -> SCU_CONF_FUNCTION4
 */
uint32_t get_scu_func_for_gpio(uint8_t gpio_port, uint8_t gpio_pin)
{
	/* For consistency; pin should be needed but isn't due to the
	   optimization below. */
	(void)gpio_pin;

	/* Optimization: this was originally a lookup table but it was observed
	   that only GPIO port 5 required a different function number. */
	return (gpio_port == 5) ? SCU_CONF_FUNCTION4 : SCU_CONF_FUNCTION0;
}
