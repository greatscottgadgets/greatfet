/*
 * This file is part of GreatFET
 */

#include "usb_bulk_buffer.h"

const uint32_t usb_bulk_buffer_mask = 32768 - 1;
volatile uint32_t usb_bulk_buffer_offset = 0;
volatile uint32_t usb_bulk_buffer_fill_count = 0;
