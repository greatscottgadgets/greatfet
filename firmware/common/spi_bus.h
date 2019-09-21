/*
 * This file is part of GreatFET
 */

#ifndef __SPI_BUS_H__
#define __SPI_BUS_H__

#include <stddef.h>
#include "gpio.h"

typedef struct {
	void* const data;
	const size_t count;
} spi_transfer_t;

struct spi_bus_t;
typedef struct spi_bus_t spi_bus_t;

struct spi_target_t {
	spi_bus_t* bus;
	gpio_t gpio_hold;
	gpio_t gpio_wp;
	gpio_t gpio_select;
};

struct spi_target_t;
typedef struct spi_target_t spi_target_t;

struct spi_bus_t {
	void* const obj;
	const void* config;
	void (*start)(spi_target_t* target, const void* const config);
	void (*stop)(spi_bus_t* const bus);
	void (*transfer)(spi_target_t* target, void* const data, const size_t count);
	void (*transfer_data)(spi_target_t* target, void* const data, const size_t count);
	void (*transfer_gather)(spi_target_t* target, const spi_transfer_t* const transfers, const size_t count);
	void (*transfer_gather_partial)(spi_target_t* target, const spi_transfer_t* const transfers, const size_t count);
};

void spi_bus_start(spi_target_t* target, const void* const config);
void spi_bus_stop(spi_bus_t* const bus);
void spi_bus_transfer(spi_target_t* target, void* const data, const size_t count);
void spi_bus_transfer_gather(spi_target_t* target, const spi_transfer_t* const transfers, const size_t count);
void spi_bus_transfer_data(spi_target_t* target, void* const data, const size_t count);
void spi_bus_transfer_gather_partial(spi_target_t* target, const spi_transfer_t* const transfers, const size_t count);


#endif/*__SPI_BUS_H__*/
