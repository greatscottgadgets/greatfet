/*
 * This file is part of GreatFET.
 *
 * ISR stubs for Rhododendron.
 */

 .syntax unified


// Register address constants.
.equ SGPIO_POSITION_REGISTER,          0x401011C0


// Comment this out if you're sure text is mutable and want the benefits of being in the text section. :)
//.section .data.generated_code
.section .text
.thumb


//
// Pin direction change ISR.
//
.thumb_func
.global rhododendron_direction_isr
rhododendron_direction_isr:

	// Branch to our C inner implementation.
	blx rhododendron_direction_isr_inner

	// Return from our subroutine.
	bx lr
