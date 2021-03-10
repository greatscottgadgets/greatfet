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

#include <config.h>

typedef struct backtrace_frame backtrace_frame_t;

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

	// Log printing flags.
	LOG_LEVEL_MASK = 0x0fff,
	LOG_CONTINUE = 0x8000
};
typedef enum debug_log_level loglevel_t;

#ifndef CONFIG_ENABLE_LOGGING
	//
	// If logging isn't enabled, all of our log functionality is a no-op.
	//
	#define printk(...)
	#define pr_emergency(...)
	#define pr_alert(...)
	#define pr_critical(...)
	#define pr_error(...)
	#define pr_warning(...)
	#define pr_notice(...)
	#define pr_info(...)
	#define pr_debug(...)
	#define pr_trace(...)
	#define print_backtrace(...)
	#define print_backtrace_from_frame(...)
#else

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
#ifdef CONFIG_ENABLE_QUIET_LOGGING
	#define pr_notice(...)
#else
	ATTR_PRINTF void pr_notice(char *fmt, ...);
#endif


/**
 * Convenience function that prints errors using the INFO loglevel.
 */
#ifdef CONFIG_ENABLE_QUIET_LOGGING
	#define pr_info(...)
#else
	ATTR_PRINTF void pr_info(char *fmt, ...);
#endif



/**
 * Convenience function that prints errors using the DEBUG loglevel.
 */
#ifdef CONFIG_ENABLE_VERBOSE_LOGGING
	ATTR_PRINTF void pr_debug(char *fmt, ...);
#else
	#define pr_debug(...)
#endif


/**
 * Convenience function that prints errors using the DEBUG loglevel.
 */
#if defined(CONFIG_ENABLE_VERBOSE_LOGGING_TRACING) && defined(CONFIG_ENABLE_VERBOSE_LOGGING)
	ATTR_PRINTF void pr_trace(char *fmt, ...);
#else
	#define pr_trace(...)
#endif

/**
 * @return true iff there is currently a debugger connected.
 */
bool debugger_is_connected(void);


/**
 * Prints a backtrace starting with the current line of code.
 */
void print_backtrace(loglevel_t level, uint32_t levels_to_omit);


/**
 * Prints a backtrace starting with the current line of code.
 */
void print_backtrace_from_frame(loglevel_t level, backtrace_frame_t *frame, uint32_t levels_to_omit);

#endif // ENABLE_LOGGING
#endif //__DEBUG_H__
