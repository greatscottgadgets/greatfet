/*
 * This file is part of GreatFET
 */

#include <stddef.h>

#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/m4/nvic.h>

#include <greatfet_core.h>
#include <clocks.h>

#include "usb.h"
#include "usb_standard_request.h"

#include <rom_iap.h>
#include "../greatfet_usb/usb_descriptor.h"
#include "../greatfet_usb/usb_device.h"
#include "../greatfet_usb/usb_endpoint.h"
#include "../greatfet_usb/usb_api_board.h"
#include "../greatfet_usb/usb_api_spiflash.h"

#include "glitchkit.h"

volatile const char flash_stub_serial[] = "dfu_flash_stub";\

/* No USB1 support. */
const usb_request_handlers_t usb1_request_handlers = {};

/**
 * Vendor requests. The stub uses the same request IDs and handlers as the
 * main greatfet_usb, but only supports the requests necessary for 
 */
static const usb_request_handler_fn usb0_vendor_request_handler[] = {
	usb_vendor_request_spiflash_init,
	usb_vendor_request_spiflash_write,
	usb_vendor_request_spiflash_read,
	usb_vendor_request_spiflash_erase,
	usb_vendor_request_read_board_id,
	usb_vendor_request_read_version_string,
	usb_vendor_request_read_partid_serialno,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	usb_vendor_request_reset,
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


void patch_serial_number()
{
    char * c;
    uint8_t position = 1;

    // USB string descriptor header
    usb0_descriptor_string_serial_number[position++] = USB_DESCRIPTOR_TYPE_STRING;

    // and body
    c = flash_stub_serial;
    while(*c) {

      // Add a character.
      usb0_descriptor_string_serial_number[position++] = *c;
      usb0_descriptor_string_serial_number[position++] = 0;

      // Move to the next character.
      ++c;
    }

    // Finally, set the length of the string in the header.
    usb0_descriptor_string_serial_number[0] = position;

}

void init_usb0(void) {
	patch_serial_number();
	usb_peripheral_reset(&usb_peripherals[0]);
	usb_device_init(&usb_peripherals[0]);

	usb_queue_init(&usb0_endpoint_control_out_queue);
	usb_queue_init(&usb0_endpoint_control_in_queue);
	usb_queue_init(&usb0_endpoint_bulk_out_queue);
	usb_queue_init(&usb0_endpoint_bulk_in_queue);

	usb_endpoint_init(&usb0_endpoint_control_out);
	usb_endpoint_init(&usb0_endpoint_control_in);

	usb_endpoint_init(&usb0_endpoint_bulk_in);

	nvic_set_priority(NVIC_USB0_IRQ, 254);

	usb_run(&usb_peripherals[0]);
}


int main(void) {
	cpu_clock_init();
	cpu_clock_pll1_max_speed();
	pin_setup();

	init_usb0();

	// Simple heartbeat so we know what's going on.
	while(true) {
		led_toggle(LED4);
		delay(10000000);
	}

	return 0;
}
