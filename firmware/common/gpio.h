/*
 * This file is part of GreatFET
 */

#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdbool.h>
#include <stdint.h>

typedef const struct gpio_t* gpio_t;

void gpio_init();
void gpio_set(gpio_t gpio);
void gpio_write_multiple(gpio_t gpio, uint32_t value);
void gpio_clear(gpio_t gpio);
void gpio_toggle(gpio_t gpio);
void gpio_output(gpio_t gpio);
void gpio_input(gpio_t gpio);
void gpio_write(gpio_t gpio, const bool value);
bool gpio_read(gpio_t gpio);

#endif/*__GPIO_H__*/
