/*
 * This file is part of GreatFET
 */

#include "swra124.h"

#include <gpio.h>
#include <gpio_lpc.h>
#include <greatfet_core.h>
#include <libopencm3/lpc43xx/scu.h>
#include <pins.h>

static struct gpio_t swra124_reset = GPIO(0, 10);	// GF1 Pin: J1.40
static struct gpio_t swra124_clock = GPIO(0, 11);	// GF1 Pin: J1.39
static struct gpio_t swra124_data = GPIO(0, 15);	// GF1 Pin: J1.37

void swra124_setup()
{
	scu_pinmux(SCU_PINMUX_GPIO0_10, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO0_11, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO0_15, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);

	gpio_write(&swra124_reset, 1);
	gpio_write(&swra124_clock, 0);
	gpio_write(&swra124_data, 0);

	gpio_output(&swra124_reset);
	gpio_output(&swra124_clock);
	gpio_input(&swra124_data);
}

void swra124_debug_init()
{
	struct {
		struct gpio_t *gpio;
		uint8_t value;
	} steps[] = {
		{.gpio = &swra124_reset, .value = 0},
		{.gpio = &swra124_clock, .value = 1},
		{.gpio = &swra124_clock, .value = 0},
		{.gpio = &swra124_clock, .value = 1},
		{.gpio = &swra124_clock, .value = 0},
		{.gpio = &swra124_reset, .value = 1},
		{.gpio = NULL},
	};
	for (int i = 0; steps[i].gpio; i++) {
		gpio_write(steps[i].gpio, steps[i].value);
		delay_us(1);
	}
}

void swra124_write_byte(const uint8_t v)
{
	gpio_output(&swra124_data);
	for (int i = 0; i < 8; i++) {
		gpio_write(&swra124_data, (v >> (7 - i)) & 0x01);
		delay_us(1);
		gpio_write(&swra124_clock, 1);
		delay_us(1);
		gpio_write(&swra124_clock, 0);
		delay_us(1);
	}
}

void swra124_write(const uint8_t *data, const size_t size)
{
	for (size_t i = 0; i < size; i++) {
		swra124_write_byte(data[i]);
	}
}

uint8_t swra124_read()
{
	uint8_t result = 0;
	gpio_input(&swra124_data);
	for (int i = 0; i < 8; i++) {
		gpio_write(&swra124_clock, 1);
		delay_us(1);
		result = (result << 1) | gpio_read(&swra124_data);
		delay_us(1);
		gpio_write(&swra124_clock, 0);
		delay_us(1);
	}
	return result;
}

void swra124_chip_erase()
{
	uint8_t command[] = {0x14};
	swra124_write(command, 1);
	swra124_read();
}

void swra124_write_config(const uint8_t config)
{
	uint8_t command[] = {0x1d, config};
	swra124_write(command, 2);
	swra124_read();
}

uint8_t swra124_read_status()
{
	uint8_t command[] = {0x34};
	swra124_write(command, 1);
	return swra124_read();
}

uint16_t swra124_get_chip_id()
{
	uint8_t command[] = {0x68};
	swra124_write(command, 1);
	return (swra124_read() << 8) | swra124_read();
}

void swra124_halt()
{
	uint8_t command[] = {0x44};
	swra124_write(command, 1);
	swra124_read();
}

void swra124_resume()
{
	uint8_t command[] = {0x4c};
	swra124_write(command, 1);
	swra124_read();
}

uint8_t swra124_debug_instr(const uint8_t *instr, const size_t size)
{
	uint8_t command[] = {0x54 | (size & 0x03)};
	swra124_write(command, 1);
	swra124_write(instr, size);
	return swra124_read();
}

void swra124_step_instr()
{
	uint8_t command[] = {0x5c};
	swra124_write(command, 1);
	swra124_read();
}

uint16_t swra124_get_pc()
{
	uint8_t command[] = {0x28};
	swra124_write(command, 1);
	return (swra124_read() << 8) | swra124_read();
}
