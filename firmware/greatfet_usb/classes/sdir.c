/*
 * This file is part of GreatFET
 *
 * Code for using the Gladiolus SDIR neighbor.
 */

#include <debug.h>

#include <drivers/comms.h>
#include <drivers/gpio.h>
#include <drivers/dac/ad970x.h>

#include "../pin_manager.h"

#define CLASS_NUMBER_SELF (0x10E)

// Store the state of the SDIR initialization.
static bool sdir_initialized = false;


/**
 * State of the DAC used for SDIR transmit.
 *
 * FIXME: use a neighbor configuration API to figure these out!
 */
static ad970x_t tx_dac = {

	// Port configuration.
 	.gpio_port_cs   = 2,
	.gpio_port_sck  = 2,
	.gpio_port_data = 2,
	.gpio_port_mode = 2,

	.gpio_pin_cs   = 9,
	.gpio_pin_sck  = 11,
	.gpio_pin_data = 12,
	.gpio_pin_mode = 14,
};


/**
 * Configures a given GPIO port/pin to be used for SDIR purposes.
 */
static int set_up_sdir_gpio(uint8_t port, uint8_t pin)
{
	int rc;
	uint8_t scu_group, scu_pin;

	// Identify the SCU group and pin for the relevant GPIO pin.
	scu_group = gpio_get_group_number(port, pin);
	scu_pin   = gpio_get_pin_number(port, pin);

	// If this port/pin doesn't correspond to a valid physical pin,
	// fail out.
	if ((scu_group == 0xff) || (scu_pin == 0xff)) {
		return EINVAL;
	}

	// Reserve the pin for this class. If we can't get a hold of the pin, fail out.
	if (!pin_ensure_reservation(scu_group, scu_pin, CLASS_NUMBER_SELF)) {
		pr_warning("sdir: couldn't reserve busy pin GPIO%d[%d]!\n", port, pin);
		return EBUSY;
	}

	// Configure the pin to be used as a GPIO in the SCU.
	rc = gpio_configure_pinmux(port, pin);
	if (rc) {
		pr_warning("sdir: couldn't configure pinmux for GPIO%d[%d]!\n", port, pin);
		return rc;
	}

	return 0;
}


/**
 * Configures a given GPIO port/pin to be used for SDIR purposes.
 */
static int tear_down_sdir_gpio(uint8_t port, uint8_t pin)
{
	uint8_t scu_group, scu_pin;

	// Identify the SCU group and pin for the relevant GPIO pin.
	scu_group = gpio_get_group_number(port, pin);
	scu_pin   = gpio_get_pin_number(port, pin);

	// If this port/pin doesn't correspond to a valid physical pin,
	// fail out.
	if ((scu_group == 0xff) || (scu_pin == 0xff)) {
		return EINVAL;
	}

    // Place the input pin back into a high-Z state by disconnecting
    // its output driver.  TODO: should this be done in the SCU, as well?
    gpio_set_pin_direction(port, pin, false);

    // Finally, release the relevant reservation.
    return pin_release_reservation(scu_group, scu_pin);
}


/**
 * Set up the SDIR API.
 *
 * @return 0 on success, or an error code on failure.
 */
static int initialize_sdir(void)
{
	int rc;

	// If we're already initialized, trivially succeed.
	if (sdir_initialized) {
		return 0;
	}

	// Set up each of the GPIO we're using.
	rc = set_up_sdir_gpio(tx_dac.gpio_port_cs,   tx_dac.gpio_pin_cs);
	if (rc) {
		return rc;
	}
	rc = set_up_sdir_gpio(tx_dac.gpio_port_sck,  tx_dac.gpio_pin_sck);
	if (rc) {
		return rc;
	}
	rc = set_up_sdir_gpio(tx_dac.gpio_port_data, tx_dac.gpio_pin_data);
	if (rc) {
		return rc;
	}
	rc = set_up_sdir_gpio(tx_dac.gpio_port_mode, tx_dac.gpio_pin_mode);
	if (rc) {
		return rc;
	}

	// Set up the TX DAC.
	rc = ad970x_initialize(&tx_dac, 0);
	if (rc) {
		pr_error("sdir: could not configure DAC!\n");
		return rc;
	}

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

	rc = tear_down_sdir_gpio(tx_dac.gpio_port_cs,   tx_dac.gpio_pin_cs);
	if (rc) {
		return rc;
	}
	rc = tear_down_sdir_gpio(tx_dac.gpio_port_sck,  tx_dac.gpio_pin_sck);
	if (rc) {
		return rc;
	}
	rc = tear_down_sdir_gpio(tx_dac.gpio_port_data, tx_dac.gpio_pin_data);
	if (rc) {
		return rc;
	}
	rc = tear_down_sdir_gpio(tx_dac.gpio_port_mode, tx_dac.gpio_pin_mode);
	if (rc) {
		return rc;
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

static int verb_stop(struct command_transaction *trans)
{
	(void)trans;
	return terminate_sdir();
}


static struct comms_verb _verbs[] = {
		{  .name = "dac_register_read", .handler = verb_dac_register_read, .in_signature = "<B",
		   .out_signature = "<B", .in_param_names = "register_address", .out_param_names = "value",
           .doc = "TEMPORARY: read a raw value from an AD904 register" },
		{  .name = "dac_register_write", .handler = verb_dac_register_write, .in_signature = "<BB",
		   .out_signature = "", .in_param_names = "register_address, value", .out_param_names = "",
           .doc = "TEMPORARY: write a raw value from an AD904 register" },
		{  .name = "stop", .handler = verb_stop, .in_signature = "", .out_signature = "",
           .doc = "Halt SDIR communications; termianting any active communications" },

		// Sentinel.
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(sdir, CLASS_NUMBER_SELF, "sdir", _verbs,
        "WIP: functionality for Software Defined InfraRed");
