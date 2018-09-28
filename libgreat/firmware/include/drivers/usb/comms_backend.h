/*
 * This file is part of libgreat
 *
 * USB driver backend to the libgreat communications API.
 */

#ifndef __LIBGREAT_COMMS_BACKEND_H__
#define __LIBGREAT_COMMS_BACKEND_H__

// FIXME: move me
#define LIBGREAT_USB_COMMAND_REQUEST 0x65

usb_request_status_t libgreat_comms_vendor_request_handler(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);

#endif


