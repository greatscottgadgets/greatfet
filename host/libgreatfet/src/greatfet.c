/*
Copyright (c) 2012, Jared Boone <jared@sharebrained.com>
Copyright (c) 2013, Benjamin Vernoux <titanmkd@gmail.com>
Copyright (c) 2013, Michael Ossmann <mike@ossmann.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the 
	documentation and/or other materials provided with the distribution.
    Neither the name of Great Scott Gadgets nor the names of its contributors may be used to endorse or promote products derived from this software
	without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "greatfet.h"

#include <stdlib.h>

#include <libusb.h>
#include <pthread.h>

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#ifdef GREATFET_BIG_ENDIAN
#define TO_LE(x) __builtin_bswap32(x)
#define TO_LE64(x) __builtin_bswap64(x)
#else
#define TO_LE(x) x
#define TO_LE64(x) x
#endif

// TODO: Factor this into a shared #include so that firmware can use
// the same values.
typedef enum {
	GREATFET_VENDOR_REQUEST_SET_TRANSCEIVER_MODE = 1,
	GREATFET_VENDOR_REQUEST_MAX2837_WRITE = 2,
	GREATFET_VENDOR_REQUEST_MAX2837_READ = 3,
	GREATFET_VENDOR_REQUEST_SI5351C_WRITE = 4,
	GREATFET_VENDOR_REQUEST_SI5351C_READ = 5,
	GREATFET_VENDOR_REQUEST_SAMPLE_RATE_SET = 6,
	GREATFET_VENDOR_REQUEST_BASEBAND_FILTER_BANDWIDTH_SET = 7,
	GREATFET_VENDOR_REQUEST_RFFC5071_WRITE = 8,
	GREATFET_VENDOR_REQUEST_RFFC5071_READ = 9,
	GREATFET_VENDOR_REQUEST_SPIFLASH_ERASE = 10,
	GREATFET_VENDOR_REQUEST_SPIFLASH_WRITE = 11,
	GREATFET_VENDOR_REQUEST_SPIFLASH_READ = 12,
	GREATFET_VENDOR_REQUEST_BOARD_ID_READ = 14,
	GREATFET_VENDOR_REQUEST_VERSION_STRING_READ = 15,
	GREATFET_VENDOR_REQUEST_SET_FREQ = 16,
	GREATFET_VENDOR_REQUEST_AMP_ENABLE = 17,
	GREATFET_VENDOR_REQUEST_BOARD_PARTID_SERIALNO_READ = 18,
	GREATFET_VENDOR_REQUEST_SET_LNA_GAIN = 19,
	GREATFET_VENDOR_REQUEST_SET_VGA_GAIN = 20,
	GREATFET_VENDOR_REQUEST_SET_TXVGA_GAIN = 21,
	GREATFET_VENDOR_REQUEST_ANTENNA_ENABLE = 23,
	GREATFET_VENDOR_REQUEST_SET_FREQ_EXPLICIT = 24,
} greatfet_vendor_request;

typedef enum {
	USB_CONFIG_STANDARD = 0x1,
	USB_CONFIG_CPLD_UPDATE  = 0x2,
} greatfet_usb_configurations;

typedef enum {
	GREATFET_TRANSCEIVER_MODE_OFF = 0,
	GREATFET_TRANSCEIVER_MODE_RECEIVE = 1,
	GREATFET_TRANSCEIVER_MODE_TRANSMIT = 2,
} greatfet_transceiver_mode;

struct greatfet_device {
	libusb_device_handle* usb_device;
	struct libusb_transfer** transfers;
	greatfet_sample_block_cb_fn callback;
	volatile bool transfer_thread_started; /* volatile shared between threads (read only) */
	pthread_t transfer_thread;
	uint32_t transfer_count;
	uint32_t buffer_size;
	volatile bool streaming; /* volatile shared between threads (read only) */
	void* rx_ctx;
	void* tx_ctx;
};

typedef struct {
	uint32_t bandwidth_hz;
} max2837_ft_t;

static const max2837_ft_t max2837_ft[] = {
	{ 1750000  },
	{ 2500000  },
	{ 3500000  },
	{ 5000000  },
	{ 5500000  },
	{ 6000000  },
	{ 7000000  },
	{ 8000000  },
	{ 9000000  },
	{ 10000000 },
	{ 12000000 },
	{ 14000000 },
	{ 15000000 },
	{ 20000000 },
	{ 24000000 },
	{ 28000000 },
	{ 0        }
};

volatile bool do_exit = false;

static const uint16_t greatfet_usb_vid = 0x1d50;
static const uint16_t greatfet_azalea_usb_pid = 0x60E6;

static libusb_context* g_libusb_context = NULL;

static void request_exit(void)
{
	do_exit = true;
}

static int cancel_transfers(greatfet_device* device)
{
	uint32_t transfer_index;

	if( device->transfers != NULL )
	{
		for(transfer_index=0; transfer_index<device->transfer_count; transfer_index++)
		{
			if( device->transfers[transfer_index] != NULL )
			{
				libusb_cancel_transfer(device->transfers[transfer_index]);
			}
		}
		return GREATFET_SUCCESS;
	} else {
		return GREATFET_ERROR_OTHER;
	}
}

static int free_transfers(greatfet_device* device)
{
	uint32_t transfer_index;

	if( device->transfers != NULL )
	{
		// libusb_close() should free all transfers referenced from this array.
		for(transfer_index=0; transfer_index<device->transfer_count; transfer_index++)
		{
			if( device->transfers[transfer_index] != NULL )
			{
				libusb_free_transfer(device->transfers[transfer_index]);
				device->transfers[transfer_index] = NULL;
			}
		}
		free(device->transfers);
		device->transfers = NULL;
	}
	return GREATFET_SUCCESS;
}

static int allocate_transfers(greatfet_device* const device)
{
	if( device->transfers == NULL )
	{
		uint32_t transfer_index;
		device->transfers = (struct libusb_transfer**) calloc(device->transfer_count, sizeof(struct libusb_transfer));
		if( device->transfers == NULL )
		{
			return GREATFET_ERROR_NO_MEM;
		}

		for(transfer_index=0; transfer_index<device->transfer_count; transfer_index++)
		{
			device->transfers[transfer_index] = libusb_alloc_transfer(0);
			if( device->transfers[transfer_index] == NULL )
			{
				return GREATFET_ERROR_LIBUSB;
			}

			libusb_fill_bulk_transfer(
				device->transfers[transfer_index],
				device->usb_device,
				0,
				(unsigned char*)malloc(device->buffer_size),
				device->buffer_size,
				NULL,
				device,
				0
			);

			if( device->transfers[transfer_index]->buffer == NULL )
			{
				return GREATFET_ERROR_NO_MEM;
			}
		}
		return GREATFET_SUCCESS;
	} else {
		return GREATFET_ERROR_BUSY;
	}
}

static int prepare_transfers(
	greatfet_device* device,
	const uint_fast8_t endpoint_address,
	libusb_transfer_cb_fn callback)
{
	int error;
	uint32_t transfer_index;
	if( device->transfers != NULL )
	{
		for(transfer_index=0; transfer_index<device->transfer_count; transfer_index++)
		{
			device->transfers[transfer_index]->endpoint = endpoint_address;
			device->transfers[transfer_index]->callback = callback;

			error = libusb_submit_transfer(device->transfers[transfer_index]);
			if( error != 0 )
			{
				return GREATFET_ERROR_LIBUSB;
			}
		}
		return GREATFET_SUCCESS;
	} else {
		// This shouldn't happen.
		return GREATFET_ERROR_OTHER;
	}
}

static int detach_kernel_drivers(libusb_device_handle* usb_device_handle)
{
	int i, num_interfaces, result;
	libusb_device* dev;
	struct libusb_config_descriptor* config;

	dev = libusb_get_device(usb_device_handle);
	result = libusb_get_active_config_descriptor(dev, &config);
	if( result < 0 )
	{
		return GREATFET_ERROR_LIBUSB;
	}

	num_interfaces = config->bNumInterfaces;
	libusb_free_config_descriptor(config);
	for(i=0; i<num_interfaces; i++)
	{
		result = libusb_kernel_driver_active(usb_device_handle, i);
		if( result < 0 )
		{
			if( result == LIBUSB_ERROR_NOT_SUPPORTED ) {
				return 0;
			}
			return GREATFET_ERROR_LIBUSB;
		} else if( result == 1 ) {
			result = libusb_detach_kernel_driver(usb_device_handle, i);
			if( result != 0 )
			{
				return GREATFET_ERROR_LIBUSB;
			}
		}
	}
	return GREATFET_SUCCESS;
}

static int set_greatfet_configuration(libusb_device_handle* usb_device, int config)
{
	int result, curr_config;
	result = libusb_get_configuration(usb_device, &curr_config);
	if( result != 0 )
	{
		return GREATFET_ERROR_LIBUSB;
	}

	if(curr_config != config)
	{
		result = detach_kernel_drivers(usb_device);
		if( result != 0 )
		{
			return result;
		}
		result = libusb_set_configuration(usb_device, config);
		if( result != 0 )
		{
			return GREATFET_ERROR_LIBUSB;
		}
	}

	result = detach_kernel_drivers(usb_device);
	if( result != 0 )
	{
		return result;
	}
	return LIBUSB_SUCCESS;
}

#ifdef __cplusplus
extern "C"
{
#endif

int ADDCALL greatfet_init(void)
{
	int libusb_error;
	if (g_libusb_context != NULL) {
		return GREATFET_SUCCESS;
	}
	
	libusb_error = libusb_init(&g_libusb_context);
	if( libusb_error != 0 )
	{
		return GREATFET_ERROR_LIBUSB;
	} else {
		return GREATFET_SUCCESS;
	}
}

int ADDCALL greatfet_exit(void)
{
	if( g_libusb_context != NULL )
	{
		libusb_exit(g_libusb_context);
		g_libusb_context = NULL;
	}

	return GREATFET_SUCCESS;
}

#include <stdio.h>
#include <string.h>

greatfet_device_list_t* ADDCALL greatfet_device_list()
{
	ssize_t i;
	libusb_device_handle* usb_device = NULL;
	uint_fast8_t serial_descriptor_index;
	char serial_number[64];
	int serial_number_length;

	greatfet_device_list_t* list = calloc(1, sizeof(*list));
	if ( list == NULL )
		return NULL;
		
	list->usb_devicecount = libusb_get_device_list(g_libusb_context, (libusb_device ***)&list->usb_devices);
	
	list->serial_numbers = calloc(list->usb_devicecount, sizeof(void *));
	list->usb_board_ids = calloc(list->usb_devicecount, sizeof(enum greatfet_usb_board_id));
	list->usb_device_index = calloc(list->usb_devicecount, sizeof(int));
	
	if ( list->serial_numbers == NULL || list->usb_board_ids == NULL || list->usb_device_index == NULL) {
		greatfet_device_list_free(list);
		return NULL;
	}
	
	for (i=0; i<list->usb_devicecount; i++) {
		struct libusb_device_descriptor device_descriptor;
		libusb_get_device_descriptor(list->usb_devices[i], &device_descriptor);
		
		printf("vid (0x%x) pid (0x%x)\n", device_descriptor.idVendor, device_descriptor.idProduct);
		if( device_descriptor.idVendor == greatfet_usb_vid ) {
			printf("Matched vendor\n");
			if((device_descriptor.idProduct == greatfet_azalea_usb_pid)) {
				printf("Matched board\n");
				int idx = list->devicecount++;
				list->usb_board_ids[idx] = device_descriptor.idProduct;
				list->usb_device_index[idx] = i;
				
				serial_descriptor_index = device_descriptor.iSerialNumber;
				if( serial_descriptor_index > 0 ) {
					printf("Got serial description\n");
					int device_open_rv = -1;
					if( (device_open_rv = libusb_open(list->usb_devices[i], &usb_device)) != 0 ) {
						printf("Couldn't open device using libusb_open: error (%d)\n", device_open_rv);
						usb_device = NULL;
						continue;
					}
					printf("Getting serial number\n");
					serial_number_length = libusb_get_string_descriptor_ascii(usb_device, serial_descriptor_index, (unsigned char*)serial_number, sizeof(serial_number));
					if( serial_number_length == 32 ) {
						serial_number[32] = 0;
						list->serial_numbers[idx] = strdup(serial_number);
					}
					
					libusb_close(usb_device);
					usb_device = NULL;
				}
			}
		}
	}
	
	return list;
}

void ADDCALL greatfet_device_list_free(greatfet_device_list_t *list)
{
	int i;
	
	libusb_free_device_list((libusb_device **)list->usb_devices, 1);
	
	for (i = 0; i < list->devicecount; i++) {
		if (list->serial_numbers[i])
			free(list->serial_numbers[i]);
	}
	
	free(list->serial_numbers);
	free(list->usb_board_ids);
	free(list->usb_device_index);
	free(list);
}

libusb_device_handle* greatfet_open_usb(const char* const desired_serial_number)
{
	libusb_device_handle* usb_device = NULL;
	libusb_device** devices = NULL;
	const ssize_t list_length = libusb_get_device_list(g_libusb_context, &devices);
	int match_len = 0;
	ssize_t i;
	char serial_number[64];
	int serial_number_length;
	
	printf("Number of USB devices: %ld\n", list_length);
	
	if( desired_serial_number ) {
		/* If a shorter serial number is specified, only match against the suffix.
		 * Should probably complain if the match is not unique, currently doesn't.
		 */
		match_len = strlen(desired_serial_number);
		if ( match_len > 32 )
			return NULL;
	}
	
	for (i=0; i<list_length; i++) {
		struct libusb_device_descriptor device_descriptor;
		libusb_get_device_descriptor(devices[i], &device_descriptor);
		
		if( device_descriptor.idVendor == greatfet_usb_vid ) {
			if((device_descriptor.idProduct == greatfet_azalea_usb_pid)) {
				printf("USB device %4x:%4x:", device_descriptor.idVendor, device_descriptor.idProduct);
				
				if( desired_serial_number != NULL ) {
					const uint_fast8_t serial_descriptor_index = device_descriptor.iSerialNumber;
					if( serial_descriptor_index > 0 ) {
						if( libusb_open(devices[i], &usb_device) != 0 ) {
							usb_device = NULL;
							continue;
						}
						serial_number_length = libusb_get_string_descriptor_ascii(usb_device, serial_descriptor_index, (unsigned char*)serial_number, sizeof(serial_number));
						if( serial_number_length == 32 ) {
							serial_number[32] = 0;
							printf(" %s", serial_number);
							if( strncmp(serial_number + 32-match_len, desired_serial_number, match_len) == 0 ) {
								printf(" match\n");
								break;
							} else {
								printf(" skip\n");
								libusb_close(usb_device);
								usb_device = NULL;
							}
						} else {
							printf(" wrong length of serial number: %d\n", serial_number_length);
							libusb_close(usb_device);
							usb_device = NULL;
						}
					}
				} else {
					printf(" default\n");
					libusb_open(devices[i], &usb_device);
					break;
				}
			}
		}
	}
	
	libusb_free_device_list(devices, 1);
	
	return usb_device;
}

static int greatfet_open_setup(libusb_device_handle* usb_device, greatfet_device** device)
{
	int result;
	greatfet_device* lib_device;

	//int speed = libusb_get_device_speed(usb_device);
	// TODO: Error or warning if not high speed USB?
	
	result = set_greatfet_configuration(usb_device, USB_CONFIG_STANDARD);
	if( result != LIBUSB_SUCCESS )
	{
		libusb_close(usb_device);
		return result;
	}

	result = libusb_claim_interface(usb_device, 0);
	if( result != LIBUSB_SUCCESS )
	{
		libusb_close(usb_device);
		return GREATFET_ERROR_LIBUSB;
	}

	lib_device = NULL;
	lib_device = (greatfet_device*)malloc(sizeof(*lib_device));
	if( lib_device == NULL )
	{
		libusb_release_interface(usb_device, 0);
		libusb_close(usb_device);
		return GREATFET_ERROR_NO_MEM;
	}

	lib_device->usb_device = usb_device;
	lib_device->transfers = NULL;
	lib_device->callback = NULL;
	lib_device->transfer_thread_started = false;
	/*
	lib_device->transfer_count = 1024;
	lib_device->buffer_size = 16384;
	*/
	lib_device->transfer_count = 4;
	lib_device->buffer_size = 262144; /* 1048576; */
	lib_device->streaming = false;
	do_exit = false;

	result = allocate_transfers(lib_device);
	if( result != 0 )
	{
		free(lib_device);
		libusb_release_interface(usb_device, 0);
		libusb_close(usb_device);
		return GREATFET_ERROR_NO_MEM;
	}

	*device = lib_device;

	return GREATFET_SUCCESS;
}

int ADDCALL greatfet_open(greatfet_device** device)
{
	libusb_device_handle* usb_device;
	
	if( device == NULL )
	{
		return GREATFET_ERROR_INVALID_PARAM;
	}
	
        printf("Opening device with vid (%x) and pid (%x)\n", greatfet_usb_vid, greatfet_azalea_usb_pid);

	usb_device = libusb_open_device_with_vid_pid(g_libusb_context, greatfet_usb_vid, greatfet_azalea_usb_pid);
	
	if( usb_device == NULL )
	{
		return GREATFET_ERROR_NOT_FOUND;
	}
	
	return greatfet_open_setup(usb_device, device);
}

int ADDCALL greatfet_open_by_serial(const char* const desired_serial_number, greatfet_device** device)
{
	libusb_device_handle* usb_device;
	
	if( desired_serial_number == NULL )
	{
		return greatfet_open(device);
	}
	
	if( device == NULL )
	{
		return GREATFET_ERROR_INVALID_PARAM;
	}
	
	usb_device = greatfet_open_usb(desired_serial_number);
	
	if( usb_device == NULL )
	{
		return GREATFET_ERROR_NOT_FOUND;
	}
	
	return greatfet_open_setup(usb_device, device);
}

int ADDCALL greatfet_device_list_open(greatfet_device_list_t *list, int idx, greatfet_device** device)
{
	libusb_device_handle* usb_device;
	int i;
	
	if( device == NULL || list == NULL || idx < 0 || idx >= list->devicecount )
	{
		return GREATFET_ERROR_INVALID_PARAM;
	}
	
	i = list->usb_device_index[idx];

	if( libusb_open(list->usb_devices[i], &usb_device) != 0 ) {
		usb_device = NULL;
		return GREATFET_ERROR_LIBUSB;
	}
	
	return greatfet_open_setup(usb_device, device);
}

int ADDCALL greatfet_set_transceiver_mode(greatfet_device* device, greatfet_transceiver_mode value)
{
	int result;
	result = libusb_control_transfer(
		device->usb_device,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		GREATFET_VENDOR_REQUEST_SET_TRANSCEIVER_MODE,
		value,
		0,
		NULL,
		0,
		0
	);

	if( result != 0 )
	{
		return GREATFET_ERROR_LIBUSB;
	} else {
		return GREATFET_SUCCESS;
	}
}

int ADDCALL greatfet_max2837_read(greatfet_device* device, uint8_t register_number, uint16_t* value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_max2837_write(greatfet_device* device, uint8_t register_number, uint16_t value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_si5351c_read(greatfet_device* device, uint16_t register_number, uint16_t* value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_si5351c_write(greatfet_device* device, uint16_t register_number, uint16_t value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_set_baseband_filter_bandwidth(greatfet_device* device, const uint32_t bandwidth_hz)
{
		return GREATFET_SUCCESS;
}


int ADDCALL greatfet_rffc5071_read(greatfet_device* device, uint8_t register_number, uint16_t* value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_rffc5071_write(greatfet_device* device, uint8_t register_number, uint16_t value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_spiflash_erase(greatfet_device* device)
{
	int result;
	result = libusb_control_transfer(
		device->usb_device,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		GREATFET_VENDOR_REQUEST_SPIFLASH_ERASE,
		0,
		0,
		NULL,
		0,
		0
	);

	if (result != 0)
	{
		return GREATFET_ERROR_LIBUSB;
	} else {
		return GREATFET_SUCCESS;
	}
}

int ADDCALL greatfet_spiflash_write(greatfet_device* device, const uint32_t address,
		const uint16_t length, unsigned char* const data)
{
	int result;
	
	if (address > 0x0FFFFF)
	{
		return GREATFET_ERROR_INVALID_PARAM;
	}

	result = libusb_control_transfer(
		device->usb_device,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		GREATFET_VENDOR_REQUEST_SPIFLASH_WRITE,
		address >> 16,
		address & 0xFFFF,
		data,
		length,
		0
	);

	if (result < length)
	{
		return GREATFET_ERROR_LIBUSB;
	} else {
		return GREATFET_SUCCESS;
	}
}

int ADDCALL greatfet_spiflash_read(greatfet_device* device, const uint32_t address,
		const uint16_t length, unsigned char* data)
{
	int result;
	
	if (address > 0x0FFFFF)
	{
		return GREATFET_ERROR_INVALID_PARAM;
	}

	result = libusb_control_transfer(
		device->usb_device,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		GREATFET_VENDOR_REQUEST_SPIFLASH_READ,
		address >> 16,
		address & 0xFFFF,
		data,
		length,
		0
	);

	if (result < length)
	{
		return GREATFET_ERROR_LIBUSB;
	} else {
		return GREATFET_SUCCESS;
	}
}

int ADDCALL greatfet_cpld_write(greatfet_device* device,
		unsigned char* const data, const unsigned int total_length)
{
	return GREATFET_SUCCESS;
}

int ADDCALL greatfet_board_id_read(greatfet_device* device, uint8_t* value)
{
	int result;
	result = libusb_control_transfer(
		device->usb_device,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		GREATFET_VENDOR_REQUEST_BOARD_ID_READ,
		0,
		0,
		value,
		1,
		0
	);

	if (result < 1)
	{
		return GREATFET_ERROR_LIBUSB;
	} else {
		return GREATFET_SUCCESS;
	}
}

int ADDCALL greatfet_version_string_read(greatfet_device* device, char* version,
		uint8_t length)
{
	int result;
	result = libusb_control_transfer(
		device->usb_device,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		GREATFET_VENDOR_REQUEST_VERSION_STRING_READ,
		0,
		0,
		(unsigned char*)version,
		length,
		0
	);

	if (result < 0)
	{
		return GREATFET_ERROR_LIBUSB;
	} else {
		version[result] = '\0';
		return GREATFET_SUCCESS;
	}
}

typedef struct {
	uint32_t freq_mhz; /* From 0 to 6000+MHz */
	uint32_t freq_hz; /* From 0 to 999999Hz */
	/* Final Freq = freq_mhz+freq_hz */
} set_freq_params_t;
#define FREQ_ONE_MHZ	(1000*1000ull)

int ADDCALL greatfet_set_freq(greatfet_device* device, const uint64_t freq_hz)
{
		return GREATFET_SUCCESS;
}

struct set_freq_explicit_params {
	uint64_t if_freq_hz; /* intermediate frequency */
	uint64_t lo_freq_hz; /* front-end local oscillator frequency */
	uint8_t path;        /* image rejection filter path */
};

int ADDCALL greatfet_set_freq_explicit(greatfet_device* device,
		const uint64_t if_freq_hz, const uint64_t lo_freq_hz,
		const enum rf_path_filter path)
{
		return GREATFET_SUCCESS;
}

typedef struct {
	uint32_t freq_hz;
	uint32_t divider;
} set_fracrate_params_t;


int ADDCALL greatfet_set_sample_rate_manual(greatfet_device* device,
                                       const uint32_t freq_hz, uint32_t divider)
{
	set_fracrate_params_t set_fracrate_params;
	uint8_t length;
	int result;

	set_fracrate_params.freq_hz = TO_LE(freq_hz);
	set_fracrate_params.divider = TO_LE(divider);
	length = sizeof(set_fracrate_params_t);

	result = libusb_control_transfer(
		device->usb_device,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		GREATFET_VENDOR_REQUEST_SAMPLE_RATE_SET,
		0,
		0,
		(unsigned char*)&set_fracrate_params,
		length,
		0
	);

	if (result < length)
	{
		return GREATFET_ERROR_LIBUSB;
	} else {
		return GREATFET_SUCCESS;
	}
}

int ADDCALL greatfet_set_sample_rate(greatfet_device* device, const double freq)
{
	const int MAX_N = 32;
	uint32_t freq_hz, divider;
	double freq_frac = 1.0 + freq - (int)freq;
	uint64_t a, m;
	int i, e;

	union {
		uint64_t u64;
		double d;
	} v;
	v.d = freq;

	e = (v.u64 >> 52) - 1023;

	m = ((1ULL << 52) - 1);

	v.d = freq_frac;
	v.u64 &= m;

	m &= ~((1 << (e+4)) - 1);

	a = 0;

	for (i=1; i<MAX_N; i++) {
		a += v.u64;
		if (!(a & m) || !(~a & m))
			break;
	}

	if (i == MAX_N)
		i = 1;

	freq_hz = (uint32_t)(freq * i + 0.5);
	divider = i;

	return greatfet_set_sample_rate_manual(device, freq_hz, divider);
}

int ADDCALL greatfet_set_amp_enable(greatfet_device* device, const uint8_t value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_board_partid_serialno_read(greatfet_device* device, read_partid_serialno_t* read_partid_serialno)
{
	uint8_t length;
	int result;
	
	length = sizeof(read_partid_serialno_t);
	result = libusb_control_transfer(
		device->usb_device,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		GREATFET_VENDOR_REQUEST_BOARD_PARTID_SERIALNO_READ,
		0,
		0,
		(unsigned char*)read_partid_serialno,
		length,
		0
	);

	if (result < length)
	{
		return GREATFET_ERROR_LIBUSB;
	} else {

		read_partid_serialno->part_id[0] = TO_LE(read_partid_serialno->part_id[0]);
		read_partid_serialno->part_id[1] = TO_LE(read_partid_serialno->part_id[1]);
		read_partid_serialno->serial_no[0] = TO_LE(read_partid_serialno->serial_no[0]);
		read_partid_serialno->serial_no[1] = TO_LE(read_partid_serialno->serial_no[1]);
		read_partid_serialno->serial_no[2] = TO_LE(read_partid_serialno->serial_no[2]);
		read_partid_serialno->serial_no[3] = TO_LE(read_partid_serialno->serial_no[3]);

		return GREATFET_SUCCESS;
	}
}

int ADDCALL greatfet_set_lna_gain(greatfet_device* device, uint32_t value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_set_vga_gain(greatfet_device* device, uint32_t value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_set_txvga_gain(greatfet_device* device, uint32_t value)
{
		return GREATFET_SUCCESS;
}

int ADDCALL greatfet_set_antenna_enable(greatfet_device* device, const uint8_t value)
{
		return GREATFET_SUCCESS;
}

static void* transfer_threadproc(void* arg)
{
	greatfet_device* device = (greatfet_device*)arg;
	int error;
	struct timeval timeout = { 0, 500000 };

	while( (device->streaming) && (do_exit == false) )
	{
		error = libusb_handle_events_timeout(g_libusb_context, &timeout);
		if( (error != 0) && (error != LIBUSB_ERROR_INTERRUPTED) )
		{
			device->streaming = false;
		}
	}

	return NULL;
}

static void greatfet_libusb_transfer_callback(struct libusb_transfer* usb_transfer)
{
	greatfet_device* device = (greatfet_device*)usb_transfer->user_data;

	if(usb_transfer->status == LIBUSB_TRANSFER_COMPLETED)
	{
		greatfet_transfer transfer = {
			transfer.device = device,
			transfer.buffer = usb_transfer->buffer,
			transfer.buffer_length = usb_transfer->length,
			transfer.valid_length = usb_transfer->actual_length,
			transfer.rx_ctx = device->rx_ctx,
			transfer.tx_ctx = device->tx_ctx
		};

		if( device->callback(&transfer) == 0 )
		{
			if( libusb_submit_transfer(usb_transfer) < 0)
			{
				request_exit();
			}else {
				return;
			}
		}else {
			request_exit();
		}
	} else {
		/* Other cases LIBUSB_TRANSFER_NO_DEVICE
		LIBUSB_TRANSFER_ERROR, LIBUSB_TRANSFER_TIMED_OUT
		LIBUSB_TRANSFER_STALL,	LIBUSB_TRANSFER_OVERFLOW
		LIBUSB_TRANSFER_CANCELLED ...
		*/
		request_exit(); /* Fatal error stop transfer */
	}
}

static int kill_transfer_thread(greatfet_device* device)
{
	void* value;
	int result;
	
	request_exit();

	if( device->transfer_thread_started != false )
	{
		value = NULL;
		result = pthread_join(device->transfer_thread, &value);
		if( result != 0 )
		{
			return GREATFET_ERROR_THREAD;
		}
		device->transfer_thread_started = false;

		/* Cancel all transfers */
		cancel_transfers(device);
	}

	return GREATFET_SUCCESS;
}

static int create_transfer_thread(greatfet_device* device,
									const uint8_t endpoint_address,
									greatfet_sample_block_cb_fn callback)
{
	int result;
	
	if( device->transfer_thread_started == false )
	{
		device->streaming = false;

		result = prepare_transfers(
			device, endpoint_address,
			(libusb_transfer_cb_fn)greatfet_libusb_transfer_callback
		);

		if( result != GREATFET_SUCCESS )
		{
			return result;
		}

		device->streaming = true;
		device->callback = callback;
		result = pthread_create(&device->transfer_thread, 0, transfer_threadproc, device);
		if( result == 0 )
		{
			device->transfer_thread_started = true;
		}else {
			return GREATFET_ERROR_THREAD;
		}
	} else {
		return GREATFET_ERROR_BUSY;
	}

	return GREATFET_SUCCESS;
}

int ADDCALL greatfet_is_streaming(greatfet_device* device)
{
	/* return greatfet is streaming only when streaming, transfer_thread_started are true and do_exit equal false */
	
	if( (device->transfer_thread_started == true) &&
		(device->streaming == true) && 
		(do_exit == false) )
	{
		return GREATFET_TRUE;
	} else {
	
		if(device->transfer_thread_started == false)
		{
			return GREATFET_ERROR_STREAMING_THREAD_ERR;
		}

		if(device->streaming == false)
		{
			return GREATFET_ERROR_STREAMING_STOPPED;
		}

		return GREATFET_ERROR_STREAMING_EXIT_CALLED;
	}
}

int ADDCALL greatfet_start_rx(greatfet_device* device, greatfet_sample_block_cb_fn callback, void* rx_ctx)
{
	int result;
	const uint8_t endpoint_address = LIBUSB_ENDPOINT_IN | 1;
	result = greatfet_set_transceiver_mode(device, GREATFET_TRANSCEIVER_MODE_RECEIVE);
	if( result == GREATFET_SUCCESS )
	{
		device->rx_ctx = rx_ctx;
		result = create_transfer_thread(device, endpoint_address, callback);
	}
	return result;
}

int ADDCALL greatfet_stop_rx(greatfet_device* device)
{
	int result;
	result = greatfet_set_transceiver_mode(device, GREATFET_TRANSCEIVER_MODE_OFF);
	if (result != GREATFET_SUCCESS)
	{
		return result;
	}
	return kill_transfer_thread(device);
}

int ADDCALL greatfet_start_tx(greatfet_device* device, greatfet_sample_block_cb_fn callback, void* tx_ctx)
{
	int result;
	const uint8_t endpoint_address = LIBUSB_ENDPOINT_OUT | 2;
	result = greatfet_set_transceiver_mode(device, GREATFET_TRANSCEIVER_MODE_TRANSMIT);
	if( result == GREATFET_SUCCESS )
	{
		device->tx_ctx = tx_ctx;
		result = create_transfer_thread(device, endpoint_address, callback);
	}
	return result;
}

int ADDCALL greatfet_stop_tx(greatfet_device* device)
{
	int result1, result2;
	result1 = kill_transfer_thread(device);
	result2 = greatfet_set_transceiver_mode(device, GREATFET_TRANSCEIVER_MODE_OFF);
	if (result2 != GREATFET_SUCCESS)
	{
		return result2;
	}
	return result1;
}

int ADDCALL greatfet_close(greatfet_device* device)
{
	int result1, result2;

	result1 = GREATFET_SUCCESS;
	result2 = GREATFET_SUCCESS;
	
	if( device != NULL )
	{
		result1 = greatfet_stop_rx(device);
		result2 = greatfet_stop_tx(device);
		if( device->usb_device != NULL )
		{
			libusb_release_interface(device->usb_device, 0);
			libusb_close(device->usb_device);
			device->usb_device = NULL;
		}

		free_transfers(device);

		free(device);
	}

	if (result2 != GREATFET_SUCCESS)
	{
		return result2;
	}
	return result1;
}

const char* ADDCALL greatfet_error_name(enum greatfet_error errcode)
{
	switch(errcode)
	{
	case GREATFET_SUCCESS:
		return "GREATFET_SUCCESS";

	case GREATFET_TRUE:
		return "GREATFET_TRUE";

	case GREATFET_ERROR_INVALID_PARAM:
		return "GREATFET_ERROR_INVALID_PARAM";

	case GREATFET_ERROR_NOT_FOUND:
		return "GREATFET_ERROR_NOT_FOUND";

	case GREATFET_ERROR_BUSY:
		return "GREATFET_ERROR_BUSY";

	case GREATFET_ERROR_NO_MEM:
		return "GREATFET_ERROR_NO_MEM";

	case GREATFET_ERROR_LIBUSB:
		return "GREATFET_ERROR_LIBUSB";

	case GREATFET_ERROR_THREAD:
		return "GREATFET_ERROR_THREAD";

	case GREATFET_ERROR_STREAMING_THREAD_ERR:
		return "GREATFET_ERROR_STREAMING_THREAD_ERR";

	case GREATFET_ERROR_STREAMING_STOPPED:
		return "GREATFET_ERROR_STREAMING_STOPPED";

	case GREATFET_ERROR_STREAMING_EXIT_CALLED:
		return "GREATFET_ERROR_STREAMING_EXIT_CALLED";

	case GREATFET_ERROR_OTHER:
		return "GREATFET_ERROR_OTHER";

	default:
		return "GREATFET unknown error";
	}
}

const char* ADDCALL greatfet_board_id_name(enum greatfet_board_id board_id)
{
	switch(board_id)
	{
	case BOARD_ID_AZALEA:
		return "Azalea";

	case BOARD_ID_INVALID:
		return "Invalid Board ID";

	default:
		return "Unknown Board ID";
	}
}

extern ADDAPI const char* ADDCALL greatfet_usb_board_id_name(enum greatfet_usb_board_id usb_board_id)
{
	switch(usb_board_id)
	{
	case USB_BOARD_ID_AZALEA:
		return "Azalea";

	case USB_BOARD_ID_INVALID:
		return "Invalid Board ID";

	default:
		return "Unknown Board ID";
	}
}

const char* ADDCALL greatfet_filter_path_name(const enum rf_path_filter path)
{
	switch(path) {
	case RF_PATH_FILTER_BYPASS:
		return "mixer bypass";
	case RF_PATH_FILTER_LOW_PASS:
		return "low pass filter";
	case RF_PATH_FILTER_HIGH_PASS:
		return "high pass filter";
	default:
		return "invalid filter path";
	}
}

/* Return final bw round down and less than expected bw. */
uint32_t ADDCALL greatfet_compute_baseband_filter_bw_round_down_lt(const uint32_t bandwidth_hz)
{
	const max2837_ft_t* p = max2837_ft;
	while( p->bandwidth_hz != 0 )
	{
		if( p->bandwidth_hz >= bandwidth_hz )
		{
			break;
		}
		p++;
	}
	/* Round down (if no equal to first entry) */
	if(p != max2837_ft)
	{
		p--;
	}
	return p->bandwidth_hz;
}

/* Return final bw. */
uint32_t ADDCALL greatfet_compute_baseband_filter_bw(const uint32_t bandwidth_hz)
{
	const max2837_ft_t* p = max2837_ft;
	while( p->bandwidth_hz != 0 )
	{
		if( p->bandwidth_hz >= bandwidth_hz )
		{
			break;
		}
		p++;
	}

	/* Round down (if no equal to first entry) and if > bandwidth_hz */
	if(p != max2837_ft)
	{
		if(p->bandwidth_hz > bandwidth_hz)
			p--;
	}

	return p->bandwidth_hz;
}

#ifdef __cplusplus
} // __cplusplus defined.
#endif

