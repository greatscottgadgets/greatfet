/*
 * This file is part of GreatFET
 */

/*
 * This is a rudimentary driver for generic SPI Flash ICs using the
 * LPC43xx's SSP0 peripheral (not quad SPIFI).
 */

#include <stdint.h>
#include <stddef.h>

#include "spiflash.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define SPIFLASH_READ_DATA    0x03
#define SPIFLASH_FAST_READ    0x0b
#define SPIFLASH_WRITE_ENABLE 0x06
#define SPIFLASH_CHIP_ERASE   0xC7
#define SPIFLASH_WRITE_STATUS 0x01
#define SPIFLASH_READ_STATUS1 0x05
#define SPIFLASH_READ_STATUS2 0x35
#define SPIFLASH_PAGE_PROGRAM 0x02
#define SPIFLASH_DEVICE_ID    0xAB
#define SPIFLASH_UNIQUE_ID    0x4B

#define SPIFLASH_STATUS_BUSY  0x01
#define SPIFLASH_STATUS_WEL   0x02

/*
 * Set up pins for GPIO and SPI control, configure SSP0 peripheral for SPI.
 * SSP0_SSEL is controlled by GPIO in order to handle various transfer lengths.
 */
void spiflash_setup(spiflash_driver_t* const drv) {
	uint8_t device_id;

	drv->target_init(drv->target);

	device_id = 0;
	while(device_id != drv->device_id)
	{
		device_id = spiflash_get_device_id(drv);
	}
}

uint8_t spiflash_get_status(spiflash_driver_t* const drv)
{
	uint8_t data[] = { SPIFLASH_READ_STATUS1, 0xFF };
	spi_bus_transfer(drv->target, data, ARRAY_SIZE(data));
	return data[1];
}

/* Release power down / Device ID */  
uint8_t spiflash_get_device_id(spiflash_driver_t* const drv)
{
	uint8_t data[] = {
		SPIFLASH_DEVICE_ID,
		0xFF, 0xFF, 0xFF, 0xFF
	};
	spi_bus_transfer(drv->target, data, ARRAY_SIZE(data));
	return data[4];
}

void spiflash_get_unique_id(spiflash_driver_t* const drv, spiflash_unique_id_t* unique_id)
{
	uint8_t data[] = {
		SPIFLASH_UNIQUE_ID,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
	spi_bus_transfer(drv->target, data, ARRAY_SIZE(data));

	for(size_t i=0; i<8; i++) {
		unique_id->id_8b[i]  = data[5+i];
	}
}

void spiflash_wait_while_busy(spiflash_driver_t* const drv)
{
	while (spiflash_get_status(drv) & SPIFLASH_STATUS_BUSY);
}

void spiflash_write_enable(spiflash_driver_t* const drv)
{
	spiflash_wait_while_busy(drv);

	uint8_t data[] = { SPIFLASH_WRITE_ENABLE };
	spi_bus_transfer(drv->target, data, ARRAY_SIZE(data));
	while (!(spiflash_get_status(drv) & SPIFLASH_STATUS_WEL));
}

void spiflash_chip_erase(spiflash_driver_t* const drv)
{
	uint8_t device_id;

	device_id = 0;
	while(device_id != drv->device_id) {
		device_id = spiflash_get_device_id(drv);
	}

	spiflash_write_enable(drv);
	spiflash_wait_while_busy(drv);

	uint8_t data[] = { SPIFLASH_CHIP_ERASE };
	spi_bus_transfer(drv->target, data, ARRAY_SIZE(data));
}

/* write up a 256 byte page or partial page */
static void spiflash_page_program(spiflash_driver_t* const drv, const uint32_t addr, const uint16_t len, uint8_t* data)
{
	/* do nothing if asked to write beyond a page boundary */
	if (((addr & 0xFF) + len) > drv->page_len)
		return;

	/* do nothing if we would overflow the flash */
	if (addr > (drv->num_bytes - len))
		return;

	spiflash_write_enable(drv);
	spiflash_wait_while_busy(drv);

	uint8_t header[] = {
		SPIFLASH_PAGE_PROGRAM,
		(addr & 0xFF0000) >> 16,
		(addr & 0xFF00) >> 8,
		addr & 0xFF
	};

	const spi_transfer_t transfers[] = {
		{ header, ARRAY_SIZE(header) },
		{ data, len }
	};

	spi_bus_transfer_gather(drv->target, transfers, ARRAY_SIZE(transfers));
}

/* write an arbitrary number of bytes */
void spiflash_program(spiflash_driver_t* const drv, uint32_t addr, uint32_t len, uint8_t* data)
{
	uint16_t first_block_len;
	uint8_t device_id;

	device_id = 0;
	spiflash_wait_while_busy(drv);
	while(device_id != drv->device_id) {
		device_id = spiflash_get_device_id(drv);
	}	
	
	/* do nothing if we would overflow the flash */
	if ((len > drv->num_bytes) || (addr > drv->num_bytes)
			|| ((addr + len) > drv->num_bytes))
		return;

	/* handle start not at page boundary */
	first_block_len = drv->page_len - (addr % drv->page_len);
	if (len < first_block_len)
		first_block_len = len;
	if (first_block_len) {
		spiflash_page_program(drv, addr, first_block_len, data);
		addr += first_block_len;
		data += first_block_len;
		len -= first_block_len;
	}

	/* one page at a time on boundaries */
	while (len >= drv->page_len) {
		spiflash_page_program(drv, addr, drv->page_len, data);
		addr += drv->page_len;
		data += drv->page_len;
		len -= drv->page_len;
	}

	/* handle end not at page boundary */
	if (len) {
		spiflash_page_program(drv, addr, len, data);
	}
}

/* write an arbitrary number of bytes */
void spiflash_read(spiflash_driver_t* const drv, uint32_t addr, uint32_t len, uint8_t* const data)
{
	/* do nothing if we would overflow the flash */
	if ((len > drv->num_bytes) || (addr > drv->num_bytes)
			|| ((addr + len) > drv->num_bytes))
		return;

	spiflash_wait_while_busy(drv);

	uint8_t header[] = {
		SPIFLASH_FAST_READ,
		(addr & 0xFF0000) >> 16,
		(addr & 0xFF00) >> 8,
		addr & 0xFF,
		0x00
	};

	const spi_transfer_t transfers[] = {
		{ header, ARRAY_SIZE(header) },
		{ data, len }
	};

	spi_bus_transfer_gather(drv->target, transfers, ARRAY_SIZE(transfers));
}

void spiflash_clear_status(spiflash_driver_t* const drv)
{
	spiflash_write_enable(drv);
	spiflash_wait_while_busy(drv);
	uint8_t data[] = { SPIFLASH_WRITE_STATUS, 0x00, 0x00 };
	spi_bus_transfer(drv->target, data, ARRAY_SIZE(data));
}

void spiflash_get_full_status(spiflash_driver_t* const drv, uint8_t* data)
{
	uint8_t cmd[] = { SPIFLASH_READ_STATUS1, 0xFF };
	spi_bus_transfer(drv->target, cmd, ARRAY_SIZE(cmd));
	data[0] = cmd[1];
	uint8_t cmd2[] = { SPIFLASH_READ_STATUS2, 0xFF };
	spi_bus_transfer(drv->target, cmd2, ARRAY_SIZE(cmd2));
	data[1] = cmd2[1];
}
