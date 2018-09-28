/*
 * This file is part of GreatFET
 */

#include "usb_api_heartbeat.h"
#include <libopencm3/lpc43xx/rtc.h>

#include <drivers/usb/lpc43xx/usb.h>
#include <drivers/usb/lpc43xx/usb_queue.h>
#include "usb_endpoint.h"

#include "pins.h"

volatile bool heartbeat_mode_enabled = true;

void heartbeat_init(void) {
	led_on(HEARTBEAT_LED);
#ifdef BOARD_CAPABILITY_RTC
	rtc_init();
#endif
}

#ifdef BOARD_CAPABILITY_RTC

/* Poll current RTC second and toggle the heartbeat LED if it has changed.
   This is called from the main loop.  It's polled instead of an ISR so that
   the LED will stop blinking if the main loop gets stuck. */
void heartbeat_mode(void) {
  static volatile uint32_t oldsec = 0;
  volatile uint32_t newsec = RTC_SEC;

  if (newsec != oldsec)
  {
    led_toggle(HEARTBEAT_LED);
    oldsec = newsec;
  }
}

#else
	/* Fall back to the old LED behavior for devices that don't support an RTC. */
	void heartbeat_mode(void) {
		led_toggle(HEARTBEAT_LED);
		delay(10000000);
	}
#endif

usb_request_status_t usb_vendor_request_heartbeat_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		heartbeat_mode_enabled = true;
    led_off(HEARTBEAT_LED);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_heartbeat_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		heartbeat_mode_enabled = false;
    led_off(HEARTBEAT_LED);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
