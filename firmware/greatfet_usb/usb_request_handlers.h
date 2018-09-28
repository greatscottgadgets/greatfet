/*
 * This file is part of GreatFET
 */

#ifndef __GREATFET_VENDOR_REQUEST_H__
#define __GREATFET_VENDOR_REQUEST_H__

#include <drivers/usb/lpc43xx/usb_type.h>
#include <drivers/usb/lpc43xx/usb_request.h>

usb_request_status_t usb0_vendor_request(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

#endif /*__GREATFET_VENDOR_REQUEST_H__*/
