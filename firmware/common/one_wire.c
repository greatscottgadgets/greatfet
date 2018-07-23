/*
 * This file is part of GreatFET
 */

#include <gpio.h>
#include <gpio_lpc.h>
#include <greatfet_core.h>
#include <libopencm3/lpc43xx/timer.h>
#include "one_wire.h"

static struct gpio_t dq = GPIO(5, 8);

void one_wire_init(void) {
    // timer3 period -- 1 microsecond
    timer_set_prescaler(TIMER3, 203);
    timer_enable_counter(TIMER3);

    gpio_output(&dq);
    gpio_write(&dq, 1);
}

void one_wire_delay_us(uint32_t us) {
    uint32_t start = TIMER3_TC;
    while (TIMER3_TC - start < us)
        ;
}

uint8_t one_wire_init_target(void)
{
    uint8_t device_present = 0;

    gpio_output(&dq);
    // pull low
    gpio_write(&dq, 0);
    // wait 500 us
    one_wire_delay_us(500);
    // switch to input
    gpio_write(&dq, 1);
    gpio_input(&dq);
    // wait ~60 us
    one_wire_delay_us(80);
    // read pin
    // if low, device is present
    device_present = gpio_read(&dq) == 0;
    one_wire_delay_us(420);
    return device_present;
}

uint8_t one_wire_read_bit(void) {
    uint8_t value = 0;

    gpio_output(&dq);
    // pull low
    gpio_write(&dq, 0);
    // wait 5 us
    one_wire_delay_us(5);
    // switch to input
    gpio_input(&dq);
    // wait 5 us
    one_wire_delay_us(10);
    value = gpio_read(&dq);
    one_wire_delay_us(45);
    return value;
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
    gpio_input(&dq);
    return byte;
}

void one_wire_write_bit(uint8_t bit)
{
    gpio_output(&dq);
    // pull low
    gpio_write(&dq, 0);
    if(bit) {
        // wait 5 us
        one_wire_delay_us(5);
        gpio_write(&dq, 1);
        one_wire_delay_us(55);
    } else {
        // wait >60 us
        one_wire_delay_us(30);
        gpio_write(&dq, 1);
        one_wire_delay_us(30);
    }
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
    gpio_input(&dq);
}
