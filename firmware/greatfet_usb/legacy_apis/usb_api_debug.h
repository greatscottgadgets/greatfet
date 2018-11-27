/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_DEBUG_H__
#define __USB_API_DEBUG_H__

#include <drivers/usb/lpc43xx/usb_type.h>
#include <drivers/usb/lpc43xx/usb_request.h>

usb_request_status_t usb_vendor_request_read_dmesg(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

#endif
