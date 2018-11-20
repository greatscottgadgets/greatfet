/*
 * This file is part of GreatFET
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <toolchain.h>

#include <pins.h>
#include <time.h>

#ifndef __RUNNING_ON_HOST__
	#include <libopencm3/lpc43xx/scu.h>
	#include <libopencm3/lpc43xx/uart.h>
	#include <libopencm3/cm3/memorymap.h>
	#include <libopencm3/cm3/scs.h>
#endif

#include "debug.h"


#define SEMIHOST_SWI		"0xAB"
#define SEMIHOST_STDOUT_FILENO "2"
#define SEMIHOST_SYS_WRITEC	"0x3"
#define SEMIHOST_SYS_WRITE0	"0x4"
#define SEMIHOST_SYS_WRITE	"0x5"

// FIXME: Don't assume this from Common -- pull this in from e.g. a configuration header file.
#ifndef CONFIG_DEBUG_BUFFER_SIZE
#define CONFIG_DEBUG_BUFFER_SIZE 4096
#undef CONFIG_DEBUG_INCLUDE_TRACE
#endif

extern volatile uint32_t reset_reason;

/* Storage for the debug ringbuffer. */
static char debug_ring[CONFIG_DEBUG_BUFFER_SIZE] ATTR_SECTION(".bss.persistent");

unsigned int debug_read_index ATTR_SECTION(".bss.persistent");
unsigned int debug_write_index ATTR_SECTION(".bss.persistent");

// Store the active loglevel.
// TODO: reduce this, maybe?
static loglevel_t debug_loglevel = LOGLEVEL_INFO;


/**
 * Return true iff it's likely our debug ring is intact
 * from a previous boot; e.g. after a soft reset. This can be
 * used to keep the debug ring around across soft resets.
 */
static bool debug_memory_seems_okay()
{
	return
		(reset_reason & RESET_DEBUG_LIKELY_VALID_MASK) ==
		RESET_DEBUG_LIKELY_VALID_MASK;
}


/**
 * Initializes debugging support.
 */
void debug_init(void)
{
	// If it doesn't seem likely our debug ring is intact from a
	// previous boot, then clear out the debug ring.
	if (debug_memory_seems_okay()) {
		debug_ring_write_string("\n");
	} else {
		debug_read_index = 0;
		debug_write_index = 0;
	}

}
CALL_ON_PREINIT(debug_init);


/**
 * @return The total used size in the debug ring.
 */
size_t debug_ring_used_space(void)
{
	// Since we let the read and write buffers grow unbounded,
	// we never need to handle wraparound. Our size is simply the difference.
	return debug_write_index - debug_read_index;
}


/**
 * @return The amount of free bytes in the debug ring.
 */
size_t debug_ring_free_space(void)
{
	return sizeof(debug_ring) - debug_ring_used_space();
}


/**
 * @return True iff the debug ring is full.
 */
bool debug_ring_full(void)
{
	return debug_ring_used_space() == sizeof(debug_ring);
}


/**
 * @return True iff the debug ring is empty.
 */
bool debug_ring_empty(void)
{
	return debug_write_index == debug_read_index;
}


/** @return the wrapped version of the debug ring's write index */
static unsigned int debug_ring_write_index()
{
	return debug_write_index % sizeof(debug_ring);
}


/** @return the wrapped version of the debug ring's read index */
static unsigned int debug_ring_read_index()
{
	return debug_read_index % sizeof(debug_ring);
}


/**
 * Reads a set of raw bytes from the system's debug ringbuffer.
 *
 * @param buffer The buffer to be populated.
 * @param maximum The maximum length to be populated.
 */
unsigned int debug_ring_read(char *buffer, unsigned int maximum, bool clear)
{
	unsigned int immediate_length, wrapped_length;
	unsigned int length = MIN(maximum, debug_ring_used_space());

	// Figure out how much data is available following the read buffer, vs
	// available at the beginning of the ring.
	immediate_length = MIN(sizeof(debug_ring) - debug_ring_read_index(), length);
	wrapped_length = length - immediate_length;

	// Copy the immediate and wrapped sections.
	memcpy(buffer, &debug_ring[debug_ring_read_index()], immediate_length);
	memcpy(&buffer[immediate_length], debug_ring, wrapped_length);

	// Update the read pointer, if we're clearing as we read.
	if (clear)
		debug_read_index += length;

	// Return the length actually read.
	return length;
}


/**
 * Consumes a single line from the debug ring, freeing space without
 * leaving partial sentences in the ringbuffer.
 */
void debug_ring_reclaim_line(void)
{
	// Find the next newline after the read pointer.
	unsigned int walk_index;

	// Walk until we find a newline.
	for (walk_index = debug_read_index; walk_index < debug_write_index; ++walk_index) {

		// Figure out where the spot in the ring buffer is that contains our current index.
		unsigned int index_to_read = walk_index % sizeof(debug_ring);

		// If we've found the newline, advance the read pointer to this index,
		// and stop searching.
		if (debug_ring[index_to_read] == '\n') {
			debug_read_index = walk_index + 1;
			return;
		}
	}
}


/**
 * Writes a string to the system's debug ringbuffer.
 *
 * @param str The string to write.
 * @param length The length of the string to write.
 */
void debug_ring_write(const char *const str, unsigned int length)
{
	unsigned int immediate_length, wrapped_length;

	// If this can't fit in our ringbuffer _at all_, truncate it to the
	// size of the ringbuffer.
	if (length > sizeof(debug_ring))
		length = sizeof(debug_ring);

	// If we can't fit the given string, reclaim whole lines until we can.
	while (debug_ring_free_space() < length)
		debug_ring_reclaim_line();

	// Figure out how much of the string should go immediately following
	// the write pointer, vs wrapping around to the beginning of the buffer.
	immediate_length = MIN(sizeof(debug_ring) - debug_ring_write_index(), length);
	wrapped_length = length - immediate_length;

	// Copy the immediate and wrapped sections.
	memcpy(&debug_ring[debug_ring_write_index()], str, immediate_length);
	memcpy(debug_ring, &str[immediate_length], wrapped_length);

	// Update the write pointer.
	debug_write_index += length;
}


/**
 * Writes a string to the system's debug ringbuffer.
 *
 * @param str The string to write.
 * @param length The length of the string to write.
 */
void debug_ring_write_string(const char *const str)
{
	// Get the length of the string to be committed to the ringbuffer.
	// This is the length we'll need to copy.
	unsigned int length = strlen(str);

	// Write the string with the relevant length.
	debug_ring_write(str, length);
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
#ifndef __RUNNING_ON_HOST__
	return (SCS_DHCSR & SCS_DHCSR_C_DEBUGEN);
#else
	return true;
#endif
}


/**
 * Prints a single character via semihosting.
 * Will hard fault if no debugger is connected.
 *
 * @param c The character to print.
 */
static void semihosting_putc(char c)
{
#ifndef __RUNNING_ON_HOST__
	// Operation #3, SYS_WRITEC; r0 = operation number, r1 = pointer to character to write
	asm volatile(
		 "mov  r1, %0\n"
		 "mov  r0, #" SEMIHOST_SYS_WRITEC "\n"
		 "bkpt	" SEMIHOST_SWI "\n"
		 : : "r" (&c) : "r0", "r1", "memory");
#else
	putchar(c);
#endif
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
#ifndef __RUNNING_ON_HOST__
	asm volatile(
		 "mov  r1, %0\n"
		 "mov  r0, #" SEMIHOST_SYS_WRITE "\n"
		 "bkpt	" SEMIHOST_SWI "\n"
		 : : "r" (parameters) : "r0", "r1", "memory");
#else
	puts((const char *)s);
#endif
}

/**
 * Prints a string to the debug console. Ignores loglevel.
 *
 * @param str The string to print.
 */
void debug_puts(char *str)
{
	debug_ring_write_string(str);

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
	debug_ring_write(&c, 1);

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

	// TODO: support something like Linux's LOGLEVEL_CONTINUE

	printf("[%12" PRIu32 "] ", get_time());
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
 * Convenience function that prints errors using the ALERT loglevel.
 */
void pr_alert(char *fmt, ...)
{
	va_list list;

	va_start(list, fmt);
	vprintk(LOGLEVEL_ALERT, fmt, list);
	va_end(list);
}


/**
 * Convenience function that prints errors using the ERROR loglevel.
 */
void pr_critical(char *fmt, ...)
{
	va_list list;

	va_start(list, fmt);
	vprintk(LOGLEVEL_CRITICAL, fmt, list);
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
void pr_info(char *fmt, ...)
{
	va_list list;

	va_start(list, fmt);
	vprintk(LOGLEVEL_INFO, fmt, list);
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


/**
 * Convenience function that prints errors using the TRACE loglevel.
 * Normally, calls to pr_trace are optimized out if DEBUG_INCLUDE_TRACE
 * isn't configured here.
 */
void pr_trace(char *fmt, ...)
{
	(void)fmt;

	#ifdef CONFIG_DEBUG_INCLUDE_TRACE
		va_list list;

		va_start(list, fmt);
		vprintk(LOGLEVEL_TRACE, fmt, list);
		va_end(list);
	#endif
}
