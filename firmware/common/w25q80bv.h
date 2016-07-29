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

#ifndef __W25Q80BV_H__
#define __W25Q80BV_H__

#include "spiflash.h"
#include "spiflash_target.h"
#include "gpio_lpc.h"
#include "greatfet_core.h"

#define W25Q80BV_DEVICE_ID_RES  0x14 /* Expected device_id for W25Q16DV */

#define W25Q80BV_PAGE_LEN 256U
#define W25Q80BV_NUM_PAGES 8192U
#define W25Q80BV_NUM_BYTES (W25Q80BV_PAGE_LEN * W25Q80BV_NUM_PAGES)

struct gpio_t gpio_spiflash_hold   = GPIO(1, 14);
struct gpio_t gpio_spiflash_wp     = GPIO(1, 15);
struct gpio_t gpio_spiflash_select = GPIO(5, 11);
spi_target_t spi_target = {
	.bus = &spi_bus_ssp0,
	.gpio_hold = &gpio_spiflash_hold,
	.gpio_wp = &gpio_spiflash_wp,
	.gpio_select = &gpio_spiflash_select,
};

spiflash_driver_t spi_flash_drv = {
	.target = &spi_target,
    .target_init = spiflash_target_init,
	.page_len = W25Q80BV_PAGE_LEN,
	.num_pages = W25Q80BV_NUM_PAGES,
	.num_bytes = W25Q80BV_NUM_BYTES,
    .device_id = W25Q80BV_DEVICE_ID_RES,
};


#endif//__W25Q80BV_H__
