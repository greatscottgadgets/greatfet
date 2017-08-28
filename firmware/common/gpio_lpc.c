/*
 * This file is part of GreatFET
 */

#include "gpio_lpc.h"

#include <stddef.h>

void gpio_init() {
	for(size_t i=0; i<8; i++) {
		GPIO_LPC_PORT(i)->dir = 0;
	}
}

void gpio_set(gpio_t gpio) {
	gpio->port->set = gpio->mask;
}

void gpio_write_multiple(gpio_t gpio, uint32_t value) {
	gpio->port->pin = value;
}

void gpio_clear(gpio_t gpio) {
	gpio->port->clr = gpio->mask;
}

void gpio_toggle(gpio_t gpio) {
	gpio->port->not = gpio->mask;
}

void gpio_output(gpio_t gpio) {
	gpio->port->dir |= gpio->mask;
}

void gpio_input(gpio_t gpio) {
	gpio->port->dir &= ~gpio->mask;
}

void gpio_write(gpio_t gpio, const bool value) {
	*gpio->gpio_w = value;
}

bool gpio_read(gpio_t gpio) {
	return *gpio->gpio_w;
}
