/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_USBHOST_H_
#define __USB_API_USBHOST_H_

#include <usb_type.h>
#include <usb_request.h>

usb_request_status_t usb_vendor_request_usbhost_connect(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

usb_request_status_t usb_vendor_request_usbhost_get_status(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

usb_request_status_t usb_vendor_request_usbhost_bus_reset(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

#endif
