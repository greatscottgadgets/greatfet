/*
 * This file is part of GreatFET
 */

#ifndef __USB_STANDARD_REQUEST_H__
#define __USB_STANDARD_REQUEST_H__

#include "usb_type.h"
#include "usb_request.h"

void usb_set_configuration_changed_cb(
        void (*callback)(usb_peripheral_t* const)
);

usb_request_status_t usb_standard_request(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage
);

const uint8_t* usb_endpoint_descriptor(
	const usb_endpoint_t* const endpoint
);

uint_fast16_t usb_endpoint_descriptor_max_packet_size(
	const uint8_t* const endpoint_descriptor
);

usb_transfer_type_t usb_endpoint_descriptor_transfer_type(
	const uint8_t* const endpoint_descriptor
);

bool usb_set_configuration(
	usb_peripheral_t* const device,
	const uint_fast8_t configuration_number
);

#endif//__USB_STANDARD_REQUEST_H__
