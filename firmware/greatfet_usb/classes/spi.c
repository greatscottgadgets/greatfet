/*
 * This file is part of GreatFET
 */

#include <errno.h>
#include <stddef.h>
#include <spi_bus.h>

// FIXME: replace these with libgreat drivers!
#include <gpio_lpc.h>
#include <libopencm3/lpc43xx/ssp.h>

#include <pins.h>
#include <greatfet_core.h>

#include <drivers/scu.h>
#include <drivers/comms.h>

#define CLASS_NUMBER_SELF (0x109)

// Forward declarations.
static void spi1_init(spi_target_t* const target);


/* SSP1 SSEL (CS) pin, used as GPIO so that we control
 * it rather than the SSP.
 */
static struct gpio_t gpio_spi_select = GPIO(0, 15);

static spi_target_t spi1_target = {
	.bus = &spi_bus_ssp1,
	.gpio_select = &gpio_spi_select,
};

static platform_scu_pin_configuration_t spi_pin_configuration = {
	.pull_resistors = SCU_NO_PULL,
	.use_fast_slew = true,
	.input_buffer_enabled = true,
	.disable_glitch_filter = true
};

/**
 * TODO: abstract these into a driver function?
 */
static scu_function_mapping_t primary_spi_scu_configurations[] = {
	{ .group = 1, .pin = 3,  .function = 5 }, // MISO
	{ .group = 1, .pin = 4,  .function = 5 }, // MOSI
	{ .group = 1, .pin = 19, .function = 1 }, // SCK
	{ .group = 1, .pin = 20, .function = 0 }, // SSEL
};


static spiflash_driver_t spi1_target_drv = {
	.target = &spi1_target,
    .target_init = spi1_init,
};


/**
 * Configure the pins used for SSPI1.
 */
static void spi1_configure_pins(void)
{
	// Multiplex each of the SSP1/SPI functions onto their respective pins.
	for (unsigned i = 0; i < ARRAY_SIZE(primary_spi_scu_configurations); ++i) {
		scu_function_mapping_t *mapping = &primary_spi_scu_configurations[i];
		platform_scu_apply_mapping(*mapping, spi_pin_configuration);
	}
}

static void spi1_init(spi_target_t* const target)
{
	(void)target;
	spi1_configure_pins();
}

static int spi_verb_init(struct command_transaction *trans)
{
	static ssp_config_t local_spi_config;
	ssp_config_t *config;

	uint16_t scr = comms_argument_parse_uint16_t(trans);
	uint16_t cpsdvsr = comms_argument_parse_uint16_t(trans);

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


static int spi_verb_clock_data(struct command_transaction *trans)
{
	uint32_t write_length;
	uint8_t read_length	= comms_argument_parse_uint8_t(trans);
	uint8_t *spi_buffer = comms_argument_read_buffer(trans, -1, &write_length);

	spi_bus_transfer_data(&spi1_target, spi_buffer, write_length);
	comms_response_add_raw(trans, spi_buffer, read_length);

	return 0;
}


static int spi_verb_enable_drive(struct command_transaction *trans)
{
	bool enable_drive = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// If we're setting high impedance, switch the SSP bus into slave mode,
	// and then turn on the slave-output-disable. This turns all of our pins
	// to input mode without us having to change our pinmuxing.
	if (enable_drive) {
		SSP1_CR1 &= ~(SSP_SLAVE | SSP_SLAVE_OUT_DISABLE);
	} else {
		SSP1_CR1 |=  (SSP_SLAVE | SSP_SLAVE_OUT_DISABLE);
	}

	return 0;
}


static int spi_verb_set_clock_polarity_and_phase(struct command_transaction *trans)
{
	uint8_t spi_mode = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	uint32_t masked_cr0_value = (SSP1_CR0 & ~SSP_CPOL_1_CPHA_1);
	SSP1_CR0 = masked_cr0_value | (spi_mode << 6);

	return 0;
}



/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {

		// Configuration.
		{ .name = "init", .handler = spi_verb_init,
			.in_signature = "<HH", .out_signature = "",
			.in_param_names = "serial_clock_rate, clock_prescale_rate",
			.doc = "Initialize a SPI device" },
		{ .name = "set_clock_polarity_and_phase", .handler = spi_verb_set_clock_polarity_and_phase,
			.in_signature = "<B", .out_signature = "", .in_param_names = "spi_mode",
			.doc = "Applies a standard SPI mode to set the polarity and phase." },

		// Tx/Rx
		{ .name = "transmit", .handler = spi_verb_transmit,
			.in_signature = "<B*X", .out_signature = "<*B",
			.in_param_names = "read_length, data", .out_param_names = "response",
			.doc = "Write to a SPI device and read response" },
		{ .name = "clock_data", .handler = spi_verb_clock_data,
			.in_signature = "<B*X", .out_signature = "<*B",
			.in_param_names = "read_length, data", .out_param_names = "response",
			.doc = "Clock data out and in; but don't change the chip select." },

		// Advanced control.
		{ .name = "enable_drive", .handler = spi_verb_clock_data,
			.in_signature = "<?", .out_signature = "", .in_param_names = "enable_drive",
			.doc = "If enable is false, the SPI pins will be tri-stated; if true, pins will be set to normal drive." },


		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(spi, CLASS_NUMBER_SELF, "spi", _verbs,
		"API for SPI communication.");
