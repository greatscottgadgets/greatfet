/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_OPERACAKE_H__
#define __USB_API_OPERACAKE_H__

#include <usb_type.h>
#include <usb_request.h>

usb_request_status_t  usb_vendor_request_operacake(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

#endif /* end of include guard: __USB_API_OPERACAKE_H__ */
