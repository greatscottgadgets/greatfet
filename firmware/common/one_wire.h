/*
 * This file is part of GreatFET
 */

#ifndef __ONE_WIRE_H__
#define __ONE_WIRE_H__

#include <stdint.h>

void one_wire_init(void);
void one_wire_delay_us(uint32_t us);

uint8_t one_wire_init_target(void);

uint8_t one_wire_read(void);

void one_wire_write(uint8_t byte);

#endif/* __ONE_WIRE_H__ */
