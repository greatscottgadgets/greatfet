/*
 * This file is part of GreatFET
 */

#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

#include <stdint.h>
#include <stddef.h>
#include <toolchain.h>

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



/**
 * Paramter header for the Basic Flash Parameters table.
 */
typedef struct ATTR_PACKED {

	// First DWORD: basic metadata for the relevant table.
	uint8_t length;
	uint8_t minor_rev;
	uint8_t major_rev;
	uint8_t id_number;

	// Second DWORD: location of the relevant table.
	struct {
		uint32_t paramter_table_address : 24;
		uint32_t always_ff              : 8;
	};


} spi_flash_spdf_parameter_header_t;


/**
 * Structure describing the layout of the start of the SPDF address space.
 */
typedef struct ATTR_PACKED {

	// Magic letters 'SFDP'.
	char magic[4];

	// SFDP revision number.
	uint8_t minor_rev;
	uint8_t major_rev;

	// Parameter headers to follow.
	uint8_t number_of_parameter_headers;

	// Information on how to access the SFDP data.
	uint8_t access_protocol;

	// Information about the basic information table,.
	spi_flash_spdf_parameter_header_t basic_flash_parameter_info;

	// Any additional parmeter headers identified by the number_of_parameter_headers field, if read.
	spi_flash_spdf_parameter_header_t additional_parameter_headers[];

} spi_flash_spdf_start_of_memory_t;


/**
 * Structure that breaks down the SPDF Basic Information table.
 * TODO: break the raw uint32_t variables into bits
 */
typedef struct ATTR_PACKED {

	// TODO: implement interpreting this
	// 1st DWORD
	uint32_t basic_device_info;

	// Size of the flash in bytes.
	// 2nd DWORD
	struct {
		uint32_t memory_density            : 31;
		uint32_t memory_density_is_order   :  1;
	};

	// 3-7th DWORD
	uint32_t fast_read_info[5];

	// 8th DWORD
	struct {
		uint32_t sector_type1_size         : 8;
		uint32_t sector_type1_erase_opcode : 8;
		uint32_t sector_type2_size         : 8;
		uint32_t sector_type2_erase_opcode : 8;
	};

	// 9th DWORD
	struct {
		uint32_t sector_type3_size         : 8;
		uint32_t sector_type3_erase_opcode : 8;
		uint32_t sector_type4_size         : 8;
		uint32_t sector_type5_erase_opcode : 8;
	};

	// 10th DWORD
	uint32_t erase_time_info;

	// 11th DWORD
	struct {
		uint32_t typical_to_max_program_multiplier     : 4; // 0, 1, 2, 3
		uint32_t page_size_order                       : 4; // 4, 5, 6, 7

		uint32_t page_program_typical_time             : 5; // 8, 9, 10, 11, 12
		uint32_t page_program_is_in_64us_units         : 1; // 13

		uint32_t byte_program_typical_time             : 4; // 14, 15, 16, 17
		uint32_t byte_program_is_in_8us_units          : 1; // 18

		uint32_t byte_program_time_per_additional_byte : 4; // 19, 20, 21, 22
		uint32_t additional_byte_time_is_in_8us_units  : 1; // 23

		uint32_t chip_erase_typical_time               : 5; // 24, 25, 26, 27, 28
		uint32_t chip_erase_time_units                 : 2; // 29, 30
		uint32_t                                       : 1; // 31
	};

	// 12 - 20th DWORDS
	// TODO: fill out
	uint32_t not_yet_implemented[9];

} spi_flash_sfdp_info_t;


typedef struct ATTR_PACKED {
	uint8_t manufacturer;
	uint16_t device_id;
	uint8_t capacity;


} spi_flash_jedec_id_t;


struct spiflash_driver_t;
typedef struct spiflash_driver_t spiflash_driver_t;

int spiflash_setup(spiflash_driver_t* const drv);
void spiflash_chip_erase(spiflash_driver_t* const drv);
void spiflash_program(spiflash_driver_t* const drv, uint32_t addr, uint32_t len, uint8_t* data);
uint8_t spiflash_get_device_id(spiflash_driver_t* const drv);
void spiflash_get_unique_id(spiflash_driver_t* const drv, spiflash_unique_id_t* unique_id);
void spiflash_read(spiflash_driver_t* const drv, uint32_t addr, uint32_t len, uint8_t* const data);


/**
 * Reads the JEDEC-specified device ID from the target device.
 */
void spiflash_read_jedec_id(spiflash_driver_t* const drv, spi_flash_jedec_id_t *id);


/**
 * Performs a raw SPDF read.
 *
 * @param drv The SPI flash driver to perform the given read.
 * @param spdf_address The SPDF address to read from.
 * @param buffer A buffer to store the results of the read.
 * @param length The total data length to read.
 */
void spiflash_read_sfdp_data(spiflash_driver_t *const drv, uint32_t sfdp_address, void *buffer, size_t length);


/**
 * Attempts to read information about the provided SPI flash using SPDF.
 *
 * @param info The object to populate.
 */
int spiflash_read_sfdp_info(spiflash_driver_t *const drv, spi_flash_sfdp_info_t *info);

#endif //__SPIFLASH_H__
