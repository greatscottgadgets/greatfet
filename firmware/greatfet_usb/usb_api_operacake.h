/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_OPERACAKE_H__
#define __USB_API_OPERACAKE_H__

#include <usb_type.h>
#include <usb_request.h>

extern volatile bool operacake_tx_enabled;

usb_request_status_t  usb_vendor_request_operacake(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
void operacake_tx_mode(void);

#endif /* __USB_API_OPERACAKE_H__ */
