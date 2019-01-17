/*
 * This file is part of GreatFET
 */

 #ifndef __DEBUG_H__
 #define __DEBUG_H__

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <toolchain.h>
#include <errno.h>

#include <greatfet_core.h>

// Possible log levels.
enum debug_log_level {
	LOGLEVEL_EMERGENCY,
	LOGLEVEL_ALERT,
	LOGLEVEL_CRITICAL,
	LOGLEVEL_ERROR,
	LOGLEVEL_WARNING,
	LOGLEVEL_NOTICE,
	LOGLEVEL_INFO,
	LOGLEVEL_DEBUG,
	LOGLEVEL_TRACE,
};
typedef enum debug_log_level loglevel_t;

/* Initialize debugging. */
void debug_init(void);

/* Log text to the active debug interface, ignoring loglevel. */
void debug_puts(char *str);
void debug_putc(char c);


/**
 * @return The total used size in the debug ring.
 */
size_t debug_ring_used_space(void);


/**
 * @return The amount of free bytes in the debug ring.
 */
size_t debug_ring_free_space(void);

/**
 * @return True iff the debug ring is full.
 */
bool debug_ring_full(void);


/**
 * @return True iff the debug ring is empty.
 */
bool debug_ring_empty(void);



/**
 * Reads a set of raw bytes from the system's debug ringbuffer.
 *
 * @param buffer The buffer to be populated.
 * @param maximum The maximum length to be populated.
 */
unsigned int debug_ring_read(char *buffer, unsigned int maximum, bool clear);


/**
 * Consumes a single line from the debug ring, freeing space without
 * leaving partial sentences in the ringbuffer.
 */
void debug_ring_reclaim_line(void);


/**
 * Writes a string to the system's debug ringbuffer.
 *
 * @param str The string to write.
 * @param length The length of the string to write.
 */
void debug_ring_write(const char *const str, unsigned int length);


/**
 * Writes a string to the system's debug ringbuffer.
 *
 * @param str The string to write.
 * @param length The length of the string to write.
 */
void debug_ring_write_string(const char *const str);


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
ATTR_PRINTF_N(2) void printk(int loglevel, char *fmt, ...);


/**
 * Convenience function that prints errors using the EMEREGENCY loglevel.
 */
ATTR_PRINTF void pr_emergency(char *fmt, ...);


/**
 * Convenience function that prints errors using the ALERT loglevel.
 */
ATTR_PRINTF void pr_alert(char *fmt, ...);


/**
 * Convenience function that prints errors using the CRITICAL loglevel.
 */
ATTR_PRINTF void pr_critical(char *fmt, ...);


/**
 * Convenience function that prints errors using the ERROR loglevel.
 */
ATTR_PRINTF void pr_error(char *fmt, ...);


/**
 * Convenience function that prints errors using the WARNING loglevel.
 */
ATTR_PRINTF void pr_warning(char *fmt, ...);


/**
 * Convenience function that prints errors using the INFO loglevel.
 */
ATTR_PRINTF void pr_info(char *fmt, ...);


/**
 * Convenience function that prints errors using the DEBUG loglevel.
 */
ATTR_PRINTF void pr_debug(char *fmt, ...);


/**
 * Convenience function that prints errors using the DEBUG loglevel.
 */
ATTR_PRINTF void pr_trace(char *fmt, ...);

/**
 * @return true iff there is currently a debugger connected.
 */
bool debugger_is_connected(void);


#endif //__DEBUG_H__
