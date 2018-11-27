/*
 * This file is part of GreatFET
 */

MEMORY
{
	ram_local1 (rwx) : ORIGIN = 0x10000000, LENGTH = 128K
	ram_l0dable(rwx) : ORIGIN = 0x10080000, LENGTH = 16K
	ram_local2 (rwx) : ORIGIN = 0x10084000, LENGTH = 48K
	ram_sleep  (rwx) : ORIGIN = 0x10090000, LENGTH = 8K

	ram_m0     (rwx) : ORIGIN = 0x20000000, LENGTH = 28K
	ram_shared (rwx) : ORIGIN = 0x20007000, LENGTH =  4K
	ram_usb    (rwx) : ORIGIN = 0x20008000, LENGTH = 32K
}

usb_bulk_buffer = ORIGIN(ram_usb);
