/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_SPIFLASH_H__
#define __USB_API_SPIFLASH_H__

#include <drivers/usb/lpc43xx/usb_type.h>
#include <drivers/usb/lpc43xx/usb_request.h>

usb_request_status_t usb_vendor_request_spiflash_init(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_spiflash_erase(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_spiflash_write(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_spiflash_read(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

#endif /* end of include guard: __USB_API_SPIFLASH_H__ */
