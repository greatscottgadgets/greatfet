/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <stdio.h>

#include <toolchain.h>

#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/m4/nvic.h>

#include <greatfet_core.h>

#include "drivers/usb/lpc43xx/usb.h"

#include "usb_request_handlers.h"

// TODO: get rid of these
#include "legacy_apis/usb_api_sdir.h"
#include "legacy_apis/usb_api_usbhost.h"
#include "legacy_apis/usb_api_logic_analyzer.h"
#include "legacy_apis/usb_api_adc.h"
#include "legacy_apis/usb_api_greatdancer.h"

#include "classes/heartbeat.h"
#include "glitchkit.h"

#include <rom_iap.h>
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_endpoint.h"

#include "usb_bulk_buffer.h"

#include "debug.h"

#include <drivers/memory/allocator.h>


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
	heartbeat_init();

	// For now, don't bring up the RTC, as bring up is slow and we don't
	// immediately use it. This can be enabled here, but it's likely best to
	// just bring the RTC up on-demand.
	/* rtc_init(); */

	init_usb0();
	init_greatdancer_api();
	init_usbhost_api();

	while(true) {
		if(logic_analyzer_enabled) {
			logic_analyzer_mode();
		}
		if(sdir_rx_enabled) {
			sdir_rx_mode();
		}
		if(sdir_tx_enabled) {
			sdir_tx_mode();
		}
		if(adc_mode_enabled) {
			adc_mode();
		}
		service_heartbeat();
		service_glitchkit();
	}

	return 0;
}
