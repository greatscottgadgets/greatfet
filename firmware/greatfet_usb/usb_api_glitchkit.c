/*
 * This file is part of GreatFET
 *
 * GlichKit: simple condition counter
 *
 * Feel free to expand me to support arbitrarily complex conditions, but be
 * aware that runtime in the ISR shouldn't grow too long or we'll miss conditions.
 */

#include "usb_api_glitchkit_simple.h"
#include "glitchkit.h"

#include <greatfet_core.h>
#include <string.h>

#include "usb.h"
#include "usb_queue.h"
#include "usb_endpoint.h"

typedef enum {

	/* Accepts a glitchkit_event_t set of flags that will specify the set of _all_
	 * events that must be met before active GlitchKit modules will start their work. */
	GLITCHKIT_SETUP_SET_SYNCHRONIZATION_EVENTS = 0,


	/* Accepts a glitchkit_event_t set of flags that will specify the set of _all_
	 * events that will generate a trigger. */
	GLITCHKIT_SETUP_SET_TRIGGER_EVENTS,

	/* Similar to the above, but adds events instead of overwriting. */
	GLITCHKIT_SETUP_ADD_TRIGGER_EVENTS,

} glitchkit_setup_command_t;


/**
 * Vendor request that performs a GlitchKit setup event.
 *
 * index: a value from glitchkit_setup_command_t
 *
 *	data stage:
 *		the value to be written to the given register (little endian)
 */
usb_request_status_t usb_vendor_request_glitchkit_setup(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	static uint32_t value_to_write;

	if (stage == USB_TRANSFER_STAGE_SETUP) {

		// Ensure we have a well-sized command.
		if (endpoint->setup.length != sizeof(value_to_write)) {
			return USB_REQUEST_STATUS_STALL;
		}

		// Read in the configuration command from the host.
		usb_transfer_schedule_block(endpoint->out, &value_to_write, endpoint->setup.length, NULL, NULL);

	} else if(stage == USB_TRANSFER_STAGE_DATA) {

		// Set up GlitchKit if it hasn't been set up already.
		glitchkit_enable();

		// Execute the provided command.
		switch(endpoint->setup.index) {

			case GLITCHKIT_SETUP_SET_SYNCHRONIZATION_EVENTS:
				glitchkit_use_event_for_synchronization(value_to_write);
				break;

			case GLITCHKIT_SETUP_SET_TRIGGER_EVENTS:
				glitchkit_disable_trigger_on(~0);
				// deliberate fallthrough

			case GLITCHKIT_SETUP_ADD_TRIGGER_EVENTS:
				glitchkit_enable_trigger_on(value_to_write);
				break;

			default:
				return USB_REQUEST_STATUS_STALL;
		}

    usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}


