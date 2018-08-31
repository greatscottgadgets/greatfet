/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_I2C_H__
#define __USB_API_I2C_H__

#include <usb_type.h>
#include <usb_request.h>

usb_request_status_t usb_vendor_request_i2c_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_i2c_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_i2c_xfer(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_i2c_response(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_i2c_get_status(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
#endif /* end of include guard: __USB_API_I2C_H__ */
