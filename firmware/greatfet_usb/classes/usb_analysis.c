/*
 * This file is part of GreatFET
 *
 * Code for ULPI interfacing, for e.g. USB analysis.
 */

#include <debug.h>

#include <drivers/comms.h>
#include <drivers/gpio.h>
#include <drivers/sgpio.h>
#include <toolchain.h>


#include "../pin_manager.h"
#include "../usb_streaming.h"
#include "../rhododendron.h"

#define CLASS_NUMBER_SELF (0x113)

// For debug only.
extern sgpio_t analyzer;

static bool phy_initialized = false;
static bool capture_running = false;
static bool capture_triggered = false;

#undef pr_debug
#define pr_debug pr_info


static gpio_pin_t ulpi_stp_gpio  = { .port = 1, .pin = 6  };


void service_usb_analysis(void)
{
	if (capture_triggered && !capture_running) {
		rhododendron_start_capture();
		capture_running = true;
	}
	if (!capture_triggered && capture_running) {
		rhododendron_stop_capture();
		capture_running = false;
	}

}


static int ulpi_write_with_retries(uint8_t address, uint8_t data)
{
	int rc;
	uint8_t retries = 128;

	while (retries--) {
		rc = ulpi_register_write(address, data);
		if (rc == 0)  {
			pr_debug("completed reg[%02x] := %02x after %d retries\n", address, data, 127 - retries);
			return 0;
		}

		delay_us(1000);
	}

	pr_warning("Failed to write addr %02x := %02x after many retries.\n", address, data);
	return EIO;
}


static int verb_initialize(struct command_transaction *trans)
{
	int rc;

	uint8_t capture_speed = comms_argument_parse_uint8_t(trans) & 0b11;

	comms_response_add_uint32_t(trans, USB_STREAMING_BUFFER_SIZE);
	comms_response_add_uint8_t(trans,  USB_STREAMING_IN_ADDRESS);

	// If our PHY is already initialized, trivially return.
	if (phy_initialized) {
		return 0;
	}

	rhododendron_turn_off_led(LED_STATUS);

	// Set up the Rhododendron board for basic use.
	rc = initialize_rhododendron();
	if (rc) {
		return rc;
	}

	delay_us(100000);


	// Put the PHY into non-driving mode, and select the capture speed.
	rc = ulpi_write_with_retries(0x04, 0b01001000 | capture_speed);
	if (rc) {
		return rc;
	}


	// Swap D+ and D-.
	rc = ulpi_write_with_retries(0x39, 0b10);
	if (rc) {
		return rc;
	}

	// Stick STP down at zero, as it's 1) no longer needed, and 2) having it go high
	// would interfere with the PHY's delivery of USB data.
	gpio_configure_pinmux(ulpi_stp_gpio);
	gpio_set_pin_value(ulpi_stp_gpio, 0);
	gpio_set_pin_direction(ulpi_stp_gpio, true);


	rhododendron_turn_on_led(LED_STATUS);
	phy_initialized = true;

	return 0;
}


static int verb_ulpi_register_write(struct command_transaction *trans)
{
	uint8_t address = comms_argument_parse_uint8_t(trans);
	uint8_t value   = comms_argument_parse_uint8_t(trans);

	if (!comms_argument_parse_okay(trans)) {
		return EBADMSG;
	}

	return ulpi_write_with_retries(address, value);
}


static int verb_dump_register_sgpio_config(struct command_transaction *trans)
{
	bool include_unused = comms_argument_parse_bool(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	sgpio_dump_configuration(LOGLEVEL_INFO, &analyzer, include_unused);
	return 0;
}


static int verb_start_capture(struct command_transaction *trans)
{
	pr_info("usb_analyzer: force-triggering USB capture\n");
	capture_triggered = true;

	return 0;
}

static int verb_stop_capture(struct command_transaction *trans)
{
	(void)trans;
	pr_info("usb_analyzer: terminating triggered USB capture\n");
	capture_triggered = false;

	return 0;
}


static struct comms_verb _verbs[] = {

		// Control.
		{  .name = "initialize", .handler = verb_initialize, .in_signature = "<B",
			.out_signature = "<IB", .out_param_names = "buffer_size, endpoint",
			.doc = "configures the target Rhododendendron board for capture (and pass-through)" },

		{  .name = "start_capture", .handler = verb_start_capture, .in_signature = "", .out_signature = "",
           .doc = "starts a capture of high-speed USB data" },
		{  .name = "stop_capture", .handler = verb_stop_capture, .in_signature = "", .out_signature = "",
           .doc = "halts the active USB capture" },


		// Debug.
		{  .name = "ulpi_register_write", .handler = verb_ulpi_register_write, .in_signature = "<BB",
		   .out_signature = "", .in_param_names = "register_address, register_value", .out_param_names = "",
           .doc = "debug: write directly to a register on the attached USB phy"},
		{ .name = "dump_register_sgpio_configuration",  .handler = verb_dump_register_sgpio_config,
			.in_signature = "<?", .out_signature="", .in_param_names = "include_unused",
			.doc = "Requests that the system dumps its SGPIO configuration state to the debug ring." },

		// Sentinel.
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(usb_analyzer, CLASS_NUMBER_SELF, "usb_analyzer", _verbs,
        "functionality for analyzing USB");
