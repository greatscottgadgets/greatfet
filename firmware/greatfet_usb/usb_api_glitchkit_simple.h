/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_GLITCHKIT_SIMPLE_H__
#define __USB_API_GLITCHKIT_SIMPLE_H__

#include "glitchkit.h"

#include <greatfet_core.h>
#include <usb_type.h>
#include <usb_request.h>

// The maximum simultaneous number of pin conditions.
// This can be upped arbitrarily, but will consume memory for each
// condition added.
#define MAX_PIN_CONDITIONS 8

usb_request_status_t usb_vendor_request_glitchkit_simple_enable_trigger(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);



 #endif /*__USB_API_LEDS_H__*/
