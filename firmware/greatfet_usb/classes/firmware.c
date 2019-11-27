/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include <debug.h>

#include <stddef.h>
#include <errno.h>

#include <greatfet_core.h>
#include <spiflash.h>
#include <spiflash_target.h>
#include <gpio_lpc.h>
#include <pins.h>

#define CLASS_NUMBER_FIRMWARE (0x1)

/**
 * Configuration for the SPI bus we used to
 * communicate with our onboard flash.
 */
static spi_target_t spi_target = {
	.bus         = &spi_bus_ssp0,
	.gpio_hold   = &gpio_onboard_flash_hold,
	.gpio_wp     = &gpio_onboard_flash_select,
	.gpio_select = &gpio_onboard_flash_select,
};

/**
 * Configuration for the SPI flash driver we used to
 * communicate with our onboard flash.
 */
static spiflash_driver_t spi_flash_drv = {
	.target      = &spi_target,
    .target_init = spiflash_target_init,
    .page_len    = ONBOARD_FLASH_PAGE_LEN,
    .num_pages   = ONBOARD_FLASH_NUM_PAGES,
    .num_bytes   = ONBOARD_FLASH_NUM_BYTES,
    .device_id   = ONBOARD_FLASH_DEVICE_ID
};


/**
 * Command to initialize use of the SPIFlash class / API and configure
 * how we'll talk to the SPI flash.
 */
int firmware_verb_initialize(struct command_transaction *trans)
{
	// Set up the flash to be written.
    spi_bus_start(spi_flash_drv.target, &ssp_config_spi);
    spiflash_setup(&spi_flash_drv);

	// Return information about the device.
	comms_response_add_uint32_t(trans, spi_flash_drv.page_len);
	comms_response_add_uint32_t(trans, spi_flash_drv.num_bytes);
    return 0;
}

/**
 * Command to erase the full flash chip.
 */
int firmware_verb_full_erase(struct command_transaction *trans)
{
    (void)trans;
    spiflash_chip_erase(&spi_flash_drv);
	return 0;
}


/**
 * Command to write a page to the relevant flash chip.
 */
int firmware_verb_write_page(struct command_transaction *trans)
{
    uint32_t length;
	uint32_t address = comms_argument_parse_uint32_t(trans);
    void *data_to_write = comms_argument_read_buffer(trans, -1, &length);

    // Ensure we have data.
    if (!data_to_write || !comms_transaction_okay(trans)) {
        pr_error("error: received invalid firmware write request (insufficient data)!");
        return EINVAL;
    }

    // Validate our write spans.
    if (length > spi_flash_drv.page_len) {
        pr_warning("firmware: rejecting write of more than page length! (%d > %d)\n",
                length, spi_flash_drv.page_len);
        return EINVAL;
    }
    if (address > spi_flash_drv.num_bytes) {
        pr_warning("firmware: rejecting write that's larger than our flash! (%d > %d)\n",
                length, spi_flash_drv.num_bytes);
        return EINVAL;
    }
    if ((address + length) > spi_flash_drv.num_bytes) {
        pr_warning("firmware: rejecting write that extends past the end of flash! (%d > %d)\n",
                address + length, spi_flash_drv.num_bytes);
        return EINVAL;
    }

    spiflash_program(&spi_flash_drv, address, length, data_to_write);
    return 0;
}


/**
 * Command to read a page from the relevant flash chip.
 */
int firmware_verb_read_page(struct command_transaction *trans)
{
	uint32_t address = comms_argument_parse_uint32_t(trans);
    void *target_buffer = comms_response_reserve_space(trans, spi_flash_drv.page_len);

    // Ensure we have a buffer to put our result into.
    if (!target_buffer || !comms_transaction_okay(trans)) {
        pr_error("error: firmware: could not find enough space to read a page into\n");
        return EINVAL;
    }

    // Validate our read spans.
    if (address > spi_flash_drv.num_bytes) {
        pr_warning("firmware: rejecting read that's larger than our flash! (%d > %d)\n",
                address, spi_flash_drv.num_bytes);
        return EINVAL;
    }
    if ((address + spi_flash_drv.page_len) > spi_flash_drv.num_bytes) {
        pr_warning("firmware: rejecting read that extends past the end of flash! (%d > %d)\n",
                address + spi_flash_drv.page_len, spi_flash_drv.num_bytes);
        return EINVAL;
    }

    spiflash_read(&spi_flash_drv, address, spi_flash_drv.page_len, target_buffer);
    return 0;
}

