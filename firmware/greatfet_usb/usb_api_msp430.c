/*
 * This file is part of GreatFET
 */

#include "usb_api_msp430.h"
#include "usb_queue.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <jtag430.h>
#include <jtag.h>

#include "badge_firmware.h"

usb_request_status_t usb_vendor_request_msp430_jtag(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint16_t i, result = 0;
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		if(endpoint->setup.value == 0) {
			jtag_setup();
			result = jtag430_start_reset_halt();
			if(result != 0x89) {
				// Try a second time
				result = jtag430_start_reset_halt();
			}
			if(result == 0x89) {
				led_on(LED2);
			}
		}

		if(endpoint->setup.value == 1) {
			jtag430_eraseflash(0xA506,0xFFFE,0x3000,0);
			led_on(LED3);
		}

		uint16_t *buffer = (uint16_t*) badge_firmware;
		if(endpoint->setup.value == 2) {
			// Write firmware
			jtag430_writeflash_bulk(0xF800, badge_firmware_len/2, buffer);
			led_on(LED4);
		}

		if(endpoint->setup.value == 3) {
			for(i = 0; i < badge_firmware_len/2; i++) {
				if(jtag430_readmem(0xF800+(i*2)) != buffer[i]) {
					result=i;
					break;
				}
			}
			result=i;
		}
		if(endpoint->setup.value == 4) {
			jtag430_stop();
		}
		usb_transfer_schedule_block(endpoint->in, &result, 2, NULL, NULL);
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}
