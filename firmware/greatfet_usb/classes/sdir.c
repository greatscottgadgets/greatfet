/*
 * This file is part of GreatFET
 *
 * Code for using the Gladiolus SDIR neighbor.
 */

#include <debug.h>

#include <drivers/comms.h>
#include <drivers/gpio.h>
#include <drivers/dac/ad970x.h>
#include <drivers/sgpio.h>

#include "../pin_manager.h"
#include "../usb_streaming.h"

#define CLASS_NUMBER_SELF (0x10E)

// Store the state of the SDIR initialization.
static bool sdir_initialized = false;

enum {
	SDIR_DEFAULT_FREQUENCY = 10200000 // 10.2 MHz
};


/**
 * Data capture pins for the Gladiolus ADC.
 */
static sgpio_pin_configuration_t sdir_adc_data_pins[] = {
	{ .sgpio_pin = 0,  .scu_group = 0, .scu_pin =  0, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 1,  .scu_group = 0, .scu_pin =  1, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 2,  .scu_group = 1, .scu_pin = 15, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 3,  .scu_group = 1, .scu_pin = 16, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 4,  .scu_group = 6, .scu_pin =  3, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 5,  .scu_group = 6, .scu_pin =  6, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 6,  .scu_group = 2, .scu_pin =  2, .pull_resistors = SCU_NO_PULL},
	{ .sgpio_pin = 7,  .scu_group = 1, .scu_pin =  0, .pull_resistors = SCU_NO_PULL},
};

/**
 * Clock generation pin for the ADC clock.
 */
static sgpio_pin_configuration_t sdir_adc_clk_pin =
	{ .sgpio_pin = 8,  .scu_group = 9, .scu_pin =  6, .pull_resistors = SCU_NO_PULL};

/**
 * Definition of a set of logic analyzer functions.
 */
static sgpio_function_t sdir_functions[] = {
	{
		.enabled                 = true,

		// We're observing only; and not generating a pattern.
		.mode                    = SGPIO_MODE_STREAM_DATA_IN,

		// Bind each of the lower eight pins to their proper places,
		// and by deafault sample the eight of them.
		.pin_configurations      = sdir_adc_data_pins,
		.bus_width               = ARRAY_SIZE(sdir_adc_data_pins),

		// Shift at the default logic analyzer frequency, unless configured otherwise.
		.shift_clock_source      = SGPIO_CLOCK_SOURCE_COUNTER,
		.shift_clock_frequency   = SDIR_DEFAULT_FREQUENCY,
		.shift_clock_qualifier   = SGPIO_ALWAYS_SHIFT_ON_SHIFT_CLOCK,

		// Capture our data into the USB bulk buffer, all ready to be sent up to the host.
		.buffer                  = usb_bulk_buffer,
		.buffer_order            = 15, // 16384 * 2 (size of the USB streaming buffer)

		// Output the shift clock on SGPIO8.
		.shift_clock_output      = &sdir_adc_clk_pin
	},
};

/**
 * Logic analyzer configuration using SGPIO.
 */
static sgpio_t sdir  = {
	.functions      = sdir_functions,
	.function_count = ARRAY_SIZE(sdir_functions),
};



/**
 * State of the DAC used for SDIR transmit.
 *
 * FIXME: use a neighbor configuration API to figure these out!
 */
static ad970x_t tx_dac = {

	// Port configuration.
 	.gpio_cs   = {2,  9},
	.gpio_sck  = {2, 11},
	.gpio_data = {2, 12},
	.gpio_mode = {2, 14},
};


// GPIO pins that control our parallel ADC.
static gpio_pin_t adc_powerdown  = {5, 3};
static gpio_pin_t adc_amplifier_enable  = {5, 5};


/**
 * Configures a given GPIO port/pin to be used for SDIR purposes.
 */
static int set_up_sdir_gpio(gpio_pin_t pin)
{
	int rc;
	uint8_t scu_group, scu_pin;

	// Identify the SCU group and pin for the relevant GPIO pin.
	scu_group = gpio_get_group_number(pin);
	scu_pin   = gpio_get_pin_number(pin);

	// If this port/pin doesn't correspond to a valid physical pin,
	// fail out.
	if ((scu_group == 0xff) || (scu_pin == 0xff)) {
		return EINVAL;
	}

	// Reserve the pin for this class. If we can't get a hold of the pin, fail out.
	if (!pin_ensure_reservation(scu_group, scu_pin, CLASS_NUMBER_SELF)) {
		pr_warning("sdir: couldn't reserve busy pin GPIO%d[%d]!\n", pin.port, pin.pin);
		return EBUSY;
	}

	// Configure the pin to be used as a GPIO in the SCU.
	rc = gpio_configure_pinmux(pin);
	if (rc) {
		pr_warning("sdir: couldn't configure pinmux for GPIO%d[%d]!\n", pin.port, pin.pin);
		return rc;
	}

	return 0;
}


/**
 * Configures a given GPIO port/pin to be used for SDIR purposes.
 */
static int tear_down_sdir_gpio(gpio_pin_t pin)
{
	uint8_t scu_group, scu_pin;

	// Identify the SCU group and pin for the relevant GPIO pin.
	scu_group = gpio_get_group_number(pin);
	scu_pin   = gpio_get_pin_number(pin);

	// If this port/pin doesn't correspond to a valid physical pin,
	// fail out.
	if ((scu_group == 0xff) || (scu_pin == 0xff)) {
		return EINVAL;
	}

    // Place the input pin back into a high-Z state by disconnecting
    // its output driver.  TODO: should this be done in the SCU, as well?
    gpio_set_pin_direction(pin, false);

    // Finally, release the relevant reservation.
    return pin_release_reservation(scu_group, scu_pin);
}


/**
 * Power on the receive path's ADC and front-end amplifier.
 */
 // TODO: make initial amplifier state configurable?
static void sdir_power_up_adc(void)
{
	gpio_set_pin_direction(adc_powerdown, true);
	gpio_set_pin_direction(adc_amplifier_enable, true);


	// Disable ADC powerdown, and enable our amplifier, by default.
	gpio_clear_pin(adc_powerdown);
	gpio_set_pin(adc_amplifier_enable);
}


/**
 * Power down the ADC; should be called upon terminating Rx.
 */
static void sdir_power_down_adc(void)
{
	gpio_clear_pin(adc_amplifier_enable);
	gpio_set_pin(adc_powerdown);

}


/**
 * Set up the SDIR API.
 *
 * @return 0 on success, or an error code on failure.
 */
static int initialize_sdir(void)
{
	int rc;
	gpio_pin_t gpio_pins[] = {
		tx_dac.gpio_cs, tx_dac.gpio_sck, tx_dac.gpio_data, tx_dac.gpio_mode,
		adc_powerdown, adc_amplifier_enable
	};


	// Set up each of the GPIO we're using for our DAC.
	for (unsigned i = 0; i < ARRAY_SIZE(gpio_pins); ++i) {
		rc = set_up_sdir_gpio(gpio_pins[i]);
		if (rc) {
			return rc;
		}
	}

	// Set up the TX DAC.
	rc = ad970x_initialize(&tx_dac, 0);
	if (rc) {
		pr_error("sdir: could not configure DAC!\n");
		return rc;
	}

	sdir_power_up_adc();
	pr_info("SDIR initialized.\n");

	sdir_initialized = true;
	return 0;
}


/**
 * Tear down the SDIR API, releasing the relevant resources.
 */
static int terminate_sdir(void)
{
	int rc;

	gpio_pin_t gpio_pins[] = {
		tx_dac.gpio_cs, tx_dac.gpio_sck, tx_dac.gpio_data, tx_dac.gpio_mode,
		adc_powerdown, adc_amplifier_enable
	};

	// Ensure our SDIR functionality is no longer running.
	sgpio_halt(&sdir);

	// Stop our capture ADC.
	sdir_power_down_adc();

	// Release each of our GPIO pins.
	for (unsigned i = 0; i < ARRAY_SIZE(gpio_pins); ++i) {
		rc = tear_down_sdir_gpio(gpio_pins[i]);
		if (rc) {
			return rc;
		}
	}

	sdir_initialized = false;
	return 0;
}


static int verb_dac_register_write(struct command_transaction *trans)
{
	int rc;
	uint8_t address, value;

	address = comms_argument_parse_uint8_t(trans);
	value   = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	// Ensure that we can use SDIR functionality.
	rc = initialize_sdir();
	if (rc) {
		pr_error("sdir: couldn't initialize SDIR! (%d)\n", rc);
		return rc;
	}

	// Perform the actual register write.
	ad970x_register_write(&tx_dac, address, value);
	return 0;
}


static int verb_dac_register_read(struct command_transaction *trans)
{
	int rc;
	uint8_t address, value;

	address = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	// Ensure that we can use SDIR functionality.
	rc = initialize_sdir();
	if (rc) {
		pr_error("sdir: couldn't initialize SDIR! (%d)\n", rc);
		return rc;
	}

	// Issue the actual read, and return what we get back.
	value = ad970x_register_read(&tx_dac, address);
	comms_response_add_uint8_t(trans, value);

	return 0;
}

static int verb_start_receive(struct command_transaction *trans)
{
	int rc;
	(void)trans;

	// Set up our SDIR frontend...
	rc = initialize_sdir();
	if (rc) {
		pr_error("sdir: couldn't initialize SDIR! (%d)\n", rc);
		return rc;
	}

	rc = sgpio_set_up_functions(&sdir);
	if (rc) {
		return rc;
	}

	// Finally, start the SGPIO streaming for the relevant buffer.
	usb_streaming_start_streaming_to_host(&sdir_functions[0].position_in_buffer, &sdir_functions[0].data_in_buffer);
	sgpio_run(&sdir);

	return 0;
}


static int verb_stop(struct command_transaction *trans)
{
	(void)trans;

	// Halt transmission / receipt.
	usb_streaming_stop_streaming_to_host();
	return terminate_sdir();
}


static struct comms_verb _verbs[] = {
		{  .name = "start_receive", .handler = verb_start_receive, .in_signature = "", .out_signature = "",
           .doc = "Start receipt of SDIR data on the primary bulk comms pipe." },
		{  .name = "stop", .handler = verb_stop, .in_signature = "", .out_signature = "",
           .doc = "Halt SDIR communications; termianting any active communications" },


		// Debug.
		{  .name = "dac_register_read", .handler = verb_dac_register_read, .in_signature = "<B",
		   .out_signature = "<B", .in_param_names = "register_address", .out_param_names = "value",
           .doc = "debug: read a raw value from an AD904 register" },
		{  .name = "dac_register_write", .handler = verb_dac_register_write, .in_signature = "<BB",
		   .out_signature = "", .in_param_names = "register_address, value", .out_param_names = "",
           .doc = "debug: write a raw value from an AD904 register" },

		// Sentinel.
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(sdir, CLASS_NUMBER_SELF, "sdir", _verbs,
        "functionality for Software Defined InfraRed");
