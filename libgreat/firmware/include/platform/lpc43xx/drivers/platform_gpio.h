/*
 * This file is part of libgreat
 *
 * LPC43xx GPIO functions
 */


#ifndef __LIBGREAT_PLATFORM_GPIO_H__
#define __LIBGREAT_PLATFORM_GPIO_H__

// Describe the chip's GPIO capabilities.
#define GPIO_MAX_PORTS 6
#define GPIO_MAX_PORT_BITS 20

/**
 * Returns the SCU group number for the given GPIO bit.
 */
uint8_t gpio_get_group_number(uint8_t port, uint8_t pin);

/**
 * Returns the SCU pin number for the given GPIO bit.
 */
uint8_t gpio_get_pin_number(uint8_t port, uint8_t pin);

#endif // __LIBGREAT_GPIO_H__
