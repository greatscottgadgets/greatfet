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

#define CLASS_NUMBER_SPI_FLASH (0x101)

/* Active objects referring to each of the GPIO used to talk to the SPI flash. */
static struct gpio_t gpio_spiflash_hold   = GPIO(1, 14);
static struct gpio_t gpio_spiflash_wp     = GPIO(1, 15);
static struct gpio_t gpio_spiflash_select;

/**
 * Object representing a generic SPI communication.
 */
static spi_target_t spi_target = {
	.bus         = &spi_bus_ssp0,
	.gpio_hold   = &gpio_spiflash_hold,
	.gpio_wp     = &gpio_spiflash_wp,
	.gpio_select = &gpio_spiflash_select,
};


/**
 * Object representing our communcation with the target flash.
 */
static spiflash_driver_t spi_flash_drv = {
	.target = &spi_target,
    .target_init = spiflash_target_init,
};


/**
 * Command to initialize use of the SPIFlash class / API and configure
 * how we'll talk to the SPI flash.
 */
static int spi_flash_verb_initialize(struct command_transaction *trans)
{
    uint8_t gpio_pin, gpio_port;

	// FIXME: allow use of other GPIO ports, where possible, for hold/WP
    spi_flash_drv.page_len  = comms_argument_parse_uint16_t(trans);
    spi_flash_drv.num_pages = comms_argument_parse_uint16_t(trans);
    spi_flash_drv.num_bytes = comms_argument_parse_uint32_t(trans);
    gpio_port               = comms_argument_parse_uint8_t(trans);
    gpio_pin                = comms_argument_parse_uint8_t(trans);
    spi_flash_drv.device_id = comms_argument_parse_uint8_t(trans);

	// Apply our GPIO settings.
    GPIO_SET(gpio_spiflash_select, gpio_port, gpio_pin);

    spi_bus_start(spi_flash_drv.target, &ssp_config_spi);
    spiflash_setup(&spi_flash_drv);
    return 0;
}


/**
 * Command to erase the full flash chip.
 */
static int spi_flash_verb_full_erase(struct command_transaction *trans)
{
    (void)trans;
    spiflash_chip_erase(&spi_flash_drv);
	return 0;
}


/**
 * Command to write a page to the relevant flash chip.
 */
static int spi_flash_verb_write_page(struct command_transaction *trans)
{
    uint32_t length;
	uint32_t address = comms_argument_parse_uint32_t(trans);
    void *data_to_write = comms_argument_read_buffer(trans, -1, &length);

    // Ensure we have data.
    if (!data_to_write) {
        pr_error("error: recieved invalid firmware write request (no data)!");
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
static int spi_flash_verb_read_page(struct command_transaction *trans)
{
	uint32_t address = comms_argument_parse_uint32_t(trans);
    void *target_buffer = comms_response_reserve_space(trans, spi_flash_drv.page_len);

    // Ensure we have a buffer to put our result into.
    if (!target_buffer) {
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
        pr_warning("firmware: rejecting read that extends past the end of flash flash! (%d > %d)\n", 
                address + spi_flash_drv.page_len, spi_flash_drv.num_bytes);
        return EINVAL;
    }

    spiflash_read(&spi_flash_drv, address, spi_flash_drv.page_len, target_buffer);
    return 0;
}

/**
 * Verbs for the firmware API.
 */
static struct comms_verb spi_flash_verbs[] = {
		{ .verb_number = 0x0, .name = "initialize", .handler = spi_flash_verb_initialize,
            .in_signature = "<HHIBBB", .out_signature = "",
            .in_param_names = "page_len, num_pages, num_bytes, gpio_port, gpio_pin, expected_device_id",
            .doc = "Sets up the board to have its spi_flash programmed." },
		{ .verb_number = 0x1, .name = "full_erase", .handler = spi_flash_verb_full_erase,
            .in_signature = "", .out_signature	= "", .doc = "Erases the entire spi_flash flash chip." },
        /* TODO: implement
		{ .verb_number = 0x2, .name = "page_erase", .handler = spi_flash_verb_erase_page,
            .in_signature = "<I", .out_signature = "", .in_param_names = "address",
            .doc = "Erases the page with the provided address on the fw flash." },
        */
		{ .verb_number = 0x3, .name = "write_page", .handler = spi_flash_verb_write_page,
            .in_signature = "<I*X", .out_signature = "", .in_param_names = "address, data",
            .doc = "Writes the provided data to a single spi_flash flash page." },
		{ .verb_number = 0x4, .name = "read_page",	.handler = spi_flash_verb_read_page,
            .in_signature = "<I", .out_signature = "<*X", .in_param_names = "address", .out_param_names = "data",
            .doc = "Returns the contents of the flash page at the given address." },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(spi_flash, CLASS_NUMBER_SPI_FLASH, "spi_flash", spi_flash_verbs,
        "API that allows use of the GreatFET to program a SPI flash.");

