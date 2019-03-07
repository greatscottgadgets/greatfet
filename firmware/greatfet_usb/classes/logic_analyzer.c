/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>

#include "sgpio.h"
#include "../sgpio_isr.h"

#include <drivers/usb/usb.h>
#include <drivers/usb/usb_queue.h>

#include "../usb_bulk_buffer.h"
#include "../usb_endpoint.h"

#include <greatfet_core.h>

#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/lpc43xx/sgpio.h>
#include <libopencm3/cm3/vector.h>

#define CLASS_NUMBER_SELF (0x10D)

volatile bool logic_analyzer_enabled = false;

static const sgpio_config_t sgpio_config = {
	.slice_mode_multislice = true,
	.clock_divider = 3,
};

static void logic_analyzer_sgpio_start()
{
	sgpio_configure_pin_functions(&sgpio_config);
	sgpio_configure(&sgpio_config, SGPIO_DIRECTION_INPUT);

	vector_table.irq[NVIC_SGPIO_IRQ] = sgpio_isr_input;

	nvic_set_priority(NVIC_SGPIO_IRQ, 0);
	nvic_enable_irq(NVIC_SGPIO_IRQ);
	SGPIO_SET_EN_1 = (1 << SGPIO_SLICE_A);
}

static void logic_analyzer_sgpio_stop()
{
	SGPIO_CLR_EN_1 = (1 << SGPIO_SLICE_A);

	nvic_disable_irq(NVIC_SGPIO_IRQ);
}

void service_logic_analyzer(void)
{
	if(!logic_analyzer_enabled)
		return;

	usb_endpoint_init(&usb0_endpoint_bulk_in);

	logic_analyzer_sgpio_start();

	unsigned int phase = 1;
	while(logic_analyzer_enabled) {
		if ( usb_bulk_buffer_offset >= 16384
		     && phase == 1) {

			usb_transfer_schedule_block(
				&usb0_endpoint_bulk_in,
				&usb_bulk_buffer[0x0000],
				0x4000, 0, 0
				);
			phase = 0;
		}

		// Set up IN transfer of buffer 1.
		if ( usb_bulk_buffer_offset < 16384
		     && phase == 0) {

			usb_transfer_schedule_block(
				&usb0_endpoint_bulk_in,
				&usb_bulk_buffer[0x4000],
				0x4000, 0, 0
			);
			phase = 1;
		}
	}

	logic_analyzer_sgpio_stop();

	usb_endpoint_disable(&usb0_endpoint_bulk_in);
}

static int logic_analyzer_verb_start(struct command_transaction *trans)
{
	(void)trans;

	logic_analyzer_enabled = true;
	return 0;
}

static int logic_analyzer_verb_stop(struct command_transaction *trans)
{
	(void)trans;

	logic_analyzer_enabled = false;
	return 0;
}

static struct comms_verb logic_analyzer_verbs[] = {
	{ .name = "start", .handler = logic_analyzer_verb_start,
		.in_signature = "", .out_signature = "", .doc = "starts a logic analyzer capture" },
	{ .name = "stop", .handler = logic_analyzer_verb_stop,
		.in_signature = "", .out_signature = "", .doc = "terminates an active logic analyzer capture" },
	{}
};
COMMS_DEFINE_SIMPLE_CLASS(logic_analyzer, CLASS_NUMBER_SELF, "logic_analyzer", logic_analyzer_verbs,
		"Controls the logic analyzer function using the SGPIO peripheral");
