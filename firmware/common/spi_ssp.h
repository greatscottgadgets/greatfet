/*
 * This file is part of GreatFET
 */

#ifndef __SPI_SSP_H__
#define __SPI_SSP_H__

#include <stdint.h>
#include <stddef.h>

#include "spi_bus.h"

#include "gpio.h"

#include <libopencm3/lpc43xx/ssp.h>

typedef struct ssp_config_t {
	ssp_datasize_t data_bits;
	uint8_t serial_clock_rate;
	uint8_t clock_prescale_rate;
} ssp_config_t;

void spi_ssp_start(spi_target_t* target, const void* const config);
void spi_ssp1_start(spi_target_t* target, const void* const config);
void spi_ssp_stop(spi_bus_t* const bus);
void spi_ssp_transfer(spi_target_t* target, void* const data, const size_t count);
void spi_ssp_transfer_gather(spi_target_t* target,
							 const spi_transfer_t* const transfers,
							 const size_t count);
void spi_ssp_transfer_gather_partial(spi_target_t* target,
	const spi_transfer_t* const transfers, const size_t count);
void spi_ssp_transfer_data(spi_target_t* target, void* const data,
					  const size_t count);

#endif/*__SPI_SSP_H__*/
