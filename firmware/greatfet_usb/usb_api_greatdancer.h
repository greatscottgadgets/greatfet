/*
 * Copyright 2017 Kyle J. Temkin
 *
 * This file is part of GreatFET.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
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
	GET_ENDPTSTATUS = 3
};
typedef enum greatdancer_status_request greatdancer_status_request_t;

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
