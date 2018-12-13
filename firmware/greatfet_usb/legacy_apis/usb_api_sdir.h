/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_SDIR_H__
#define __USB_API_SDIR_H__

#include <drivers/usb/lpc43xx/usb_type.h>
#include <drivers/usb/lpc43xx/usb_request.h>

extern volatile bool sdir_rx_enabled;
extern volatile bool sdir_tx_enabled;

usb_request_status_t usb_vendor_request_sdir_rx_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_sdir_rx_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_sdir_tx_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_sdir_tx_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

void sdir_rx_mode(void);
void sdir_tx_mode(void);

#endif/*__USB_API_SDIR_H__*/
