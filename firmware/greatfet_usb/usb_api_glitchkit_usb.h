/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_GLITCHKIT_USB_H__
#define __USB_API_GLITCHKIT_USBE_H__

#include "glitchkit.h"

#include <greatfet_core.h>
#include <usb_type.h>
#include <usb_request.h>


usb_request_status_t usb_vendor_request_glitchkit_control_in_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);


usb_request_status_t usb_vendor_request_glitchkit_usb_result_length(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

usb_request_status_t usb_vendor_request_glitchkit_usb_read_result(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);


 #endif /*__USB_API_LEDS_H__*/
