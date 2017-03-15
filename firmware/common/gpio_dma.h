/*
 * Copyright 2013 Jared Boone <jared@sharebrained.com>
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

#ifndef __GPIO_DMA_H__
#define __GPIO_DMA_H__

#include <stddef.h>

#include <libopencm3/lpc43xx/gpdma.h>

void gpio_dma_config_lli(
	gpdma_lli_t* const lli,
	const size_t lli_count,
	void* const buffer,
	void* const target_buffer,
	const size_t transfer_bytes
);

void gpio_dma_init();
void gpio_dma_tx_start(const gpdma_lli_t* const start_lli);
void gpio_dma_irq_err_clear();
void gpio_dma_irq_tc_acknowledge();
int gpio_dma_irq_is_error();
void gpio_dma_stop();

size_t gpio_dma_current_transfer_index(
	const gpdma_lli_t* const lli,
	const size_t lli_count
);

#endif/*__GPIO_DMA_H__*/
