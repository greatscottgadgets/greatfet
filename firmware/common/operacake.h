/*
 * This file is part of GreatFET
 */

#ifndef __OPERACAKE_H
#define __OPERACAKE_H

#include <stdint.h>
#include "i2c_bus.h"
#include "gpio_lpc.h"

#define OPERACAKE_PA1 0
#define OPERACAKE_PA2 1
#define OPERACAKE_PA3 2
#define OPERACAKE_PA4 3

#define OPERACAKE_PB1 4
#define OPERACAKE_PB2 5
#define OPERACAKE_PB3 6
#define OPERACAKE_PB4 7

#define MAX_OPERACAKE_RANGES 8

/* Up to 8 Operacake boards can be used with one HackRF */
extern uint8_t operacake_boards[8];

uint8_t operacake_init(void);
void operacake_gpio(void);
uint8_t operacake_set_ports(uint8_t address, uint8_t PA, uint8_t PB);
uint8_t operacake_add_range(uint16_t freq_min, uint16_t freq_max, uint8_t port);
uint8_t operacake_set_range(uint32_t freq_mhz);

#endif /* __OPERACAKE_H */
