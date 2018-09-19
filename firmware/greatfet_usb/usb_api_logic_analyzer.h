/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_LOGIC_ANALYZER_H__
#define __USB_API_LOGIC_ANALYZER_H__

#include <drivers/usb/lpc43xx/usb_type.h>
#include <drivers/usb/lpc43xx/usb_request.h>

extern volatile bool logic_analyzer_enabled;

usb_request_status_t usb_vendor_request_logic_analyzer_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_logic_analyzer_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

void logic_analyzer_mode(void);

#endif/*__USB_API_LOGIC_ANALYZER_H__*/
