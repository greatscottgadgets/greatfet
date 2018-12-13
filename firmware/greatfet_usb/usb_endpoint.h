/*
 * This file is part of GreatFET
 */

#ifndef __USB_ENDPOINT_H__
#define __USB_ENDPOINT_H__

#include <drivers/usb/lpc43xx/usb_type.h>
#include <drivers/usb/lpc43xx/usb_queue.h>

extern usb_endpoint_t usb0_endpoint_control_out;
extern USB_DECLARE_QUEUE(usb0_endpoint_control_out);

extern usb_endpoint_t usb0_endpoint_control_in;
extern USB_DECLARE_QUEUE(usb0_endpoint_control_in);

extern usb_endpoint_t usb0_endpoint_bulk_in;
extern USB_DECLARE_QUEUE(usb0_endpoint_bulk_in);

extern usb_endpoint_t usb0_endpoint_bulk_out;
extern USB_DECLARE_QUEUE(usb0_endpoint_bulk_out);


extern usb_endpoint_t usb1_endpoint_control_out;
extern USB_DECLARE_QUEUE(usb1_endpoint_control_out);

extern usb_endpoint_t usb1_endpoint_control_in;
extern USB_DECLARE_QUEUE(usb1_endpoint_control_in);


extern usb_endpoint_t usb1_endpoint1_out;
extern USB_DECLARE_QUEUE(usb1_endpoint1_out);
extern usb_endpoint_t usb1_endpoint1_in;
extern USB_DECLARE_QUEUE(usb1_endpoint1_in);

extern usb_endpoint_t usb1_endpoint2_out;
extern USB_DECLARE_QUEUE(usb1_endpoint2_out);
extern usb_endpoint_t usb1_endpoint2_in;
extern USB_DECLARE_QUEUE(usb1_endpoint2_in);

extern usb_endpoint_t usb1_endpoint3_out;
extern USB_DECLARE_QUEUE(usb1_endpoint3_out);
extern usb_endpoint_t usb1_endpoint3_in;
extern USB_DECLARE_QUEUE(usb1_endpoint3_in);


#endif /* end of include guard: __USB_ENDPOINT_H__ */
