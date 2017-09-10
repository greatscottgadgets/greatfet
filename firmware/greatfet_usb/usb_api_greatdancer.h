/*
 * This file is part of GreatFET
 */

#ifndef __USB_API_GREATDANCER_H__
#define __USB_API_GREATDANCER_H__


#include <usb_type.h>
#include <usb_request.h>

/**
 * Enumeration describing each of the possible Index values for GET_STATUS
 * requests.
 */ 
enum greatdancer_status_request {
	GET_USBSTS = 0,
	GET_ENDPTSETUPSTAT = 1,
	GET_ENDPTCOMPLETE = 2,
	GET_ENDPTSTATUS = 3,
	GET_ENDPTNAK = 4
};
typedef enum greatdancer_status_request greatdancer_status_request_t;

enum greatdancer_quirks {
	MANUAL_SET_ADDRESS = 0x01,
};
typedef enum greatdancer_quirks greatdancer_quirks_t;

//
// XXX: These are just being added on as I hack them in.
// Once I know which ones and what form they'll take, I'll collapse them
// into fewer vendor requests.
//

void init_greatdancer_api(void);

usb_request_status_t usb_vendor_request_greatdancer_connect(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_greatdancer_disconnect(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

usb_request_status_t usb_vendor_request_greatdancer_get_status(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

usb_request_status_t usb_vendor_request_greatdancer_set_address(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_greatdancer_bus_reset(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_greatdancer_set_up_endpoints(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);


usb_request_status_t usb_vendor_request_greatdancer_start_nonblocking_read(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_greatdancer_get_nonblocking_data_length(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_greatdancer_finish_nonblocking_read(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);


usb_request_status_t usb_vendor_request_greatdancer_read_setup(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_greatdancer_stall_endpoint(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);


usb_request_status_t usb_vendor_request_greatdancer_send_on_endpoint(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

usb_request_status_t usb_vendor_request_greatdancer_clean_up_transfer(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

#endif
