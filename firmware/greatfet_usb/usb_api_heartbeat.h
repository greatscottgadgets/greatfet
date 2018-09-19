/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_HEARTBEAT_H__
#define __USB_API_HEARTBEAT_H__

#include <greatfet_core.h>
#include <drivers/usb/lpc43xx/usb_type.h>
#include <drivers/usb/lpc43xx/usb_request.h>

extern volatile bool heartbeat_mode_enabled;

usb_request_status_t usb_vendor_request_heartbeat_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_heartbeat_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

void heartbeat_init(void);
void heartbeat_mode(void);

#endif /*__USB_API_HEARTBEAT_H__*/
