/*
 * This file is part of GreatFET
 */

// #include <drivers/usb/lpc43xx/usb_queue.h>

#include <stddef.h>
#include <greatfet_core.h>
#include <spi_bus.h>
#include <gpio_lpc.h>
#include <libopencm3/lpc43xx/scu.h>
#include <pins.h>

#include <drivers/comms.h>

#define CLASS_NUMBER_SELF (0x109)

/* SSP1 SSEL (CS) pin, used as GPIO so that we control
 * it rather than the SSP.
 */
static struct gpio_t gpio_spi_select = GPIO(0, 15);

static spi_target_t spi1_target = {
	.bus = &spi_bus_ssp1,
	.gpio_select = &gpio_spi_select,
};


void spi1_init(spi_target_t* const target) {
	/* configure SSP pins */
	scu_pinmux(SCU_SSP1_MISO, (SCU_SSP_IO | SCU_CONF_FUNCTION5));
	scu_pinmux(SCU_SSP1_MOSI, (SCU_SSP_IO | SCU_CONF_FUNCTION5));
	scu_pinmux(SCU_SSP1_SCK,  (SCU_SSP_IO | SCU_CONF_FUNCTION1));
	scu_pinmux(SCU_SSP1_SSEL, (SCU_GPIO_FAST | SCU_CONF_FUNCTION0));
	(void) target;
}

static spiflash_driver_t spi1_target_drv = {
	.target = &spi1_target,
    .target_init = spi1_init,
};

static int spi_verb_init(struct command_transaction *trans)
{
	static ssp_config_t local_spi_config;
	ssp_config_t *config;

	uint16_t scr = comms_argument_parse_uint8_t(trans);
	uint16_t cpsdvsr = comms_argument_parse_uint8_t(trans);

	config = (ssp_config_t *)&ssp1_config_spi;
	if (cpsdvsr != 0) {
		local_spi_config.data_bits = SSP_DATA_8BITS;
		local_spi_config.clock_prescale_rate = cpsdvsr;
		local_spi_config.serial_clock_rate = scr;
		config = &local_spi_config;
	}

	spi_bus_start(spi1_target_drv.target, config);
	spi1_init(spi1_target_drv.target);

	return 0;
}

static int spi_verb_transmit(struct command_transaction *trans)
{
	uint32_t write_length;
	uint8_t read_length	= comms_argument_parse_uint8_t(trans);
	uint8_t *spi_buffer = comms_argument_read_buffer(trans, -1, &write_length);

	spi_bus_transfer(&spi1_target, spi_buffer, write_length);
	comms_response_add_raw(trans, spi_buffer, read_length);

	return 0;
}

// TODO
// static int spi_verb_dump_flash(struct command_transaction *trans)
// {
// 	uint32_t addr;
// 	uint16_t value	= comms_argument_parse_uint16_t(trans);
// 	uint16_t index	= comms_argument_parse_uint16_t(trans);
// 	uint16_t length	= comms_argument_parse_uint16_t(trans);

// 	// not sure about spi_buffer here, not sure how dump flash gets called
// 	uint8_t *spi_buffer = comms_argument_read_buffer(trans, -1, &write_length);

// 	spi1_target_drv.page_len = 256;
// 	spi1_target_drv.num_pages = 8192;
// 	spi1_target_drv.num_bytes = 256*8192;
// 	spi1_target_drv.device_id = 0x14;
// 	addr = (value << 16) | index;
// 	spiflash_read(&spi1_target_drv, addr, length, spi_buffer);
// 	comms_response_add_raw(trans, spi_buffer, length);
// 	// usb_transfer_schedule_block(endpoint->in, &spi_buffer[0],
// 	// 							endpoint->setup.length, NULL, NULL);

// 	return 0;
// }

/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "init", .handler = spi_verb_init,
			.in_signature = "<HH", .out_signature = "",
			.in_param_names = "serial_clock_rate, clock_prescale_rate",
			.doc = "Initialize a SPI device" },
		{ .name = "transmit", .handler = spi_verb_transmit,
			.in_signature = "<B*X", .out_signature = "<*B",
			.in_param_names = "read_length, data", .out_param_names = "response",
			.doc = "Write to a SPI device and read response" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(spi, CLASS_NUMBER_SELF, "spi", _verbs,
		"API for SPI communication.");