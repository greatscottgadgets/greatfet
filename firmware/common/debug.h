/*
 * This file is part of GreatFET
 */

 #ifndef __DEBUG_H__
 #define __DEBUG_H__

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

// Possible log levels.
enum debug_log_level {
	LOGLEVEL_EMERGENCY,
	LOGLEVEL_ALERT,
	LOGLEVEL_CRITITCAL,
	LOGLEVEL_ERROR,
	LOGLEVEL_WARNING,
	LOGLEVEL_NOTICE,
	LOGLEVEL_INFO,
	LOGLEVEL_DEBUG
};
typedef enum debug_log_level loglevel_t;

/* Initialize debugging. */
void debug_init(void);

/* Log text to the active debug interface, ignoring loglevel. */
void debug_puts(char *str);
void debug_putc(char c);



/**
 * Core debugging print for GreatFET printing.
 *
 * @param loglevel The log level at which the given string should be printed.
 * @param fmt Format string; matches printf.
 */
void vprintk(int loglevel, char *fmt, va_list list);


/**
 * Core debugging print for GreatFET printing.
 *
 * @param loglevel The log level at which the given string should be printed.
 * @param fmt Format string; matches printf.
 */
void printk(int loglevel, char *fmt, ...);


/**
 * Convenience function that prints errors using the EMEREGENCY loglevel.
 */
void pr_emergency(char *fmt, ...);


/**
 * Convenience function that prints errors using the ERROR loglevel.
 */
void pr_error(char *fmt, ...);


/**
 * Convenience function that prints errors using the WARNING loglevel.
 */
void pr_warning(char *fmt, ...);


/**
 * Convenience function that prints errors using the DEBUG loglevel.
 */
void pr_debug(char *fmt, ...);


/**
 * @return true iff there is currently a debugger connected.
 */
bool debugger_is_connected(void);


#endif //__DEBUG_H__
