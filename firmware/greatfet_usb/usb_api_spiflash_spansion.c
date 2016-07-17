/*
 * Copyright 2012 Jared Boone
 * Copyright 2013 Benjamin Vernoux
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

#include "usb_api_spiflash.h"
#include "usb_queue.h"

#include <gpio.h>
#include <stddef.h>
#include <greatfet_core.h>
#include <s25fl064p.h>
#include <s25fl064p_target.h>

/* Buffer size == spi_flash_spansion.page_len */
uint8_t spiflash_spansion_buffer[256U];
static struct gpio_t gpio_s25fl064p_select	= GPIO(2, 10); /* P5_1 */

usb_request_status_t usb_vendor_request_read_spiflash_spansion(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint32_t addr;
	uint16_t len;

	/* s25fl064p from my technicolor router */
	const ssp_config_t ssp_config_s25fl064p = {
		.data_bits = SSP_DATA_8BITS,
		.serial_clock_rate = 2,
		.clock_prescale_rate = 2,
		.gpio_select = &gpio_s25fl064p_select,
	};
	
	spi_bus_t spi_bus_ssp1 = {
		.obj = (void*)SSP1_BASE,
		.config = &ssp_config_s25fl064p,
		.start = spi_ssp_start,
		.stop = spi_ssp_stop,
		.transfer = spi_ssp_transfer,
		.transfer_gather = spi_ssp_transfer_gather,
	};
	
	s25fl064p_driver_t spi_flash_spansion = {
		.bus = &spi_bus_ssp1,
		.gpio_hold = NULL,
		.gpio_wp = NULL,
		.target_init = s25fl064p_target_init,
	};
	
	spi_bus_start(spi_flash_spansion.bus, &ssp_config_s25fl064p);
	s25fl064p_setup(&spi_flash_spansion);

	if (stage == USB_TRANSFER_STAGE_SETUP) 
	{
		addr = (endpoint->setup.value << 16) | endpoint->setup.index;
		len = endpoint->setup.length;
		if ((len > spi_flash_spansion.page_len) || (addr > spi_flash_spansion.num_bytes)
			    || ((addr + len) > spi_flash_spansion.num_bytes)) {
			return USB_REQUEST_STATUS_STALL;
		} else {
			s25fl064p_read(&spi_flash_spansion, addr, len, &spiflash_spansion_buffer[0]);
			usb_transfer_schedule_block(endpoint->in, &spiflash_spansion_buffer[0], len,
						    NULL, NULL);
			return USB_REQUEST_STATUS_OK;
		}
	} else if (stage == USB_TRANSFER_STAGE_DATA) 
	{
			addr = (endpoint->setup.value << 16) | endpoint->setup.index;
			len = endpoint->setup.length;
			/* This check is redundant but makes me feel better. */
			if ((len > spi_flash_spansion.page_len) || (addr > spi_flash_spansion.num_bytes)
					|| ((addr + len) > spi_flash_spansion.num_bytes)) 
			{
				return USB_REQUEST_STATUS_STALL;
			} else
			{
				usb_transfer_schedule_ack(endpoint->out);
				return USB_REQUEST_STATUS_OK;
			}
	} else 
	{
		return USB_REQUEST_STATUS_OK;
	}
}

