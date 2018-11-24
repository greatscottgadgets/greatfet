/*
 * This file is part of GreatFET
 */

#include <stdint.h>

extern uint8_t usb0_descriptor_device[];
extern uint8_t usb0_descriptor_device_qualifier[];
extern uint8_t usb0_descriptor_configuration_full_speed[];
extern uint8_t usb0_descriptor_configuration_high_speed[];
extern uint8_t usb0_descriptor_string_languages[];
extern uint8_t usb0_descriptor_string_manufacturer[];
extern uint8_t usb0_descriptor_string_product[];

#define USB_DESCRIPTOR_STRING_SERIAL_LEN 32
#define USB_DESCRIPTOR_STRING_SERIAL_BUF_LEN (USB_DESCRIPTOR_STRING_SERIAL_LEN*2 + 2) /* UTF-16LE */
extern uint8_t usb0_descriptor_string_serial_number[];

extern uint8_t* usb0_descriptor_strings[];

extern uint8_t usb1_descriptor_device[];
extern uint8_t usb1_descriptor_device_qualifier[];
extern uint8_t usb1_descriptor_configuration_full_speed[];

extern uint8_t usb1_descriptor_string_languages[];
extern uint8_t usb1_descriptor_string_manufacturer[];
extern uint8_t usb1_descriptor_string_product[];
extern uint8_t usb1_descriptor_string_serial_number[];

extern uint8_t* usb1_descriptor_strings[];

void usb_set_descriptor_by_serial_number(void);
