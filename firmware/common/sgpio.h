/*
 * This file is part of GreatFET
 */

#ifndef __SGPIO_H__
#define __SGPIO_H__

#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/lpc43xx/sgpio.h>

#include "gpio.h"

typedef enum {
	SGPIO_DIRECTION_INPUT,
	SGPIO_DIRECTION_OUTPUT,
} sgpio_direction_t;

typedef struct sgpio_config_t {
	bool slice_mode_multislice;
	uint16_t clock_divider;
} sgpio_config_t;

void sgpio_configure_pin_functions(const sgpio_config_t* const config);
void sgpio_set_slice_mode(
	sgpio_config_t* const config,
	const bool multi_slice
);
void sgpio_configure(
	const sgpio_config_t* const config,
	const sgpio_direction_t direction
);
void sgpio_clock_out_configure(uint16_t clock_divider);

#endif//__SGPIO_H__
