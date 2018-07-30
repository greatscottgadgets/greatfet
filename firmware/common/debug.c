/*
 * This file is part of GreatFET
 */

#include <stdio.h>

#include <pins.h>
#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/uart.h>
#include <libopencm3/cm3/memorymap.h>
#include <libopencm3/cm3/scs.h>
#include "debug.h"

#define SEMIHOST_SWI		"0xAB"
#define SEMIHOST_STDOUT_FILENO "2"
#define SEMIHOST_SYS_WRITEC	"0x3"
#define SEMIHOST_SYS_WRITE0	"0x4"
#define SEMIHOST_SYS_WRITE	"0x5"


// Store the active loglevel.
static loglevel_t debug_loglevel = LOGLEVEL_ERROR;

/**
 * Initializes debugging support.
 */
void debug_init(void)
{
	// Nothing to do here, currently.
}

/**
 * Sets the system's active debug level. Any debug print with a level _higher_
 * than this will not be printed.
 *
 * @param loglevel The log level to apply.
 */
void debug_set_loglevel(loglevel_t loglevel)
{
	debug_loglevel = loglevel;
}


/**
 * @return true iff there is currently a debugger connected.
 */
bool debugger_is_connected(void)
{
	// Return true iff we have a debugger connected.
	return (SCS_DHCSR & SCS_DHCSR_C_DEBUGEN);
}


/**
 * Prints a single character via semihosting.
 * Will hard fault if no debugger is connected.
 *
 * @param c The character to print.
 */
static void semihosting_putc(char c)
{
	// Operation #3, SYS_WRITEC; r0 = operation number, r1 = pointer to character to write
	asm volatile(
		 "mov  r1, %0\n"
		 "mov  r0, #" SEMIHOST_SYS_WRITEC "\n"
		 "bkpt  " SEMIHOST_SWI "\n"
		 : : "r" (&c) : "r0", "r1", "memory");
}



/**
 * Prints a string via semihosting.
 * Will hard fault if no debugger is connected.
 *
 * @param s The string to print.
 */
static void semihosting_puts(char volatile *s)
{
	uint32_t length = 0;
	char volatile *i = s;

	// semihosting arguments
	volatile uint32_t parameters[3];

	// Count the length of the string, to avoid using SEMIHOST_WRITE0.
	while(*i) {
		++length;
		++i;
	}

	// Format per the semihosting spec.
	parameters[0] = 3;
	parameters[1] = (uint32_t)s;
	parameters[2] = length;

	// Operation #3, SYS_WRITE; r0 = operation number, r1 = pointer to string to print
	// Note that we avoid SYS_WRITE0, as some debuggers seem to not include it.
	asm volatile(
		 "mov  r1, %0\n"
		 "mov  r0, #" SEMIHOST_SYS_WRITE "\n"
		 "bkpt  " SEMIHOST_SWI "\n"
		 : : "r" (parameters) : "r0", "r1", "memory");
}

/**
 * Prints a string to the debug console. Ignores loglevel.
 *
 * @param str The string to print.
 */
void debug_puts(char *str)
{
	// TODO: log to ringbuffer

	if (debugger_is_connected())
		semihosting_puts(str);

}

/**
 * Prints a character to the debug console. Ignores loglevel.
 *
 * @param c The char to print.
 */
void debug_putc(char c)
{
	// TODO: log to ringbuffer

	if (debugger_is_connected())
		semihosting_putc(c);
}

/**
 * Core debugging print for GreatFET printing.
 *
 * @param loglevel The log level at which the given string should be printed.
 * @param fmt Format string; matches printf.
 */
void vprintk(int loglevel, char *fmt, va_list list)
{
	// If this statement is above the maximum level to be displayed,
	// bail out.
	if (loglevel > debug_loglevel)
		return;

    vprintf(fmt, list);
}

/**
 * Core debugging print for GreatFET printing.
 *
 * @param loglevel The log level at which the given string should be printed.
 * @param fmt Format string; matches printf.
 */
void printk(int loglevel, char *fmt, ...)
{
    va_list list;

    va_start(list, fmt);
    vprintk(loglevel, fmt, list);
    va_end(list);
}


/**
 * Convenience function that prints errors using the ERROR loglevel.
 */
void pr_emergency(char *fmt, ...)
{
    va_list list;

    va_start(list, fmt);
    vprintk(LOGLEVEL_EMERGENCY, fmt, list);
    va_end(list);
}


/**
 * Convenience function that prints errors using the ERROR loglevel.
 */
void pr_error(char *fmt, ...)
{
    va_list list;

    va_start(list, fmt);
    vprintk(LOGLEVEL_ERROR, fmt, list);
    va_end(list);
}


/**
 * Convenience function that prints errors using the WARNING loglevel.
 */
void pr_warning(char *fmt, ...)
{
    va_list list;

    va_start(list, fmt);
    vprintk(LOGLEVEL_WARNING, fmt, list);
    va_end(list);
}


/**
 * Convenience function that prints errors using the DEBUG loglevel.
 */
void pr_debug(char *fmt, ...)
{
    va_list list;

    va_start(list, fmt);
    vprintk(LOGLEVEL_DEBUG, fmt, list);
    va_end(list);
}
