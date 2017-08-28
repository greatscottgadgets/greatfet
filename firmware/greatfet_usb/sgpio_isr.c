/*
 * This file is part of GreatFET
 */

#include "sgpio_isr.h"

#include <libopencm3/lpc43xx/sgpio.h>

#include "usb_bulk_buffer.h"

void sgpio_isr_input() {
	SGPIO_CLR_STATUS_1 = (1 << SGPIO_SLICE_A);

	uint32_t* const p = (uint32_t*)&usb_bulk_buffer[usb_bulk_buffer_offset];
	__asm__(
		"ldr r0, [%[SGPIO_REG_SS], #44]\n\t"
		"str r0, [%[p], #0]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #20]\n\t"
		"str r0, [%[p], #4]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #40]\n\t"
		"str r0, [%[p], #8]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #8]\n\t"
		"str r0, [%[p], #12]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #36]\n\t"
		"str r0, [%[p], #16]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #16]\n\t"
		"str r0, [%[p], #20]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #32]\n\t"
		"str r0, [%[p], #24]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #0]\n\t"
		"str r0, [%[p], #28]\n\t"
		:
		: [SGPIO_REG_SS] "l" (SGPIO_PORT_BASE + 0x100),
		  [p] "l" (p)
		: "r0"
	);
	usb_bulk_buffer_offset = (usb_bulk_buffer_offset + 32) & usb_bulk_buffer_mask;
}

void sgpio_isr_output() {
	SGPIO_CLR_STATUS_1 = (1 << SGPIO_SLICE_A);

	uint32_t* const p = (uint32_t*)&usb_bulk_buffer[usb_bulk_buffer_offset];
	__asm__(
		"ldr r0, [%[p], #0]\n\t"
		"str r0, [%[SGPIO_REG_SS], #44]\n\t"
		"ldr r0, [%[p], #4]\n\t"
		"str r0, [%[SGPIO_REG_SS], #20]\n\t"
		"ldr r0, [%[p], #8]\n\t"
		"str r0, [%[SGPIO_REG_SS], #40]\n\t"
		"ldr r0, [%[p], #12]\n\t"
		"str r0, [%[SGPIO_REG_SS], #8]\n\t"
		"ldr r0, [%[p], #16]\n\t"
		"str r0, [%[SGPIO_REG_SS], #36]\n\t"
		"ldr r0, [%[p], #20]\n\t"
		"str r0, [%[SGPIO_REG_SS], #16]\n\t"
		"ldr r0, [%[p], #24]\n\t"
		"str r0, [%[SGPIO_REG_SS], #32]\n\t"
		"ldr r0, [%[p], #28]\n\t"
		"str r0, [%[SGPIO_REG_SS], #0]\n\t"
		:
		: [SGPIO_REG_SS] "l" (SGPIO_PORT_BASE + 0x100),
		  [p] "l" (p)
		: "r0"
	);
	usb_bulk_buffer_offset = (usb_bulk_buffer_offset + 32) & usb_bulk_buffer_mask;
}
