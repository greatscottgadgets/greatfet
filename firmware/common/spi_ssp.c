/*
 * This file is part of GreatFET
 */

#include "spi_ssp.h"

#include <libopencm3/lpc43xx/rgu.h>
#include <libopencm3/lpc43xx/ssp.h>

void spi_ssp_start(spi_target_t* target, const void* const _config) {
	spi_bus_t* const bus = target->bus;
	const ssp_config_t* const config = _config;

	if( bus->obj == (void*)SSP0_BASE ) {
		/* Reset SPIFI peripheral before to Erase/Write SPIFI memory through SPI */
		RESET_CTRL1 = RESET_CTRL1_SPIFI_RST;
	}

	gpio_set(target->gpio_select);
	gpio_output(target->gpio_select);

	SSP_CR1(bus->obj) = 0;
	SSP_CPSR(bus->obj) = config->clock_prescale_rate;
	SSP_CR0(bus->obj) =
		  (config->serial_clock_rate << 8)
		| SSP_CPOL_0_CPHA_0
		| SSP_FRAME_SPI
		| config->data_bits
		;
	SSP_CR1(bus->obj) =
		  SSP_SLAVE_OUT_ENABLE
		| SSP_MASTER
		| SSP_ENABLE
		| SSP_MODE_NORMAL
		;

	bus->config = config;
}

void spi_ssp_stop(spi_bus_t* const bus) {
	SSP_CR1(bus->obj) = 0;
}

static void spi_ssp_wait_for_tx_fifo_not_full(spi_bus_t* const bus) {
	while( (SSP_SR(bus->obj) & SSP_SR_TNF) == 0 );
}

static void spi_ssp_wait_for_rx_fifo_not_empty(spi_bus_t* const bus) {
	while( (SSP_SR(bus->obj) & SSP_SR_RNE) == 0 );
}

static void spi_ssp_wait_for_not_busy(spi_bus_t* const bus) {
	while( SSP_SR(bus->obj) & SSP_SR_BSY );
}

static uint32_t spi_ssp_transfer_word(spi_bus_t* const bus, const uint32_t data) {
	spi_ssp_wait_for_tx_fifo_not_full(bus);
	SSP_DR(bus->obj) = data;
	spi_ssp_wait_for_not_busy(bus);
	spi_ssp_wait_for_rx_fifo_not_empty(bus);
	return SSP_DR(bus->obj);
}

/**
 * Variant of spi_ssp_transfer_gather that does not assert or de/assert chip select.
 * This allows an external source to control the chip select.
 */
void spi_ssp_transfer_gather_partial(spi_target_t* target,
	const spi_transfer_t* const transfers, const size_t count) {

	spi_bus_t* const bus = target->bus;
	const bool word_size_u16 = (SSP_CR0(bus->obj) & 0xf) > SSP_DATA_8BITS;

	for(size_t i=0; i<count; i++) {
		const size_t data_count = transfers[i].count;

		if( word_size_u16 ) {
			uint16_t* const data = transfers[i].data;
			for(size_t j=0; j<data_count; j++) {
				data[j] = spi_ssp_transfer_word(bus, data[j]);
			}
		} else {
			uint8_t* const data = transfers[i].data;
			for(size_t j=0; j<data_count; j++) {
				data[j] = spi_ssp_transfer_word(bus, data[j]);
			}
		}
	}
}

void spi_ssp_transfer_gather(spi_target_t* target,
							 const spi_transfer_t* const transfers,
							 const size_t count) {
	gpio_clear(target->gpio_select);
	spi_ssp_transfer_gather_partial(target, transfers, count);
	gpio_set(target->gpio_select);
}



void spi_ssp_transfer(spi_target_t* target, void* const data,
					  const size_t count) {
	const spi_transfer_t transfers[] = {
		{ data, count },
	};
	spi_ssp_transfer_gather(target, transfers, 1);
}


void spi_ssp_transfer_data(spi_target_t* target, void* const data,
					  const size_t count) {
	const spi_transfer_t transfers[] = {
		{ data, count },
	};
	spi_ssp_transfer_gather_partial(target, transfers, 1);
}

