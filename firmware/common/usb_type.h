/*
 * This file is part of GreatFET
 */

#ifndef __USB_TYPE_H__
#define __USB_TYPE_H__

#include <stdint.h>
#include <stdbool.h>

// TODO: move things out of here so we don't super-pollute the namespace
#include <libopencm3/lpc43xx/usb.h>

// TODO: Move this to some common compiler-tricks location.
#define ATTR_PACKED __attribute__((packed))
#define ATTR_ALIGNED(x)	__attribute__ ((aligned(x)))
#define ATTR_SECTION(x) __attribute__ ((section(x)))

// Define the size of the host resources that should be preallocated.
#define USB_ASYNC_LIST_SIZE            8
#define USB_PERIODIC_LIST_SIZE         8
#define USB_TOTAL_TRANSFER_DESCRIPTORS 8


typedef struct ATTR_PACKED {
	uint8_t request_type;
	uint8_t request;
	union {
		struct {
			uint8_t value_l;
			uint8_t value_h;
		};
		uint16_t value;
	};
	union {
		struct {
			uint8_t index_l;
			uint8_t index_h;
		};
		uint16_t index;
	};
	union {
		struct {
			uint8_t length_l;
			uint8_t length_h;
		};
		uint16_t length;
	};
} usb_setup_t;

typedef enum {
	USB_STANDARD_REQUEST_GET_STATUS = 0,
	USB_STANDARD_REQUEST_CLEAR_FEATURE = 1,
	USB_STANDARD_REQUEST_SET_FEATURE = 3,
	USB_STANDARD_REQUEST_SET_ADDRESS = 5,
	USB_STANDARD_REQUEST_GET_DESCRIPTOR = 6,
	USB_STANDARD_REQUEST_SET_DESCRIPTOR = 7,
	USB_STANDARD_REQUEST_GET_CONFIGURATION = 8,
	USB_STANDARD_REQUEST_SET_CONFIGURATION = 9,
	USB_STANDARD_REQUEST_GET_INTERFACE = 10,
	USB_STANDARD_REQUEST_SET_INTERFACE = 11,
	USB_STANDARD_REQUEST_SYNCH_FRAME = 12,
} usb_standard_request_t;

typedef enum {
	USB_SETUP_REQUEST_TYPE_shift = 5,
	USB_SETUP_REQUEST_TYPE_mask = 3 << USB_SETUP_REQUEST_TYPE_shift,
	
	USB_SETUP_REQUEST_TYPE_STANDARD = 0 << USB_SETUP_REQUEST_TYPE_shift,
	USB_SETUP_REQUEST_TYPE_CLASS = 1 << USB_SETUP_REQUEST_TYPE_shift,
	USB_SETUP_REQUEST_TYPE_VENDOR = 2 << USB_SETUP_REQUEST_TYPE_shift,
	USB_SETUP_REQUEST_TYPE_RESERVED = 3 << USB_SETUP_REQUEST_TYPE_shift,
	
	USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_shift = 7,
	USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_mask = 1 << USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_shift,
	USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_HOST_TO_DEVICE = 0 << USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_shift,
	USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_DEVICE_TO_HOST = 1 << USB_SETUP_REQUEST_TYPE_DATA_TRANSFER_DIRECTION_shift,
} usb_setup_request_type_t;

typedef enum {
	USB_TRANSFER_DIRECTION_OUT = 0,
	USB_TRANSFER_DIRECTION_IN = 1,
} usb_transfer_direction_t;
	
typedef enum {
	USB_DESCRIPTOR_TYPE_DEVICE = 1,
	USB_DESCRIPTOR_TYPE_CONFIGURATION = 2,
	USB_DESCRIPTOR_TYPE_STRING = 3,
	USB_DESCRIPTOR_TYPE_INTERFACE = 4,
	USB_DESCRIPTOR_TYPE_ENDPOINT = 5,
	USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER = 6,
	USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION = 7,
	USB_DESCRIPTOR_TYPE_INTERFACE_POWER = 8,
} usb_descriptor_type_t;

typedef enum {
	USB_TRANSFER_TYPE_CONTROL = 0,
	USB_TRANSFER_TYPE_ISOCHRONOUS = 1,
	USB_TRANSFER_TYPE_BULK = 2,
	USB_TRANSFER_TYPE_INTERRUPT = 3,
} usb_transfer_type_t;

typedef enum {
	USB_SPEED_LOW = 0,
	USB_SPEED_FULL = 1,
	USB_SPEED_HIGH = 2,
	USB_SPEED_SUPER = 3,
} usb_speed_t;


typedef enum {
  USB_CONTROLLER_MODE_DEVICE = 0,
  USB_CONTROLLER_MODE_HOST = 1
} usb_controller_mode_t;

typedef struct {
	const uint8_t* const descriptor;
	const uint32_t number;
	const usb_speed_t speed;
} usb_configuration_t;

// From the EHCI specification, section 3.5
typedef struct ehci_transfer_descriptor ehci_transfer_descriptor_t;
struct ehci_transfer_descriptor {

	// DWord 1/2
	volatile ehci_transfer_descriptor_t *next_dtd_pointer;
	volatile ehci_transfer_descriptor_t *alternate_next_dtd_pointer;

	// DWord 3
	struct {
		uint32_t ping_state_err : 1;
		uint32_t split_transaction_state : 1;
		uint32_t missed_uframe : 1;
		uint32_t transaction_error : 1;
		uint32_t babble : 1;
		uint32_t buffer_error : 1;
		uint32_t halted : 1;
		uint32_t active : 1;

		uint32_t pid_code : 2;
		uint32_t error_counter : 2;
		uint32_t current_page : 3;
		uint32_t int_on_complete : 1;
		uint32_t total_bytes : 15;
		uint32_t data_toggle : 1;
	};

	volatile uint32_t buffer_pointer_page[5];
	volatile uint32_t _reserved;
};


// From Table 3-18 in the EHCI Spec, section 3.6
typedef enum {
	DESCRIPTOR_ITD   = 0,
	DESCRIPTOR_QH    = 1,
	DESCRIPTOR_SITD  = 2,
	DESCRIPTOR_FSTN  = 3
} ehci_data_descriptor_t;

// From the EHCI specificaitons, section 3.1/3.5
typedef union {
	uint32_t link;
	struct  {
		uint32_t terminate : 1;
		uint32_t type      : 2;
		uint32_t           : 29;
	};
} ehci_link_t;


// From the ECHI specification, section 3.6
typedef struct {

	// DWord 1
	ehci_link_t horizontal;

	// DWord 2
	struct {
		uint32_t device_address               : 7;
		uint32_t inactive_on_next_transaction : 1;
		uint32_t endpoint_number              : 4;
		uint32_t endpoint_speed               : 2;
		uint32_t data_toggle_control          : 1;
		uint32_t head_reclamation_flag        : 1;
		uint32_t max_packet_length            : 11;
		uint32_t control_endpoint_flag        : 1;
		uint32_t nak_count_reload             : 4;
	};

	// DWord 3
	struct {
		uint32_t uframe_smask                 : 8;
		uint32_t uframe_cmask                 : 8;
		uint32_t hub_address                  : 7;
		uint32_t port_number                  : 7;
		uint32_t mult                         : 2;
	};

	// Dword 4
	uint32_t current_qtd;

	// Dword 5
	ehci_transfer_descriptor_t overlay;

	// Any custom data we want, here; the hardware won't
	// touch past the end of the structure above.

} __attribute__((packed, aligned(2048))) ehci_queue_head_t;


typedef struct {
	usb_controller_mode_t mode;
	const uint8_t controller;

	union {
		// Device mode fields.
		// TODO: get e.g. descriptor things out of here!
		struct {
			const uint8_t* const descriptor;
			uint8_t** descriptor_strings;
			const uint8_t* const qualifier_descriptor;
			usb_configuration_t* (*configurations)[];
			const usb_configuration_t* configuration;

			usb_queue_head_t queue_heads_device[12] ATTR_ALIGNED(2048);
		};

		// Host mode fields.
		struct {

			// Queue heads
			// TODO: should these be folded into queue_heads below?
			ehci_queue_head_t async_queue_head;

			// TODO: rename me, I'm not really a head?
			ehci_queue_head_t periodic_queue_head;

			// TODO: abstract these counts?
			ehci_queue_head_t queue_heads_host[8];
			ehci_link_t periodic_list[8];
			ehci_transfer_descriptor_t transfer_descriptors[8];

			// TODO: support Isochronous trasfers

		};
	};
} usb_peripheral_t;

typedef struct usb_endpoint_t usb_endpoint_t;
struct usb_endpoint_t {
	usb_setup_t setup;
	uint8_t buffer[8];	// Buffer for use during IN stage.
	const uint_fast8_t address;
	usb_peripheral_t* device;
	usb_endpoint_t* const in;
	usb_endpoint_t* const out;
	void (*setup_complete)(usb_endpoint_t* const endpoint);
	void (*transfer_complete)(usb_endpoint_t* const endpoint);
};

#endif//__USB_TYPE_H__
