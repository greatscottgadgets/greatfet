/*
 * This file is part of GreatFET
 */

#include "usb_api_operacake.h"
#include "usb_queue.h"

#include <operacake.h>
#include <greatfet_core.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/timer.h>

static uint32_t tx_samplerate = 100000000;
volatile bool operacake_tx_enabled = false;

#define OPERACAKE_GPIO_U2CTRL1(x) (x<<6)
#define OPERACAKE_GPIO_U2CTRL0(x) (x<<3)
#define OPERACAKE_GPIO_U3CTRL1(x) (x<<2)
#define OPERACAKE_GPIO_U3CTRL0(x) (x<<4)
#define OPERACAKE_GPIO_U1CTRL(x)  (x<<5)

#define OPERACAKE_PORT_A1 (OPERACAKE_GPIO_U2CTRL0(0) | OPERACAKE_GPIO_U2CTRL1(0))
#define OPERACAKE_PORT_A2 (OPERACAKE_GPIO_U2CTRL0(1) | OPERACAKE_GPIO_U2CTRL1(0))
#define OPERACAKE_PORT_A3 (OPERACAKE_GPIO_U2CTRL0(0) | OPERACAKE_GPIO_U2CTRL1(1))
#define OPERACAKE_PORT_A4 (OPERACAKE_GPIO_U2CTRL0(1) | OPERACAKE_GPIO_U2CTRL1(1))

#define OPERACAKE_PORT_B1 (OPERACAKE_GPIO_U3CTRL0(0) | OPERACAKE_GPIO_U3CTRL1(0))
#define OPERACAKE_PORT_B2 (OPERACAKE_GPIO_U3CTRL0(1) | OPERACAKE_GPIO_U3CTRL1(0))
#define OPERACAKE_PORT_B3 (OPERACAKE_GPIO_U3CTRL0(0) | OPERACAKE_GPIO_U3CTRL1(1))
#define OPERACAKE_PORT_B4 (OPERACAKE_GPIO_U3CTRL0(1) | OPERACAKE_GPIO_U3CTRL1(1))

#define PATH1 (OPERACAKE_PORT_A1 | OPERACAKE_PORT_B1)
#define PATH2 (OPERACAKE_PORT_A2 | OPERACAKE_PORT_B2)
#define PATH3 (OPERACAKE_PORT_A3 | OPERACAKE_PORT_B3)
#define PATH4 (OPERACAKE_PORT_A4 | OPERACAKE_PORT_B4)

#include "beacon.h"

void operacake_tx_mode(void)
{
	volatile uint32_t* mask = &(GPIO_LPC_PORT(2)->mask);
	/* 1 = masked, 0 = set via mpin */
	*mask = ~(OPERACAKE_GPIO_U2CTRL0(1) | OPERACAKE_GPIO_U2CTRL1(1) | OPERACAKE_GPIO_U3CTRL0(1) | OPERACAKE_GPIO_U3CTRL1(1));
	volatile uint32_t* target = &(GPIO_LPC_PORT(2)->mpin);
	uint32_t i;
	while(1) {
		i = 0;
		while(i<23584) {
			*target = beacon_chips[i++];
			__asm__("nop");
			__asm__("nop");
			__asm__("nop");
			__asm__("nop");
			__asm__("nop");
			__asm__("nop");
			__asm__("nop");
			__asm__("nop");
			__asm__("nop");
			__asm__("nop");
			__asm__("nop");
		}
	}
}

usb_request_status_t usb_vendor_request_operacake(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		operacake_init();
		operacake_gpio();
		operacake_tx_enabled = true;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
