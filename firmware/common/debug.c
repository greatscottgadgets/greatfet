/*
 * This file is part of GreatFET
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <toolchain.h>
#include <backtrace.h>

#include <drivers/reset.h>

#include <pins.h>
#include <time.h>

#ifndef __RUNNING_ON_HOST__
	#include <libopencm3/lpc43xx/scu.h>
	#include <libopencm3/lpc43xx/uart.h>
	#include <libopencm3/cm3/memorymap.h>
	#include <libopencm3/cm3/scs.h>
#endif

#include <debug.h>

#include <config.h>

#ifdef CONFIG_ENABLE_LOGGING

#define SEMIHOST_SWI		"0xAB"
#define SEMIHOST_STDOUT_FILENO "2"
#define SEMIHOST_SYS_WRITEC	"0x3"
#define SEMIHOST_SYS_WRITE0	"0x4"
#define SEMIHOST_SYS_WRITE	"0x5"

extern volatile uint32_t reset_reason;



// Store the active loglevel.
// TODO: reduce this, maybe?
static loglevel_t debug_loglevel = LOGLEVEL_INFO;


#ifdef CONFIG_ENABLE_DEBUG_RING

/* Storage for the debug ringbuffer. */
static char debug_ring[CONFIG_DEBUG_RING_SIZE] ATTR_PERSISTENT;

unsigned int debug_read_index ATTR_PERSISTENT;
unsigned int debug_write_index ATTR_PERSISTENT;


/**
 * Initializes debugging support.
 */
void debug_ring_init(void)
{
	// If it doesn't seem likely our debug ring is intact from a
	// previous boot, then clear out the debug ring.
	if (system_persistent_memory_likely_intact()) {
		debug_ring_write_string("\n");
	} else {
		debug_read_index = 0;
		debug_write_index = 0;
	}

	// If we're not omitting timestamps, print a dummy timestamp to indicate our start.
	#ifndef CONFIG_DEBUG_OMIT_TIMESTAMPS
		debug_ring_write_string("[------------] ");
	#endif

	debug_ring_write_string("System started after ");
	debug_ring_write_string(system_get_reset_reason_string());
	debug_ring_write_string(".\n");
}
CALL_ON_PREINIT(debug_ring_init);



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

	// Update the read pointer, if we're clearing as we read.G
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

#endif

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

#ifdef CONFIG_ENABLE_SEMIHOSTING

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

#endif

/**
 * Prints a string to the debug console. Ignores loglevel.
 *
 * @param str The string to print.
 */
void debug_puts(char *str)
{
	#ifdef CONFIG_ENABLE_DEBUG_RING
		debug_ring_write_string(str);
	#endif

	#ifdef CONFIG_ENABLE_SEMIHOSTING
		if (debugger_is_connected())
			semihosting_puts(str);
	#endif

}

/**
 * Prints a character to the debug console. Ignores loglevel.
 *
 * @param c The char to print.
 */
void debug_putc(char c)
{
	#ifdef CONFIG_ENABLE_DEBUG_RING
		debug_ring_write(&c, 1);
	#endif

	#ifdef CONFIG_ENABLE_SEMIHOSTING
		if (debugger_is_connected())
			semihosting_putc(c);
	#endif
}

/**
 * Core debugging print for GreatFET printing.
 *
 * @param loglevel The log level at which the given string should be printed.
 * @param fmt Format string; matches printf.
 */
void vprintk(int loglevel, char *fmt, va_list list)
{
	int core_level = loglevel & LOG_LEVEL_MASK;
	bool skip_header = (loglevel & LOG_CONTINUE);

	// If this statement is above the maximum level to be displayed,
	// bail out.
	if (core_level > debug_loglevel)
		return;


#ifdef CONFIG_ENABLE_LOG_TIMESTAMPS
	if (!skip_header) {
		printf("[%12" PRIu32 "] ", get_time());
	}
#endif

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
 * Convenience function that prints errors using the CRITICAL loglevel.
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
#ifndef CONFIG_ENABLE_QUIET_LOGGING
	void pr_info(char *fmt, ...)
	{
		va_list list;

		va_start(list, fmt);
		vprintk(LOGLEVEL_INFO, fmt, list);
		va_end(list);
	}
#endif

#ifdef CONFIG_ENABLE_VERBOSE_LOGGING

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

#endif

#ifdef CONFIG_ENABLE_VERBOSE_LOGGING_TRACING

/**
 * Convenience function that prints errors using the TRACE loglevel.
 * Normally, calls to pr_trace are optimized out if DEBUG_INCLUDE_TRACE
 * isn't configured here.
 */
void pr_trace(char *fmt, ...)
{
	va_list list;

	va_start(list, fmt);
	vprintk(LOGLEVEL_TRACE, fmt, list);
	va_end(list);
}

#endif

/**
 * Prints a backtrace. Very experimental.
 */

int main(void);
void usage_fault_handler(void);

/**
 * Weak/empty implementation of backtrace functionality for targets that don't include the backtrace module.
 */
ATTR_WEAK int _backtrace_unwind(backtrace_t *backtrace, int size, backtrace_frame_t *frame)
{
	(void)backtrace;
	(void)size;
	(void)frame;
	return 0;
}


/**
 * Prints a backtrace starting with the current line of code.
 */
void print_backtrace_from_frame(loglevel_t level, backtrace_frame_t *frame, uint32_t levels_to_omit)
{
	(void)level;
	(void)frame;
	(void)levels_to_omit;

	#ifdef CONFIG_ENABLE_BACKTRACE

	#ifndef CONFIG_ENABLE_FUNCTION_NAMES
	const char *missing_name= "unknown";
	#endif

	// Fetch a backtrace...
	backtrace_t backtrace[CONFIG_MAX_BACKTRACE_SIZE];
	int length = _backtrace_unwind(backtrace, CONFIG_MAX_BACKTRACE_SIZE, frame);

	printk(level, "Call Trace:\n");


		for(int i = levels_to_omit; i < length; ++i) {
			const char *decorator;

			backtrace_t *entry = &backtrace[i];
			uintptr_t offset = entry->address - entry->function;
			const char *name = entry->name;

			decorator = (i == 0) ? "<pc>" : "";

			// FIXME: is this right?
			// end our ISR-context handlers in the right place
			if ((entry->function + 1) == usage_fault_handler) {
				printk(level, "\t [<  nvic  >] --switch to interrupt context-- \n");
				break;
			}

			#ifndef CONFIG_ENABLE_FUNCTION_NAMES
				name = missing_name;
			#endif

			if (offset) {
				printk(level, "\t [<%p>] %s+0x%x/0x%x %s\n", entry->address, name, (unsigned int)offset, (unsigned int)entry->address, decorator);
			} else {
				printk(level, "\t [<%p>] %s %s\n", entry->address, name, decorator);
			}

			// If we've reached main, it's time to stop. We don't need to expose CRT0 or bootrom details.
			// Note that we need to add one to the function address to match the thumb-encoded address.
			if ((entry->function + 1) == main) {
				break;
			}
		}
	#else
		printk(level, "Call trace not avaiable in this build configuration.\n");
	#endif

}


inline void print_backtrace(loglevel_t level, uint32_t levels_to_omit)
{
	#ifdef CONFIG_ENABLE_BACKTRACE
		// Fetch the current program counter.
		register uint32_t pc;
		__asm__ volatile("mov %0, pc" : "=r"(pc));

		// Build a backtrace frame to print from based on the current state.
		backtrace_frame_t frame;
		frame.sp = (uint32_t)__builtin_frame_address(0);
		frame.fp = (uint32_t)__builtin_frame_address(0);
		frame.lr = (uint32_t)__builtin_return_address(0);
		frame.pc = pc;

		// Print the main backtrace.
		print_backtrace_from_frame(level, &frame, levels_to_omit + 1);
	#else
		// Re-use our generic backtrace handelr to print an error message.
		print_backtrace_from_frame(level, NULL, levels_to_omit + 1);
	#endif
}


#endif
