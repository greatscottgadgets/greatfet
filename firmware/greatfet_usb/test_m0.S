
// Constants that point to registers we'll need to modify in the SGPIO block.
.equ LED2_PORT, 2
.equ LED2_PIN, 1
.equ LED2_WORD_REGISTER, (0x400f5000) + (32 * 4 * LED2_PORT) + (4 * LED2_PIN)

.global m0_vector_table



// Create the vector table for the Cortex M0.
.section .text
m0_vector_table:
	.word 0x10092000
	.word m0_reset_handler // Reset
	.word fault_handler // NMI
	.word fault_handler // Hard fault.
	.space 28 // Reserved
	.word fault_handler // SVC
	.space 8
	.word fault_handler // PendSV
	.word fault_handler // Systick

	// IRQs
	.rept 32
		.word fault_handler
	.endr


.thumb_func
.global m0_reset_handler
m0_reset_handler:
	ldr r0, =0x10090300
	ldr r1, =0xc001c0de
	str r1, [r0]

	ldr r0, =LED2_WORD_REGISTER
	ldr r1, =0x0
	str r1, [r0]
	b m0_reset_handler


.thumb_func
fault_handler:
	b fault_handler
