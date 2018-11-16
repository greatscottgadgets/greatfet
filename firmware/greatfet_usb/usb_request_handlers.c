/*
 * This file is part of GreatFET
 */

// LEGACY SUPPORT APIS
// These should be gotten rid of!

#include <drivers/usb/lpc43xx/usb_standard_request.h>
#include <drivers/usb/comms_backend.h>

#include "legacy_apis/usb_api_adc.h"
#include "legacy_apis/usb_api_dac.h"
#include "legacy_apis/usb_api_spi.h"
#include "legacy_apis/usb_api_i2c.h"
#include "legacy_apis/usb_api_leds.h"
#include "legacy_apis/usb_api_logic_analyzer.h"
#include "legacy_apis/usb_api_sdir.h"
#include "legacy_apis/usb_api_greatdancer.h"
#include "legacy_apis/usb_api_usbhost.h"
#include "legacy_apis/usb_api_glitchkit.h"
#include "legacy_apis/usb_api_glitchkit_simple.h"
#include "legacy_apis/usb_api_glitchkit_usb.h"
#include "legacy_apis/usb_api_DS18B20.h"
#include "legacy_apis/usb_api_msp430.h"

static const usb_request_handler_fn usb0_vendor_request_handler[] = {
	NULL, //usb_vendor_request_spiflash_init,
	NULL, //usb_vendor_request_spiflash_write,
	NULL, //usb_vendor_request_spiflash_read,
	NULL, //usb_vendor_request_spiflash_erase,
	NULL, //usb_vendor_request_read_board_id,
	NULL, //usb_vendor_request_read_version_string,
	NULL, //usb_vendor_request_read_partid_serialno,
	NULL,
	usb_vendor_request_set_leds,
	NULL, //usb_vendor_request_gpio_register,
	NULL, //usb_vendor_request_gpio_write,
	usb_vendor_request_spi_init,
	usb_vendor_request_spi_write,
	usb_vendor_request_spi_read,
	usb_vendor_request_spi_dump_flash,
	usb_vendor_request_i2c_start,
	usb_vendor_request_i2c_stop,
	usb_vendor_request_i2c_xfer,
	usb_vendor_request_i2c_response,
	usb_vendor_request_i2c_get_status,
	usb_vendor_request_logic_analyzer_start,
	usb_vendor_request_logic_analyzer_stop,
	NULL, //usb_vendor_request_reset,
	usb_vendor_request_adc_init,
	usb_vendor_request_adc_read, // ADC read
	NULL, // ADC stream
	usb_vendor_request_sdir_rx_start,
	usb_vendor_request_sdir_rx_stop,
	usb_vendor_request_sdir_tx_start,
	usb_vendor_request_sdir_tx_stop,
	usb_vendor_request_greatdancer_connect,
	usb_vendor_request_greatdancer_disconnect,
	usb_vendor_request_greatdancer_bus_reset,
	usb_vendor_request_greatdancer_set_address,
	usb_vendor_request_greatdancer_set_up_endpoints,
	usb_vendor_request_greatdancer_get_status,
	usb_vendor_request_greatdancer_read_setup,
	usb_vendor_request_greatdancer_stall_endpoint,
	usb_vendor_request_greatdancer_send_on_endpoint,
	usb_vendor_request_greatdancer_clean_up_transfer,
	usb_vendor_request_greatdancer_start_nonblocking_read,
	usb_vendor_request_greatdancer_finish_nonblocking_read,
	usb_vendor_request_greatdancer_get_nonblocking_data_length,
	NULL, //usb_vendor_request_heartbeat_start,
	NULL, //usb_vendor_request_heartbeat_stop,
	NULL, //usb_vendor_request_gpio_reset,
	NULL, //usb_vendor_request_gpio_read,

	
	// GlitchKit
	usb_vendor_request_glitchkit_setup,
	usb_vendor_request_glitchkit_provide_target_clock,
	usb_vendor_request_glitchkit_simple_enable_trigger, // Simple triggers
	usb_vendor_request_glitchkit_control_in_start,
	usb_vendor_request_glitchkit_usb_result_length,
	usb_vendor_request_glitchkit_usb_read_result,

	// USB Host
	usb_vendor_request_usbhost_connect,
	usb_vendor_request_usbhost_bus_reset,
	usb_vendor_request_usbhost_get_status,
	usb_vendor_request_usbhost_set_up_endpoint,
	usb_vendor_request_usbhost_send_on_endpoint,
	usb_vendor_request_usbhost_start_nonblocking_read,
	usb_vendor_request_usbhost_finish_nonblocking_read,
	usb_vendor_request_usbhost_get_nonblocking_data_length,
	//usb_vendor_request_super_hacky,

	// DS18B20 over 1-Wire bus
	usb_vendor_request_DS18B20_read,

	// MSP430 JTAG
	//usb_vendor_request_msp430_jtag

	usb_vendor_request_dac_set,
	//usb_vendor_request_read_dmesg
};

static const uint32_t usb0_vendor_request_handler_count =
	sizeof(usb0_vendor_request_handler) / sizeof(usb0_vendor_request_handler[0]);


/**
 * @returns true iff the given setup packet describes a
 *	USB-encapsulated LibGreat command.
 */
static bool _is_libgreat_command(usb_setup_t *setup_packet)
{
	uint8_t request_recipient = setup_packet->request_type
		& USB_SETUP_REQUEST_RECIPIENT_mask;

	// If this isn't to an endpoint, it's not to us.
	if (request_recipient != USB_SETUP_REQUEST_RECIPIENT_ENDPOINT)
		return false;

	// If this request isn't to endpoint zero, it's not for us.
	if (setup_packet->index != 0)
		return false;

	// If this isn't our request number, it's not to us.
	if (setup_packet->request != LIBGREAT_USB_COMMAND_REQUEST)
		return false;

	// If all of these match, this vendor request is for us!
	return true;
}


usb_request_status_t usb0_vendor_request(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage) {
	usb_request_status_t status = USB_REQUEST_STATUS_STALL;

	if (_is_libgreat_command(&endpoint->setup)) {
		return libgreat_comms_vendor_request_handler(endpoint, stage);
	}

	if( endpoint->setup.request < usb0_vendor_request_handler_count ) {
		usb_request_handler_fn handler = usb0_vendor_request_handler[endpoint->setup.request];
		if( handler ) {
			status = handler(endpoint, stage);
		}
	}
	return status;
}

const usb_request_handlers_t usb0_request_handlers = {
	.standard = usb_standard_request,
	.class = 0,
	.vendor = usb0_vendor_request,
	.reserved = 0,
};
