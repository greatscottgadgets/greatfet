/*
 * This file is part of GreatFET
 *
 * Time of day and wall clock tracking functions.
 */

#ifndef __GREATFET_TIME_H__
#define __GREATFET_TIME_H__

#include <greatfet_core.h>

#ifdef __RUNNING_ON_HOST__
    #include_next "time.h"
#else
    #include <libopencm3/lpc43xx/timer.h>
#endif

/**
 * Initialization function for the GreatFET microsecond timer, which is used
 * to track total seconds while the GreatFET is running.
 *
 * Currently must be called while on the faster clock speed.
 */
void set_up_microsecond_timer(void);

/**
 * @returns the total number of microseconds since this timer was initialized.
 * Overflows roughly once per hour. For tracking longer spans; use the RTC
 * functions, which are currently not synchronized to this one.
 */
uint32_t get_time(void);


/**
 * @returns The total number of microseconds that have passed since a reference call to get_time().
 *		Useful for computing timeouts.
 */
uint32_t get_time_since(uint32_t base);

#endif
