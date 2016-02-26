/*
 * Copyright 2013 Michael Ossmann
 * Copyright 2013 Benjamin Vernoux
 * Copyright 2014 Jared Boone, ShareBrained Technology
 * Copyright 2016 Dominic Spill
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

/*
 * This is a rudimentary driver for the S25FL064P SPI Flash IC using the
 * LPC43xx's SSP1 peripheral (not quad SPIFI).  The only goal here is to allow
 * programming the flash.
 */

#include <stdint.h>
#include <stddef.h>

#include "s25fl064p.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define S25FL064P_READ_DATA    0x03
#define S25FL064P_FAST_READ    0x0b
#define S25FL064P_READ_STATUS1 0x05
#define S25FL064P_DEVICE_ID    0xAB

#define S25FL064P_STATUS_BUSY  0x01

#define S25FL064P_DEVICE_ID_RES  0x16 /* Expected device_id for S25FL064P */

#define S25FL064P_PAGE_LEN 256U
#define S25FL064P_NUM_PAGES 8192U
#define S25FL064P_NUM_BYTES (S25FL064P_PAGE_LEN * S25FL064P_NUM_PAGES)

/*
 * Set up pins for GPIO and SPI control, configure SSP0 peripheral for SPI.
 * SSP0_SSEL is controlled by GPIO in order to handle various transfer lengths.
 */
void s25fl064p_setup(s25fl064p_driver_t* const drv)
{
	uint8_t device_id;

	drv->page_len = S25FL064P_PAGE_LEN;
	drv->num_pages = S25FL064P_NUM_PAGES;
	drv->num_bytes = S25FL064P_NUM_BYTES;

	drv->target_init(drv);

	device_id = 0;
	while(device_id != S25FL064P_DEVICE_ID_RES)
	{
		device_id = s25fl064p_get_device_id(drv);
	}
}

uint8_t s25fl064p_get_status(s25fl064p_driver_t* const drv)
{
	uint8_t data[] = { S25FL064P_READ_STATUS1, 0xFF };
	spi_bus_transfer(drv->bus, data, ARRAY_SIZE(data));
	return data[1];
}

/* Release power down / Device ID */  
uint8_t s25fl064p_get_device_id(s25fl064p_driver_t* const drv)
{
	uint8_t data[] = {
		S25FL064P_DEVICE_ID,
		0xFF, 0xFF, 0xFF, 0xFF
	};
	spi_bus_transfer(drv->bus, data, ARRAY_SIZE(data));
	return data[4];
}

void s25fl064p_wait_while_busy(s25fl064p_driver_t* const drv)
{
	while (s25fl064p_get_status(drv) & S25FL064P_STATUS_BUSY);
}

/* write an arbitrary number of bytes */
void s25fl064p_read(s25fl064p_driver_t* const drv, uint32_t addr, uint32_t len, uint8_t* const data)
{
	/* do nothing if we would overflow the flash */
	if ((len > drv->num_bytes) || (addr > drv->num_bytes)
			|| ((addr + len) > drv->num_bytes))
		return;

	s25fl064p_wait_while_busy(drv);

	uint8_t header[] = {
		S25FL064P_FAST_READ,
		(addr & 0xFF0000) >> 16,
		(addr & 0xFF00) >> 8,
		addr & 0xFF,
		0x00
	};

	const spi_transfer_t transfers[] = {
		{ header, ARRAY_SIZE(header) },
		{ data, len }
	};

	spi_bus_transfer_gather(drv->bus, transfers, ARRAY_SIZE(transfers));
}
