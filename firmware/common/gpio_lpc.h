/*
 * This file is part of GreatFET
 */

#ifndef __GPIO_LPC_H__
#define __GPIO_LPC_H__

#include <stdint.h>

#include "gpio.h"

/* NOTE: libopencm3 constants and functions not used here due to naming
 * conflicts. I'd recommend changes to libopencm3 design to separate
 * register #defines and API declarations into separate header files.
 */

typedef struct gpio_port_t {
	volatile uint32_t dir;		/* +0x000 */
	uint32_t _reserved0[31];
	volatile uint32_t mask;		/* +0x080 */
	uint32_t _reserved1[31];
	volatile uint32_t pin;		/* +0x100 */
	uint32_t _reserved2[31];
	volatile uint32_t mpin;		/* +0x180 */
	uint32_t _reserved3[31];
	volatile uint32_t set;		/* +0x200 */
	uint32_t _reserved4[31];
	volatile uint32_t clr;		/* +0x280 */
	uint32_t _reserved5[31];
	volatile uint32_t not;		/* +0x300 */
} gpio_port_t;

/* Removing the "const" here is a horrible thing to do
 * I need to find a better solution to this
 * For the reason, see usb_api_spiflash.c
 */
struct gpio_t {
	/*const*/ uint32_t mask;
	gpio_port_t* /*const*/ port;
	volatile uint32_t* /*const*/ gpio_w;
};

#define GPIO_LPC_BASE (0x400f4000)
#define GPIO_LPC_B_OFFSET (0x0)
#define GPIO_LPC_W_OFFSET (0x1000)
#define GPIO_LPC_PORT_OFFSET (0x2000)

#define GPIO_LPC_PORT(_n) ((gpio_port_t*)((GPIO_LPC_BASE + GPIO_LPC_PORT_OFFSET) + (_n) * 4))
#define GPIO_LPC_W(_port_num, _pin_num) (volatile uint32_t*)((GPIO_LPC_BASE + GPIO_LPC_W_OFFSET) + ((_port_num) * 0x80) + ((_pin_num) * 4))

#define GPIO(_port_num, _pin_num) { \
	.mask = (1UL << (_pin_num)), \
	.port = GPIO_LPC_PORT(_port_num), \
	.gpio_w = GPIO_LPC_W(_port_num, _pin_num), \
}

#define GPIO_SET(_gpio, _port_num, _pin_num) \
	_gpio.mask = (1UL << (_pin_num)); \
	_gpio.port = GPIO_LPC_PORT(_port_num); \
	_gpio.gpio_w = GPIO_LPC_W(_port_num, _pin_num);

#endif/*__GPIO_LPC_H__*/
