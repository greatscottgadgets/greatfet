/*
 * This file is part of GreatFET
 */

#ifndef __USB_HOST_H__
#define __USB_HOST_H__

// TODO: Refactor to support high performance operations without having to
// expose usb_transfer_descriptor_t. Or usb_endpoint_prime(). Or, or, or...
#include <libopencm3/lpc43xx/usb.h>
#include <libopencm3/cm3/vector.h>

#include "usb_type.h"

#define USBMODE_HOST_MODE (3)
#define TERMINATING_LINK  ((void *)0x01)


/**
 * Initializes the provided USB peripheral in host mode.
 */
void usb_host_init(usb_peripheral_t* host);


/**
 * Enable sourcing a given USB port's VBUS, if possible.
 */
int usb_provide_vbus(usb_peripheral_t *host);


/**
 * Disable sourcing a given USB port's VBUS, if possible.
 */
int usb_stop_providing_vbus(usb_peripheral_t *host);


/**
 * Issues a bus reset request, effectively asking any devices downstream
 * of the host to reset themselves.
 *
 * You should wait 100ms after calling this before interacting with the device.
 */
void usb_host_reset_device(usb_peripheral_t *host);


#endif



