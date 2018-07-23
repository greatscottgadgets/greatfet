/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_DAC_H__
#define __USB_API_DAC_H__

#include <usb_type.h>
#include <usb_request.h>

usb_request_status_t usb_vendor_request_dac_set(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

#endif /* __USB_API_DAC_H__ */
