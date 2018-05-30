/*
 * This file is part of GreatFET
 */

#include <gpio.h>
#include <gpio_lpc.h>
#include <greatfet_core.h>
#include "one_wire.h"

static struct gpio_t dq = GPIO(5, 8);

/* In the future we'll use a timer for the delay
 * but for now a delay of 100 us is delay(3000)
 */
uint8_t one_wire_init_target(void)
{
	gpio_output(&dq);
	// pull low
	gpio_write(&dq, 0);
	// wait 500 us
	delay(15000);
	// switch to input
	gpio_input(&dq);
	// wait ~60 us
	delay(1800);
	// read pin
	// if low, device is present
	if(!gpio_read(&dq)) {
		// wait 500 us
		delay(15000);
		return 0;
	}
	// wait 500 us
	delay(15000);
	return 1;
}

uint8_t one_wire_read_bit(void) {
	gpio_output(&dq);
	// pull low
	gpio_write(&dq, 0);
	// wait 5 us
	delay(150);
	// switch to input
	gpio_input(&dq);
	// wait 5 us
	delay(150);
	if(gpio_read(&dq)) {
		// wait 60 us
		delay(3000);
		return 1;
	}
	// wait 60 us
	delay(3000);
	return 0;
}

uint8_t one_wire_read(void)
{
	uint8_t i, byte = 0x00;
	for(i=0; i<8; i++) {
		// byte <<= 1;
		byte |= (one_wire_read_bit() & 0x01) << i;
	}
	// Reset state to idle
	gpio_output(&dq);
	gpio_write(&dq, 1);
	return byte;
}

void one_wire_write_bit(uint8_t bit)
{
	gpio_output(&dq);
	// pull low
	gpio_write(&dq, 0);
	if(bit) {
		// wait 5 us
		delay(150);
	} else {
		// wait >60 us
		delay(3000);
	}
	gpio_write(&dq, 1);
	delay(1800);
}

void one_wire_write(uint8_t byte)
{
	int i;
	for(i=0; i<8; i++) {
		one_wire_write_bit(byte & 0x01);
		byte >>= 1;
	}
	// Reset state to idle
	gpio_write(&dq, 1);
}