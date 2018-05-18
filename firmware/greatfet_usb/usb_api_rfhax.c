/*
 * This file is part of GreatFET
 */

#include "usb_api_rfhax.h"

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"
#include <greatfet_core.h>
#include <clocks.h>

#define RFHAX_OFF 0
#define RFHAX_CW 1
#define RFHAX_ASK 2
#define RFHAX_FSK 3

volatile bool rfhax_mode_enabled = false;

static uint8_t rfhax_mode;
uint16_t f1;
uint16_t f2;

usb_request_status_t usb_vendor_request_rfhax(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	rfhax_mode = endpoint->setup.index;
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		switch(endpoint->setup.index) {
			case RFHAX_OFF:
				rfhax_mode_enabled = false;
				pll0audio_off();
				usb_transfer_schedule_ack(endpoint->in);
				break;
			case RFHAX_CW:
				rfhax_mode_enabled = false;
				pll0audio_on();
				usb_transfer_schedule_ack(endpoint->in);
				break;
			case RFHAX_ASK:
				rfhax_mode_enabled = true;
				f1 = endpoint->setup.value;
				usb_transfer_schedule_ack(endpoint->in);
				break;
			case RFHAX_FSK:
				rfhax_mode_enabled = true;
				f1 = endpoint->setup.value;
				usb_transfer_schedule_block(endpoint->out, &f2, 2, NULL, NULL);
				break;
		}
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


void rfhax(void)
{
	bool phase = false;
	pll0audio_tune(f1);
	while(rfhax_mode_enabled) {
		if(rfhax_mode==RFHAX_ASK) {
			if(phase) {
				pll0audio_on();
			} else {
				pll0audio_off();
			}
		} else if(rfhax_mode==RFHAX_FSK) {
			pll0audio_on();
			if(phase) {
				pll0audio_tune(f1);
			} else {
				pll0audio_tune(f2);
			}
		} else {
			rfhax_mode_enabled = false;
		}
		delay(100000);
	}
}