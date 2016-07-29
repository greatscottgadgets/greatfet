/*
 * Copyright 2013 Michael Ossmann
 * Copyright 2013 Benjamin Vernoux
 * Copyright 2014 Jared Boone, ShareBrained Technology
 *
 * This file is part of GreatFET.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

#include <stdint.h>
#include <stddef.h>

#include "spi_bus.h"

typedef union
{
	uint64_t id_64b;
	uint32_t id_32b[2]; /* 2*32bits 64bits Unique ID */
	uint8_t id_8b[8]; /* 8*8bits 64bits Unique ID */
} spiflash_unique_id_t;

struct spiflash_driver_t {
	spi_target_t* target;
	void (*target_init)(spi_target_t* const drv);
	size_t page_len;
	size_t num_pages;
	size_t num_bytes;
	uint8_t device_id;
};

struct spiflash_driver_t;
typedef struct spiflash_driver_t spiflash_driver_t;

void spiflash_setup(spiflash_driver_t* const drv);
void spiflash_chip_erase(spiflash_driver_t* const drv);
void spiflash_program(spiflash_driver_t* const drv, uint32_t addr, uint32_t len, uint8_t* data);
uint8_t spiflash_get_device_id(spiflash_driver_t* const drv);
void spiflash_get_unique_id(spiflash_driver_t* const drv, spiflash_unique_id_t* unique_id);
void spiflash_read(spiflash_driver_t* const drv, uint32_t addr, uint32_t len, uint8_t* const data);

#endif//__SPIFLASH_H__
