/*
 * This file is part of libgreat.
 *
 * C Runtime 0: start of day code for the LPC4330
 */


#include <libopencm3/cm3/scb.h>
#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/cgu.h>
#include <libopencm3/lpc43xx/rtc.h>
#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/ssp.h>
#include <libopencm3/lpc43xx/timer.h>


/* This special variable is preserved across soft resets by a little bit of
 * reset handler magic. It allows us to pass a Reason across resets. */
/* FIXME: use sections to do this instead of the below */
/* FIXME: make this static and provide an accessor, somewhere? */
/* FIXME: move this out of the crt0 and to its own driver? */
volatile uint32_t reset_reason;


void main(void);

/**
 * Section start and end markers for the constructor and
 * destructor linker sections. Provided by the linker.
 */
typedef void (*funcp_t) (void);
extern funcp_t __preinit_array_start, __preinit_array_end;
extern funcp_t __init_array_start, __init_array_end;
extern funcp_t __fini_array_start, __fini_array_end;

/**
 * Section start and end markers for the standard program sections.
 * Provided by the linker.
 */
extern unsigned _data_loadaddr, _data, _edata, _bss, _ebss, _stack;
extern unsigned _etext_ram, _text_ram, _etext_rom;


/**
 * Function to be called before main, but after an initializers.
 */
static void _relocate_to_ram(void)
{
	volatile unsigned *src, *dest;

	/* Copy the code from ROM to Real RAM (if enabled) */
	if ((&_etext_ram-&_text_ram) > 0) {
		src = &_etext_rom-(&_etext_ram-&_text_ram);
		/* Change Shadow memory to ROM (for Debug Purpose in case Boot
		 * has not set correctly the M4MEMMAP because of debug)
		 */
		CREG_M4MEMMAP = (unsigned long)src;

		for (dest = &_text_ram; dest < &_etext_ram; ) {
			*dest++ = *src++;
		}

		/* Change Shadow memory to Real RAM */
		CREG_M4MEMMAP = (unsigned long)&_text_ram;

		/* Continue Execution in RAM */
	}

	/* Enable access to Floating-Point coprocessor. */
	SCB_CPACR |= SCB_CPACR_FULL * (SCB_CPACR_CP10 | SCB_CPACR_CP11);
}



/**
 * Startup code for the processor and general initialization.
 */
void __attribute__ ((naked)) reset_handler(void)
{
	volatile unsigned *src, *dest;
	funcp_t *fp;

	uint32_t stored_reset_reason = reset_reason;

	for (src = &_data_loadaddr, dest = &_data;
		dest < &_edata;
		src++, dest++) {
		*dest = *src;
	}

	for (dest = &_bss; dest < &_ebss; ) {
		*dest++ = 0;
	}

	/* 
	 * Begin executing the program from RAM, instead of
	 * ROM, if desired. This improvides performance, as we
	 * don't have to keep fetching over spifi.
	 *
	 * TODO: provide an XIP mechanism that allows us to skip
	 * this if we do want to run from spifi
	 */
	_relocate_to_ram();

	/* Constructors. */
	for (fp = &__preinit_array_start; fp < &__preinit_array_end; fp++) {
		(*fp)();
	}
	for (fp = &__init_array_start; fp < &__init_array_end; fp++) {
		(*fp)();
	}

	/* Restore our stored reset reason. */
	reset_reason = stored_reset_reason;

	/* Call the application's entry point. */
	main();

	/* Destructors. */
	for (fp = &__fini_array_start; fp < &__fini_array_end; fp++) {
		(*fp)();
	}

}
