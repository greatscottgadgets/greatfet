/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_RFHAX_H__
#define __USB_API_RFHAX_H__

// #include <greatfet_core.h>
#include <usb_type.h>
#include <usb_request.h>

extern volatile bool rfhax_enabled;

usb_request_status_t usb_vendor_request_rfhax(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

void rfhax(void);

#endif /*__USB_API_RFHAX_H__*/
