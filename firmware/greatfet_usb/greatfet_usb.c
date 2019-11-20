/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <stdio.h>

#include <scheduler.h>
#include <toolchain.h>

#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/m4/nvic.h>

#include <greatfet_core.h>

#include "drivers/usb/usb.h"


#include "usb_request_handlers.h"

// TODO: get rid of these
#include "legacy_apis/usb_api_sdir.h"
#include "legacy_apis/usb_api_usbhost.h"

#include "classes/heartbeat.h"
#include "usb_streaming.h"
#include "glitchkit.h"

#include <rom_iap.h>
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_endpoint.h"

#include "usb_bulk_buffer.h"

#include "debug.h"

#include <drivers/platform_clock.h>
#include <drivers/memory/allocator.h>



void emergency_mode(void);

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
	nvic_set_priority(NVIC_SGPIO_IRQ, 0);

	usb_run(&usb_peripherals[0]);
}



int main(void)
{
	pin_setup();
	heartbeat_init();

	init_usb0();
	pr_info("GreatFET initialization complete!\n");

	if (platform_get_parent_clock_source(CLOCK_SOURCE_PLL0_USB) == CLOCK_SOURCE_INTERNAL_OSCILLATOR) {
		emergency_mode();
	}

	// Run all of our tasks (methods defined with DEFINE_TASK), and never return.
	scheduler_run();
	return 0;
}


/**
 * Emergency mode entered when the system's main clock has failed to start.
 */
void emergency_mode(void)
{
	while(1) {
		led_toggle(LED2);
		led_toggle(LED3);
		led_toggle(LED4);
		delay_us(10000000);
		led_toggle(LED1);
	}
}
