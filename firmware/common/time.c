/*
 * This file is part of GreatFET
 *
 * Time of day and wall clock tracking functions.
 */


#include <greatfet_core.h>
#include <libopencm3/lpc43xx/timer.h>

/**
 * Initialization function for the GreatFET microsecond timer, which is used
 * to track total seconds while the GreatFET is running.
 *
 * Currently must be called while on the faster clock speed.
 */
void set_up_microsecond_timer(uint32_t m4_clk_mhz)
{
    // Set up TIMER3 to count microseconds.
    timer_set_prescaler(TIMER3, m4_clk_mhz - 1);
    timer_enable_counter(TIMER3);
}

/**
 * @returns the total number of microseconds since this timer was initialized.
 * Overflows roughly once per hour. For tracking longer spans; use the RTC
 * functions, which are currently not synchronized to this one.
 */
uint32_t get_time(void)
{
	return TIMER3_TC;
}


/**
 * @returns The total number of microseconds that have passed since a reference call to get_time().
 *		Useful for computing timeouts.
 */
uint32_t get_time_since(uint32_t base)
{
	return get_time() - base;
}
