/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include "operacake.h"
#include <operacake.h>
#include <gpio_lpc.h>

// This file's class number.
#define CLASS_NUMBER_SELF (0x10f)

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

void service_operacake(void)
{
	if(!operacake_tx_enabled)
		return;
	volatile uint32_t* mask = &(GPIO_LPC_PORT(2)->mask);
	/* 1 = masked, 0 = set via mpin */
	*mask = ~(OPERACAKE_GPIO_U2CTRL0(1) | OPERACAKE_GPIO_U2CTRL1(1) | OPERACAKE_GPIO_U3CTRL0(1) | OPERACAKE_GPIO_U3CTRL1(1));
	volatile uint32_t* target = &(GPIO_LPC_PORT(2)->mpin);
	uint32_t i;
	while(operacake_tx_enabled) {
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

static int operacake_verb_start(struct command_transaction *trans)
{
	(void)trans;
	operacake_init();
	operacake_gpio();
	operacake_tx_enabled = true;
	return 0;
}

static int operacake_verb_stop(struct command_transaction *trans)
{
	(void)trans;
	operacake_tx_enabled = false;
	return 0;
}

/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "start", .handler = operacake_verb_start,
			.in_signature = "", .out_signature = "",
			.doc = "Start beacon TX" },
		{ .name = "stop", .handler = operacake_verb_stop,
			.in_signature = "", .out_signature = "",
			.doc = "Stop beacon TX" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(operacake, CLASS_NUMBER_SELF, "operacake", _verbs,
		"API for Opera Cake PSK demo.");
