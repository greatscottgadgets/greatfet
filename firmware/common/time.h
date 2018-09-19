/*
 * This file is part of GreatFET
 *
 * Time of day and wall clock tracking functions.
 */

#ifndef __GREATFET_TIME_H__
#define __GREATFET_TIME_H__

#ifdef __RUNNING_ON_HOST__
    #include_next "time.h"
#endif

/**
 * Initialization function for the GreatFET microsecond timer, which is used
 * to track total seconds while the GreatFET is running.
 *
 * @param The frequency, in MHz, that the main M4 clock is running at.
 */
void set_up_microsecond_timer(uint32_t m4_clk_mhz);

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
