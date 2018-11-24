/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <stdio.h>
#include <toolchain.h>
#include <debug.h>

#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <drivers/usb/lpc43xx/usb.h>
#include <greatfet_core.h>

#include "../greatfet_usb/usb_descriptor.h"
#include "../greatfet_usb/usb_device.h"
#include "../greatfet_usb/usb_endpoint.h"
#include "../greatfet_usb/usb_bulk_buffer.h"

const char flash_stub_serial[] = "dfu_flash_stub";

#define BOARD_ID_FLASH_STUB 10000

/**
 * Return the board ID for a GreatFET one in DFU mode.
 * Eventually, this should automatically determine the board type
 * and indicate it here.
 */
uint32_t core_get_board_id()
{
	return BOARD_ID_FLASH_STUB;
}

void init_usb0(void) {
	usb_set_descriptor_by_serial_number();
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

	pr_info("Starting in flash stub mode.\n");
	pr_info("Awaiting firmware programming commands.\n)");

	// Simple heartbeat so we know what's going on.
	while(true) {
		led_toggle(LED4);
		delay(10000000);
	}

	return 0;
}
