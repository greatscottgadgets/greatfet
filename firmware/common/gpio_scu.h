/*
 * This file is part of GreatFET
 */

#ifndef __GPIO_SCU_H__
#define __GPIO_SCU_H__

#include <libopencm3/lpc43xx/scu.h>

#define GPIO_MAX_PORTS 6
#define GPIO_MAX_PORT_BITS 20

scu_grp_pin_t get_scu_pin_for_gpio(uint8_t gpio_port, uint8_t gpio_pin);
uint32_t get_scu_func_for_gpio(uint8_t gpio_port, uint8_t gpio_pin);

#endif /*__GPIO_SCU_H__*/
