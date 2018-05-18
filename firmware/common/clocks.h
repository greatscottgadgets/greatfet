/*
 * This file is part of GreatFET
 */

#ifndef __CLOCKS_H__
#define __CLOCKS_H__

#include <stdint.h>

void cpu_clock_init(void);
void cpu_clock_pll1_low_speed(void);
void cpu_clock_pll1_max_speed(void);

void rtc_init(void);

/* msel is the frequency in multiples of 100 kHz */
uint8_t pll0audio_config(uint16_t msel);
void pll0audio_on(void);
void pll0audio_off(void);
uint8_t pll0audio_tune(uint16_t msel);

#endif/* __CLOCKS_H__ */