/*
 * This file is part of GreatFET
 */

/*
 * This is a rudimentary driver for generic SPI Flash ICs using the
 * LPC43xx's SSP0 peripheral (not quad SPIFI).
 *
 * FIXME: move this into libgreat!
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <errno.h>


#include <toolchain.h>
#include <debug.h>


#include "spiflash.h"

enum {
	SPIFLASH_READ_DATA     = 0x03,
	SPIFLASH_FAST_READ     = 0x0b,
	SPIFLASH_WRITE_ENABLE  = 0x06,
	SPIFLASH_CHIP_ERASE    = 0xC7,
	SPIFLASH_WRITE_STATUS  = 0x01,
	SPIFLASH_READ_STATUS1  = 0x05,
	SPIFLASH_READ_STATUS2  = 0x35,
	SPIFLASH_PAGE_PROGRAM  = 0x02,
	SPIFLASH_JEDEC_ID      = 0x9F,
	SPIFLASH_DEVICE_ID     = 0xAB,
	SPIFLASH_UNIQUE_ID     = 0x4B,

	SPIFLASH_READ_SFDP     = 0x5A
};

enum {
	SPIFLASH_STATUS_BUSY   = 0x01,
	SPIFLASH_STATUS_WEL    = 0x02
};


/*
 * Set up pins for GPIO and SPI control, configure SSP0 peripheral for SPI.
 * SSP0_SSEL is controlled by GPIO in order to handle various transfer lengths.
 *
 * Checks for an expected device ID, unless the expected device ID is 0.
 */
int spiflash_setup(spiflash_driver_t* const drv)
{
	const uint32_t timeout = (100 * 1024 * 1024); // 100ms

	uint32_t timebase = get_time();
	uint8_t device_id;

	drv->target_init(drv->target);

	device_id = 0;
	while(device_id != drv->device_id)
	{
		if (get_time_since(timebase) > timeout) {
			return ETIMEDOUT;
		}

		device_id = spiflash_get_device_id(drv);
	}

	return 0;
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


/**
 * Reads the JEDEC-specified device ID from the target device.
 */
void spiflash_read_jedec_id(spiflash_driver_t* const drv, spi_flash_jedec_id_t *id)
{
	uint8_t data[] = { SPIFLASH_JEDEC_ID };

	// Build the scatter-gather transfer that will populate the JEDEC id.
	const spi_transfer_t transfers[] = {
		{ data, sizeof(data) },
		{ id, sizeof(*id) }
	};

	spi_bus_transfer_gather(drv->target, transfers, ARRAY_SIZE(transfers));
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
		{ header, sizeof(header) },
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

//
// TODO: should some of these be shared with I2C flash data?
//


/**
 * Performs a raw SPDF read.
 *
 * @param drv The SPI flash driver to perform the given read.
 * @param spdf_address The SPDF address to read from.
 * @param buffer A buffer to store the results of the read.
 * @param length The total data length to read.
 */
void spiflash_read_sfdp_data(spiflash_driver_t *const drv, uint32_t sfdp_address, void *buffer, size_t length)
{
	uint8_t command[] = {
		SPIFLASH_READ_SFDP,            // SFDP read command
		(sfdp_address >> 16) & 0xff,   // Address, in three chunks.
		(sfdp_address >>  8) & 0xff,
		(sfdp_address >>  0) & 0xff,
		0x00,                          // Dummy byte, per the SFDP standard.
	};

	// Clear out the buffer, so we're sending zeroes rather than uninitialized ram.
	// This is very important, so the SPI flash can't spy on us.~ (... spi on us?)
	// [Author's note: assign git blame for the last line to someone else.]
	memset(buffer, 0, length);

	// Build a scatter-gather list that includes our command and our user buffer.
	// This allows us to send the command and read the response without de-asserting the chip-select.
	const spi_transfer_t transfer_sg_list[] = {
		{ command, ARRAY_SIZE(command) },
		{ buffer, length }
	};

	// Perform the actual SPI data exchange.
	spi_bus_transfer_gather(drv->target, transfer_sg_list, ARRAY_SIZE(transfer_sg_list));
}


/**
 * Attempts to read information about the provided SPI flash using SPDF.
 *
 * @param info The object to populate.
 */
int spiflash_read_sfdp_info(spiflash_driver_t *const drv, spi_flash_sfdp_info_t *info)
{
	spi_flash_spdf_start_of_memory_t start_of_memory;
	int rc;

	// Read the start of the SPDF address space into our start-of-memory variable.
	spiflash_read_sfdp_data(drv, 0, &start_of_memory, sizeof(start_of_memory));

	// Validate that we're actually reading SFDP data.
	rc = memcmp(&start_of_memory.magic, "SFDP", sizeof(start_of_memory.magic));
	if (rc) {
		pr_info("spi_flash: given flash does not support SPDF! \n");
		return ENOTSUP;
	}

	// If we are, we can use its contents to request the basic information table. Do so!
	spiflash_read_sfdp_data(drv, start_of_memory.basic_flash_parameter_info.paramter_table_address,
			info, sizeof(*info));
	return 0;
}
