/*
 * This file is part of GreatFET
 */

#include "usb_api_rfhax.h"

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"
#include <greatfet_core.h>
#include <clocks.h>
#include "pins.h"

#define RFHAX_OFF 0
#define RFHAX_CW 1
#define RFHAX_ASK 2
#define RFHAX_FSK 3

volatile bool rfhax_enabled = true;

static uint8_t rfhax_mode = RFHAX_FSK;
uint16_t f1;
uint16_t f2;

usb_request_status_t usb_vendor_request_rfhax(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	rfhax_mode = endpoint->setup.index;
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		f1 = endpoint->setup.value;
		pll0audio_config(f1);
		switch(rfhax_mode) {
			case RFHAX_OFF:
				rfhax_enabled = false;
				pll0audio_off();
				usb_transfer_schedule_ack(endpoint->in);
				break;
			case RFHAX_CW:
				rfhax_enabled = false;
				pll0audio_on();
				usb_transfer_schedule_ack(endpoint->in);
				break;
			case RFHAX_ASK:
				rfhax_enabled = true;
				usb_transfer_schedule_ack(endpoint->in);
				break;
			case RFHAX_FSK:
				rfhax_enabled = true;
				f2 = 4319;
				// usb_transfer_schedule_block(endpoint->out, &f2, 2, NULL, NULL);
				usb_transfer_schedule_ack(endpoint->in);
				break;
		}
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

uint8_t lock_code[] = {
1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 
1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1
	 };

void rfhax(void)
{
	uint8_t i, code_len = 250;
	f1 = 315;
	f2 = 3151;
	// f1 = 317;
	// f2 = 3171;
	pll0usb_config(f2);
	pll0audio_config(f1);
	while(rfhax_enabled) {
		pll0audio_on();
		pll0usb_on();
		for(i=0; i<code_len; i++) {
			led_toggle(LED3);
			switch_clocks(lock_code[i]);
			delay_us(242);
		}
		pll0usb_off();
		pll0audio_off();
		delay_us(40000);
	}
}
