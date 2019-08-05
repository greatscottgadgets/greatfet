/*
 * This file is part of GreatFET
 */

#include "sgpio_isr.h"

#include <libopencm3/lpc43xx/sgpio.h>

#include "usb_bulk_buffer.h"

//
// FIXME: this whole module is a stand-in; it will disappear shortly when
//


// Offsets into the shadow register:
// A = 0
// B = 4
// C = 8
// D = 12
// E = 16
// F = 20
// G = 24
// H = 28
// I = 32
// J = 36
// K = 40
// L = 44
// M = 48
// N = 52
// O = 56
// P = 60

void sgpio_isr_input() {
	SGPIO_CLR_STATUS_1 = (1 << SGPIO_SLICE_A);

	// Our slice chain is set up as follows (ascending data age; arrows are reversed for flow):
	//     L  -> F  -> K  -> C -> J  -> E  -> I  -> A
	// Which has equivalent shadow register offsets:
	//     44 -> 20 -> 40 -> 8 -> 36 -> 16 -> 32 -> 0

// 0  <-- 7   r1
// 4          r2
// 8  <-- 3   r3
// 12         r4
// 16 <-- 5   r5
// 20 <-- 1   r6
// 24         r7
// 28         r8
// 32 <-- 6   r9
// 36 <-- 4   r10
// 40 <-- 2   r11
// 44 <-- 0   r12

//12 -> 6 -> 11 -> 3 -> 10 -> 5 -> 9 -> 1

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
	usb_bulk_buffer_fill_count += 32;
}


// XXX replace me
void sgpio_isr_input_bank_b() {
	SGPIO_CLR_STATUS_1 = (1 << SGPIO_SLICE_B);

	// Our slice chain is set up as follows (ascending data age; arrows are reversed for flow):
	//    P  -> H  -> O  -> D  -> N  -> G  -> M  -> B
	// Which has equivalent shadow register offsets:
	//    60 -> 28 -> 56 -> 12 -> 52 -> 24 -> 48 -> 4

	uint32_t* const p = (uint32_t*)&usb_bulk_buffer[usb_bulk_buffer_offset];
	__asm__(
		"ldr r0, [%[SGPIO_REG_SS], #60]\n\t"
		"str r0, [%[p], #0]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #28]\n\t"
		"str r0, [%[p], #4]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #56]\n\t"
		"str r0, [%[p], #8]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #12]\n\t"
		"str r0, [%[p], #12]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #52]\n\t"
		"str r0, [%[p], #16]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #24]\n\t"
		"str r0, [%[p], #20]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #48]\n\t"
		"str r0, [%[p], #24]\n\t"
		"ldr r0, [%[SGPIO_REG_SS], #4]\n\t"
		"str r0, [%[p], #28]\n\t"
		:
		: [SGPIO_REG_SS] "l" (SGPIO_PORT_BASE + 0x100),
		  [p] "l" (p)
		: "r0"
	);
	usb_bulk_buffer_offset = (usb_bulk_buffer_offset + 32) & usb_bulk_buffer_mask;
	usb_bulk_buffer_fill_count += 32;
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
