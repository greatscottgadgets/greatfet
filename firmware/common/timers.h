/*
 * This file is part of GreatFET
 */

#ifndef LPC43XX_GF_TIMERS_H
#define LPC43XX_GF_TIMERS_H

#include <libopencm3/cm3/common.h>
#include <libopencm3/lpc43xx/memorymap.h>


#define ALARM_TIMER_BASE      0x40040000
#define ALARM_TIMER_DOWNCOUNT MMIO32(ALARM_TIMER_BASE + 0)
#define ALARM_TIMER_PRESET    MMIO32(ALARM_TIMER_BASE + 4)

#endif
