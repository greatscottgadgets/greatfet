/*
 * This file is part of GreatFET
 */

#ifndef __CLOCKS_H__
#define __CLOCKS_H__

#include <stdint.h>

void cpu_clock_init(void);
uint8_t pll0audio_init(void);
void cpu_clock_pll1_low_speed(void);
void cpu_clock_pll1_max_speed(void);

void rtc_init(void);

#endif/* __CLOCKS_H__ */