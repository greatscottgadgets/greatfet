/*
 * This file is part of GreatFET
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
