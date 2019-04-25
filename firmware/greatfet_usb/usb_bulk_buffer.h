/*
 * This file is part of GreatFET
 */

#ifndef __USB_BULK_BUFFER_H__
#define __USB_BULK_BUFFER_H__

#include <stdint.h>

/* Address of usb_bulk_buffer is set in ldscripts. If you change the name of this
 * variable, it won't be where it needs to be in the processor's address space,
 * unless you also adjust the ldscripts.
 */
extern uint8_t usb_bulk_buffer[32768];

extern const uint32_t usb_bulk_buffer_mask;

extern volatile uint32_t usb_bulk_buffer_offset;
extern volatile uint32_t usb_bulk_buffer_fill_count;

#endif/*__USB_BULK_BUFFER_H__*/
