/*
 * This file is part of GreatFET
 */


#include <stdint.h>
#include <stdlib.h>

#include <drivers/usb/usb_type.h>
#include "usb_descriptor.h"

#include <rom_iap.h>

#define USB_VENDOR_ID			(0x1D50)
#define USB0_PRODUCT_ID			(0x60E6)
#define USB1_PRODUCT_ID			(0x60E7)

#define USB_WORD(x)	(x & 0xFF), ((x >> 8) & 0xFF)

#define USB_MAX_PACKET0     	(64)
#define USB_MAX_PACKET_BULK_FS	(64)
#define USB_MAX_PACKET_BULK_HS	(512)

#define USB_BULK_IN_EP_ADDR 	(0x81)
#define USB_BULK_OUT_EP_ADDR 	(0x02)

#define USB_STRING_LANGID		(0x0409)

uint8_t usb0_descriptor_device[] = {
	18,				   // bLength
	USB_DESCRIPTOR_TYPE_DEVICE,	   // bDescriptorType
	USB_WORD(0x0200),		   // bcdUSB
	0x00,				   // bDeviceClass
	0x00,				   // bDeviceSubClass
	0x00,				   // bDeviceProtocol
	USB_MAX_PACKET0,		   // bMaxPacketSize0
	USB_WORD(USB_VENDOR_ID),	   // idVendor
	USB_WORD(USB0_PRODUCT_ID),	   // idProduct
	USB_WORD(0x0100),		   // bcdDevice
	0x01,				   // iManufacturer
	0x02,				   // iProduct
	0x03,				   // iSerialNumber
	0x01				   // bNumConfigurations
};

uint8_t usb0_descriptor_device_qualifier[] = {
	10,					// bLength
	USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,	// bDescriptorType
	USB_WORD(0x0200),			// bcdUSB
	0x00,					// bDeviceClass
	0x00,					// bDeviceSubClass
	0x00,					// bDeviceProtocol
	64,					// bMaxPacketSize0
	0x02,					// bNumOtherSpeedConfigurations
	0x00					// bReserved
};

uint8_t usb0_descriptor_configuration_full_speed[] = {
	9,                                 // bLength
	USB_DESCRIPTOR_TYPE_CONFIGURATION, // bDescriptorType
	USB_WORD(41),                      // wTotalLength
	0x02,                              // bNumInterfaces
	0x01,                              // bConfigurationValue
	0x00,                              // iConfiguration
	0x80,                              // bmAttributes: USB-powered
	250,                               // bMaxPower: 500mA

	// libgreat command interface
	9,                                // bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,    // bDescriptorType
	0x00,                             // bInterfaceNumber
	0x00,                             // bAlternateSetting
	0x00,                             // bNumEndpoints
	0xFF,                             // bInterfaceClass: vendor-specific
	0xFF,                             // bInterfaceSubClass
	0xFF,                             // bInterfaceProtocol: vendor-specific
	0x00,                             // iInterface

	// libgreat pipe interface
	9,                                // bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,    // bDescriptorType
	0x01,                             // bInterfaceNumber
	0x00,                             // bAlternateSetting
	0x02,                             // bNumEndpoints
	0xFF,                             // bInterfaceClass: vendor-specific
	0xFF,                             // bInterfaceSubClass
	0xFF,                             // bInterfaceProtocol: vendor-specific
	0x00,                             // iInterface

	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	USB_BULK_IN_EP_ADDR,              // bEndpointAddress
	0x02,                             // bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_FS), // wMaxPacketSize
	0x00,                             // bInterval: no NAK

	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	USB_BULK_OUT_EP_ADDR,             // bEndpointAddress
	0x02,                             // bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_FS), // wMaxPacketSize
	0x00,                             // bInterval: no NAK


	// USB-CDC communications class standard interface descriptor
	9,                              // bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,  // bDescriptorType
	0x02,                           // bInterfaceNumber
	0x00,                           // bAlternateSetting
	0x01,                           // bNumEndpoints
	0x02,                           // bInterfaceClass: communications device class (CDC)
	0x02,                           // bInterfaceSubClass: abstract control model (ACM)
	0x02,                           // bInterfaceProtocol: V.25ter (AT commands / UART)
	0x00,                           // iInterface

	// CDC header descriptor
	5,                   // bLength
	0x24,                // bDescriptorType
	0x00,                // bDescriptorSubtype: header
	USB_WORD(0x0110),    // bcdCDC

	// CDC ACM functional descriptor
	4,                   // bLength
	0x24,                // bDescriptorType
	0x02,                // bDescriptorSubtype: ACM functional descriptor
	0b0010,              // bmCapabilities: support only baud/flow-control capabilities

	// CDC Union functional descriptor
	5,                   // bLength
	0x24,                // bDescriptorType
	0x06,                // bDescriptorSubtype: Union functional descriptor
	0x02,                // bControlInterface: interface number of this control (communications class) interface
	0x03,                // bSubordinateInterface0: data interface number (3)

	// CDC Call management functional descriptor
	5,                   // bLength
	0x24,                // bDescriptorType
	0x01,                // bDescriptorSubtype: Union functional descriptor
	0x00,                // bmCapabilities: the device manages calls over the comms interface; and manages calls itself
	0x03,                // bDataInterface: our data interface


	// USB-CDC communications endpoint
	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	0x83,                             // bEndpointAddress: IN 0x03
	0x03,                             // bmAttributes: INTERRUPT
	USB_WORD(USB_MAX_PACKET_BULK_FS), // wMaxPacketSize
	0xFF,                             // bInterval: use the maximum interval possible to avoid congestion


	// USB-CDC data class standard interface descriptor
	9,                              // bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,  // bDescriptorType
	0x03,                           // bInterfaceNumber
	0x00,                           // bAlternateSetting
	0x02,                           // bNumEndpoints
	0x0a,                           // bInterfaceClass: CDC data
	0x00,                           // bInterfaceSubClass: none
	0x00,                           // bInterfaceProtocol: none
	0x00,                           // iInterface

	// USB-CDC data: serial in
	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	0x84,                             // bEndpointAddress: IN 0x04
	0x02,                             // bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_FS), // wMaxPacketSize
	0x00,                             // bInterval: no NAK

	// USB-CDC data: serial out
	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	0x04,                             // bEndpointAddress: OUT 0x04
	0x02,                             // bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_FS), // wMaxPacketSize
	0x00,                             // bInterval: no NAK

	0,                                // TERMINATOR
};

uint8_t usb0_descriptor_configuration_high_speed[] = {
	9,                                 // bLength
	USB_DESCRIPTOR_TYPE_CONFIGURATION, // bDescriptorType
	//USB_WORD(99),                      // wTotalLength
	USB_WORD(49),                      // wTotalLength
	0x04,                              // bNumInterfaces
	0x01,                              // bConfigurationValue
	0x00,                              // iConfiguration
	0x80,                              // bmAttributes: USB-powered
	250,                               // bMaxPower: 500mA

	// libgreat command interface
	9,                                // bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,    // bDescriptorType
	0x00,                             // bInterfaceNumber
	0x00,                             // bAlternateSetting
	0x00,                             // bNumEndpoints
	0xFF,                             // bInterfaceClass: vendor-specific
	0xFF,                             // bInterfaceSubClass
	0xFF,                             // bInterfaceProtocol: vendor-specific
	0x00,                             // iInterface

	// libgreat pipe interface
	9,                                // bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,    // bDescriptorType
	0x01,                             // bInterfaceNumber
	0x00,                             // bAlternateSetting
	0x02,                             // bNumEndpoints
	0xFF,                             // bInterfaceClass: vendor-specific
	0xFF,                             // bInterfaceSubClass
	0xFF,                             // bInterfaceProtocol: vendor-specific
	0x00,                             // iInterface

	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	USB_BULK_IN_EP_ADDR,              // bEndpointAddress
	0x02,                             // bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_HS), // wMaxPacketSize
	0x00,                             // bInterval: no NAK

	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	USB_BULK_OUT_EP_ADDR,             // bEndpointAddress
	0x02,                             // bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_HS), // wMaxPacketSize
	0x00,                             // bInterval: no NAK


	// USB-CDC communications class standard interface descriptor
	9,                              // bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,  // bDescriptorType
	0x02,                           // bInterfaceNumber
	0x00,                           // bAlternateSetting
	0x01,                           // bNumEndpoints
	0x02,                           // bInterfaceClass: communications device class (CDC)
	0x02,                           // bInterfaceSubClass: abstract control model (ACM)
	0x02,                           // bInterfaceProtocol: V.25ter (AT commands / UART)
	0x00,                           // iInterface

	// CDC header descriptor
	5,                   // bLength
	0x24,                // bDescriptorType
	0x00,                // bDescriptorSubtype: header
	USB_WORD(0x0110),    // bcdCDC

	// CDC ACM functional descriptor
	4,                   // bLength
	0x24,                // bDescriptorType
	0x02,                // bDescriptorSubtype: ACM functional descriptor
	0b0010,              // bmCapabilities: support only baud/flow-control capabilities

	// CDC Union functional descriptor
	5,                   // bLength
	0x24,                // bDescriptorType
	0x06,                // bDescriptorSubtype: Union functional descriptor
	0x02,                // bControlInterface: interface number of this control (communications class) interface
	0x03,                // bSubordinateInterface0: data interface number (3)

	// CDC Call management functional descriptor
	5,                   // bLength
	0x24,                // bDescriptorType
	0x01,                // bDescriptorSubtype: Union functional descriptor
	0x00,                // bmCapabilities: the device manages calls over the comms interface; and manages calls itself
	0x03,                // bDataInterface: our data interface


	// USB-CDC communications endpoint
	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	0x83,                             // bEndpointAddress: IN 0x03
	0x03,                             // bmAttributes: INTERRUPT
	USB_WORD(USB_MAX_PACKET_BULK_HS), // wMaxPacketSize
	0xFF,                             // bInterval: use the maximum interval possible to avoid congestion


	// USB-CDC data class standard interface descriptor
	9,                              // bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,  // bDescriptorType
	0x03,                           // bInterfaceNumber
	0x00,                           // bAlternateSetting
	0x02,                           // bNumEndpoints
	0x0a,                           // bInterfaceClass: CDC data
	0x00,                           // bInterfaceSubClass: none
	0x00,                           // bInterfaceProtocol: none
	0x00,                           // iInterface

	// USB-CDC data: serial in
	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	0x84,                             // bEndpointAddress: IN 0x04
	0x02,                             // bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_HS), // wMaxPacketSize
	0x00,                             // bInterval: no NAK

	// USB-CDC data: serial out
	7,                                // bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,     // bDescriptorType
	0x04,                             // bEndpointAddress: OUT 0x04
	0x02,                             // bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_HS), // wMaxPacketSize
	0x00,                             // bInterval: no NAK

	0,                                // TERMINATOR
};

uint8_t usb0_descriptor_string_languages[] = {
	0x04,			    // bLength
	USB_DESCRIPTOR_TYPE_STRING,	    // bDescriptorType
	USB_WORD(USB_STRING_LANGID),	// wLANGID
};

uint8_t usb0_descriptor_string_manufacturer[] = {
	40,					// bLength
	USB_DESCRIPTOR_TYPE_STRING,	    // bDescriptorType
	'G', 0x00,
	'r', 0x00,
	'e', 0x00,
	'a', 0x00,
	't', 0x00,
	' ', 0x00,
	'S', 0x00,
	'c', 0x00,
	'o', 0x00,
	't', 0x00,
	't', 0x00,
	' ', 0x00,
	'G', 0x00,
	'a', 0x00,
	'd', 0x00,
	'g', 0x00,
	'e', 0x00,
	't', 0x00,
	's', 0x00,
};

uint8_t usb0_descriptor_string_product[] = {
	18,						// bLength
	USB_DESCRIPTOR_TYPE_STRING,		// bDescriptorType
	'G', 0x00,
	'r', 0x00,
	'e', 0x00,
	'a', 0x00,
	't', 0x00,
	'F', 0x00,
	'E', 0x00,
	'T', 0x00,
};

uint8_t usb0_descriptor_string_serial_number[USB_DESCRIPTOR_STRING_SERIAL_BUF_LEN];

uint8_t* usb0_descriptor_strings[] = {
	usb0_descriptor_string_languages,
	usb0_descriptor_string_manufacturer,
	usb0_descriptor_string_product,
	usb0_descriptor_string_serial_number,
	0,		// TERMINATOR
};







/* USB1 experiment */
uint8_t usb1_descriptor_device[] = {
	18,				   // bLength
	USB_DESCRIPTOR_TYPE_DEVICE,	   // bDescriptorType
	USB_WORD(0x0101),		   // bcdUSB
	0x00,				   // bDeviceClass
	0x00,				   // bDeviceSubClass
	0x00,				   // bDeviceProtocol
	USB_MAX_PACKET0,		   // bMaxPacketSize0
	USB_WORD(USB_VENDOR_ID),	   // idVendor
	USB_WORD(USB1_PRODUCT_ID),	   // idProduct
	USB_WORD(0x0100),		   // bcdDevice
	0x01,				   // iManufacturer
	0x02,				   // iProduct
	0x03,				   // iSerialNumber
	0x01				   // bNumConfigurations
};

uint8_t usb1_descriptor_device_qualifier[] = {
	10,					// bLength
	USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,	// bDescriptorType
	USB_WORD(0x0200),			// bcdUSB
	0x00,					// bDeviceClass
	0x00,					// bDeviceSubClass
	0x00,					// bDeviceProtocol
	64,					// bMaxPacketSize0
	0x02,					// bNumOtherSpeedConfigurations
	0x00					// bReserved
};

uint8_t usb1_descriptor_configuration_full_speed[] = {
	9,					// bLength
	USB_DESCRIPTOR_TYPE_CONFIGURATION,	// bDescriptorType
	USB_WORD(32),				// wTotalLength
	0x01,					// bNumInterfaces
	0x01,					// bConfigurationValue
	0x00,					// iConfiguration
	0x80,					// bmAttributes: USB-powered
	250,					// bMaxPower: 500mA

	9,							// bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,		// bDescriptorType
	0x00,							// bInterfaceNumber
	0x00,							// bAlternateSetting
	0x02,							// bNumEndpoints
	0xFF,							// bInterfaceClass: vendor-specific
	0xFF,							// bInterfaceSubClass
	0xFF,							// bInterfaceProtocol: vendor-specific
	0x00,							// iInterface

	7,							// bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	USB_BULK_IN_EP_ADDR,				// bEndpointAddress
	0x02,							// bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_FS),	// wMaxPacketSize
	0x00,							// bInterval: no NAK

	7,							// bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	USB_BULK_OUT_EP_ADDR,			// bEndpointAddress
	0x02,							// bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_FS),	// wMaxPacketSize
	0x00,							// bInterval: no NAK

	0,									// TERMINATOR
};

uint8_t usb1_descriptor_string_languages[] = {
	0x04,			    // bLength
	USB_DESCRIPTOR_TYPE_STRING,	    // bDescriptorType
	USB_WORD(USB_STRING_LANGID),	// wLANGID
};

uint8_t usb1_descriptor_string_manufacturer[] = {
	40,					// bLength
	USB_DESCRIPTOR_TYPE_STRING,	    // bDescriptorType
	'G', 0x00,
	'r', 0x00,
	'e', 0x00,
	'a', 0x00,
	't', 0x00,
	' ', 0x00,
	'S', 0x00,
	'c', 0x00,
	'o', 0x00,
	't', 0x00,
	't', 0x00,
	' ', 0x00,
	'G', 0x00,
	'a', 0x00,
	'd', 0x00,
	'g', 0x00,
	'e', 0x00,
	't', 0x00,
	's', 0x00,
};

uint8_t usb1_descriptor_string_product[] = {
	18,						// bLength
	USB_DESCRIPTOR_TYPE_STRING,		// bDescriptorType
	'G', 0x00,
	'F', 0x00,
	'T', 0x00,
	'a', 0x00,
	'r', 0x00,
	'g', 0x00,
	'e', 0x00,
	't', 0x00,
};
uint8_t usb1_descriptor_string_serial_number[] = {
	14,						// bLength
	USB_DESCRIPTOR_TYPE_STRING,		// bDescriptorType
	'G', 0x00,
	'S', 0x00,
	'G', 0x00,
	'1', 0x00,
	'2', 0x00,
	'3', 0x00,
};

uint8_t* usb1_descriptor_strings[] = {
	usb1_descriptor_string_languages,
	usb1_descriptor_string_manufacturer,
	usb1_descriptor_string_product,
	usb1_descriptor_string_serial_number,
	0,		// TERMINATOR
};


void usb_set_descriptor_by_serial_number(void)
{
	iap_cmd_res_t iap_cmd_res;

	/* Read IAP Serial Number Identification */
	iap_cmd_res.cmd_param.command_code = IAP_CMD_READ_SERIAL_NO;
	iap_cmd_call(&iap_cmd_res);

	if (iap_cmd_res.status_res.status_ret == CMD_SUCCESS) {
		usb0_descriptor_string_serial_number[0] = USB_DESCRIPTOR_STRING_SERIAL_BUF_LEN;
		usb0_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;

		/* 32 characters of serial number, convert to UTF-16LE */
		for (size_t i=0; i<USB_DESCRIPTOR_STRING_SERIAL_LEN; i++) {
			const uint_fast8_t nibble = (iap_cmd_res.status_res.iap_result[i >> 3] >> (28 - (i & 7) * 4)) & 0xf;
			const char c = (nibble > 9) ? ('a' + nibble - 10) : ('0' + nibble);
			usb0_descriptor_string_serial_number[2 + i * 2] = c;
			usb0_descriptor_string_serial_number[3 + i * 2] = 0x00;
		}
	} else {
		usb0_descriptor_string_serial_number[0] = 8;
		usb0_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;
		usb0_descriptor_string_serial_number[2] = 'G';
		usb0_descriptor_string_serial_number[3] = 0x00;
		usb0_descriptor_string_serial_number[4] = 'S';
		usb0_descriptor_string_serial_number[5] = 0x00;
		usb0_descriptor_string_serial_number[6] = 'G';
		usb0_descriptor_string_serial_number[7] = 0x00;
	}
}
