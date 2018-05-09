/*
 * This file is part of GreatFET
 */

#include "usb_standard_request.h"

#include "usb_api_board.h"
#include "usb_api_spiflash.h"
#include "usb_api_adc.h"
#include "usb_api_spi.h"
#include "usb_api_i2c.h"
#include "usb_api_gpio.h"
#include "usb_api_leds.h"
#include "usb_api_heartbeat.h"
#include "usb_api_logic_analyzer.h"
#include "usb_api_sdir.h"
#include "usb_api_greatdancer.h"
#include "usb_api_usbhost.h"
#include "usb_api_glitchkit.h"
#include "usb_api_glitchkit_simple.h"
#include "usb_api_glitchkit_usb.h"
#include "usb_api_1-Wire.h"

static const usb_request_handler_fn usb0_vendor_request_handler[] = {
	usb_vendor_request_spiflash_init,
	usb_vendor_request_spiflash_write,
	usb_vendor_request_spiflash_read,
	usb_vendor_request_spiflash_erase,
	usb_vendor_request_read_board_id,
	usb_vendor_request_read_version_string,
	usb_vendor_request_read_partid_serialno,
	NULL,
	usb_vendor_request_set_leds,
	usb_vendor_request_gpio_register,
	usb_vendor_request_gpio_write,
	usb_vendor_request_spi_init,
	usb_vendor_request_spi_write,
	usb_vendor_request_spi_read,
	usb_vendor_request_spi_dump_flash,
	usb_vendor_request_i2c_start,
	usb_vendor_request_i2c_stop,
	usb_vendor_request_i2c_xfer,
	usb_vendor_request_i2c_response,
	usb_vendor_request_logic_analyzer_start,
	usb_vendor_request_logic_analyzer_stop,
	usb_vendor_request_reset,
	usb_vendor_request_adc_init,
	NULL, // ADC read
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
	usb_vendor_request_heartbeat_start,
	usb_vendor_request_heartbeat_stop,
	usb_vendor_request_gpio_reset,
	usb_vendor_request_gpio_read,

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
	usb_vendor_request_super_hacky,

	// 1-Wire bus
	usb_vendor_request_1wire_init,
	usb_vendor_request_1wire_read

};

static const uint32_t usb0_vendor_request_handler_count =
	sizeof(usb0_vendor_request_handler) / sizeof(usb0_vendor_request_handler[0]);

usb_request_status_t usb0_vendor_request(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage) {
	usb_request_status_t status = USB_REQUEST_STATUS_STALL;

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
