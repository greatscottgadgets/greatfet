/*
 * This file is part of GreatFET
 */

MEMORY
{
	/* Physical address in Flash used to copy Code from Flash to RAM */
	rom_flash (rx)  : ORIGIN = 0x80000000, LENGTH =  2M
	ram_m0 (rwx) : ORIGIN = 0x20000000, LENGTH = 28K
	ram_shared (rwx) : ORIGIN = 0x20007000, LENGTH =  4K
	ram_usb (rwx) : ORIGIN = 0x20008000, LENGTH = 32K
	/* ram_usb: USB buffer. Straddles two blocks of RAM
	 * to get performance benefit of having two USB buffers addressable
	 * simultaneously (on two different buses of the AHB multilayer matrix)
	 */
}

usb_bulk_buffer = ORIGIN(ram_usb);
