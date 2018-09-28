/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_BOARD_INFO_H__
#define __USB_API_BOARD_INFO_H__

#include <stdint.h>

#include <drivers/usb/lpc43xx/usb_type.h>
#include <drivers/usb/lpc43xx/usb_request.h>

typedef struct {
	uint32_t part_id[2];
	uint32_t serial_no[4];
} read_partid_serialno_t;

usb_request_status_t usb_vendor_request_read_board_id(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_read_version_string(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_read_partid_serialno(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_reset(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

usb_request_status_t usb_vendor_request_super_hacky(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
#endif /* end of include guard: __USB_API_BOARD_INFO_H__ */
