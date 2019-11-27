/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include <drivers/gpio.h>

#include <debug.h>

#include <stddef.h>
#include <errno.h>

#include <greatfet_core.h>
#include <spiflash.h>
#include <spiflash_target.h>
#include <gpio_lpc.h>
#include <pins.h>


#define CLASS_NUMBER_SPI_FLASH (0x101)

/* Active objects referring to each of the GPIO used to talk to the SPI flash. */
static struct gpio_t gpio_spiflash_hold   = GPIO(1, 14);
static struct gpio_t gpio_spiflash_wp     = GPIO(1, 15);
static struct gpio_t gpio_spiflash_select;


/**
 * Object representing a generic SPI communication.
 */
static spi_target_t spi_target = {
	.bus         = &spi_bus_ssp1,
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
    uint8_t cs_port, cs_pin;

	// FIXME: allow use of other GPIO ports, where possible, for hold/WP
    spi_flash_drv.page_len  = comms_argument_parse_uint32_t(trans);
    spi_flash_drv.num_pages = comms_argument_parse_uint32_t(trans);
    spi_flash_drv.num_bytes = comms_argument_parse_uint32_t(trans);
    cs_port                 = comms_argument_parse_uint8_t(trans);
    cs_pin                  = comms_argument_parse_uint8_t(trans);
    spi_flash_drv.device_id = comms_argument_parse_uint8_t(trans);

	// Apply our GPIO settings.
	gpio_configure_pinmux_and_resistors(gpio_pin(cs_port, cs_pin), RESISTOR_CONFIG_NO_PULL);
	// FIXME: use libgreat's gpio dirver rather than libopencm3's
    GPIO_SET(gpio_spiflash_select, cs_port, cs_pin);

    spi_bus_start(spi_flash_drv.target, &ssp_config_spi);
    return spiflash_setup(&spi_flash_drv);
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


/*?*
 * Command to write a page to the relevant flash chip.
 */
static int spi_flash_verb_write_page(struct command_transaction *trans)
{
    uint32_t length;
	uint32_t address = comms_argument_parse_uint32_t(trans);
    void *data_to_write = comms_argument_read_buffer(trans, -1, &length);

    // Ensure we have data.
    if (!data_to_write) {
        pr_error("error: received invalid firmware write request (no data)!");
        return EINVAL;
    }

    // Validate our write spans.
    if (length > spi_flash_drv.page_len) {
        pr_warning("spi_flash: rejecting write of more than page length! (%d > %d)\n",
                length, spi_flash_drv.page_len);
        return EINVAL;
    }
    if (address > spi_flash_drv.num_bytes) {
        pr_warning("spi_flash: rejecting write that's larger than our flash! (%d > %d)\n",
                length, spi_flash_drv.num_bytes);
        return EINVAL;
    }
    if ((address + length) > spi_flash_drv.num_bytes) {
        pr_warning("spi_flash: rejecting write that extends past the end of flash! (%d > %d)\n",
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
        pr_warning("spi_flash: rejecting read that's larger than our flash! (%d > %d)\n",
                address, spi_flash_drv.num_bytes);
        return EINVAL;
    }
    if ((address + spi_flash_drv.page_len) > spi_flash_drv.num_bytes) {
        pr_warning("spi_flash: rejecting read that extends past the end of flash flash! (%d > %d)\n",
                address + spi_flash_drv.page_len, spi_flash_drv.num_bytes);
        return EINVAL;
    }

    spiflash_read(&spi_flash_drv, address, spi_flash_drv.page_len, target_buffer);
    return 0;
}


/**
 * Command to read a page from the relevant flash chip.
 */
static int spi_flash_verb_query_jedec_id(struct command_transaction *trans)
{
	spi_flash_jedec_id_t id;

	// Read the JEDEC ID.
	spiflash_read_jedec_id(&spi_flash_drv, &id);

	// ... and return its component pieces.
	comms_response_add_uint8_t(trans,  id.manufacturer);
	comms_response_add_uint16_t(trans, id.device_id);
	comms_response_add_uint8_t(trans,  id.capacity);

	return 0;
}



/**
 * Command to read device toplogy information via JEDEC Serial Flash Discoverable Paramters
 * (SDFP) Protocol.
 */
static int spi_flash_verb_query_topology(struct command_transaction *trans)
{
	uint32_t size_in_bits, size_in_bytes;
	uint32_t page_size, page_count;
	spi_flash_sfdp_info_t info;
	int rc;

	// Read the device info via SPDF.
	rc = spiflash_read_sfdp_info(&spi_flash_drv, &info);
	if (rc) {
		pr_warning("spi_flash: SFDP appears to be unsupported\n");
		return rc;
	}

	// Capture the memory density from the SFDP information...
	if (info.memory_density_is_order) {
		// If we have an order, compute 2^N.
		size_in_bits = (1 << info.memory_density);
	} else {
		// Otherwise, we have a maximum bit number, so add one to get a capacity.
		size_in_bits = info.memory_density + 1;
	}

	// ... and convert it to a usable format. :)
	size_in_bytes = size_in_bits / 8;

	// Figure out the page size, in bytes.
	page_size = (1 << info.page_size_order);

	// ... and figure out the page count, rounding up to the next whole page.
	page_count = (size_in_bytes + page_size - 1) / page_size;

	// Return our topology info.
	comms_response_add_uint32_t(trans, page_size);
	comms_response_add_uint32_t(trans, page_count);
	comms_response_add_uint32_t(trans, size_in_bytes);

	return 0;
}



/**
 * Verbs for the firmware API.
 */
static struct comms_verb spi_flash_verbs[] = {

		// Control and initialization.
		{ .name = "initialize", .handler = spi_flash_verb_initialize,
            .in_signature = "<IIIBBB", .out_signature = "",
            .in_param_names = "page_len, num_pages, num_bytes, gpio_port, gpio_pin, expected_device_id",
            .doc = "Sets up the board to program an external SPI flash." },
		{ .name = "full_erase", .handler = spi_flash_verb_full_erase,
            .in_signature = "", .out_signature	= "", .doc = "Erases the entire spi_flash flash chip." },
        /* TODO: implement
		{ .verb_number = 0x2, .name = "page_erase", .handler = spi_flash_verb_erase_page,
            .in_signature = "<I", .out_signature = "", .in_param_names = "address",
            .doc = "Erases the page with the provided address on the fw flash." },
        */
		{ .name = "write_page", .handler = spi_flash_verb_write_page,
            .in_signature = "<I*X", .out_signature = "", .in_param_names = "address, data",
            .doc = "Writes the provided data to a single spi_flash flash page." },
		{ .name = "read_page",	.handler = spi_flash_verb_read_page,
            .in_signature = "<I", .out_signature = "<*X", .in_param_names = "address", .out_param_names = "data",
            .doc = "Returns the contents of the flash page at the given address." },


		//
		// Metdata query functions.
		//
		{ .name = "query_device_id", .handler = spi_flash_verb_query_jedec_id,
            .in_signature = "<", .out_signature = "<BHB",
            .in_param_names = "", .out_param_names = "manufacturer_id, device_id, capacity_code",
            .doc =
				"Reads the target SPI flash's JEDEC ID.\n\n"
				"Returns invalid data if the device does not support the field."
		},
		{ .name = "query_topology", .handler = spi_flash_verb_query_topology,
            .in_signature = "<", .out_signature = "<III",
            .in_param_names = "", .out_param_names = "page_length, page_count, byte_length",
            .doc =
				"Attempts to read information about the device's 'shape' using SPDF.\n\n"
				"Raises an exception if the device does not support SPDF."
		},


		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(spi_flash, CLASS_NUMBER_SPI_FLASH, "spi_flash", spi_flash_verbs,
        "API that allows use of the GreatFET to program a SPI flash.");

