/*
 * This file is part of GreatFET
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
