/*
 * This file is part of GreatFET
 */

SECTIONS
{
	.text : {
		PROVIDE(__m0_start__ = .);
		KEEP(*(.m0_bin*));
		. = ALIGN(4);
		PROVIDE(__m0_end__ = .);
	} >rom
}
