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

#ifndef __GREATFET_H__
#define __GREATFET_H__

#include <stdint.h>

#ifdef _WIN32
   #define ADD_EXPORTS
   
  /* You should define ADD_EXPORTS *only* when building the DLL. */
  #ifdef ADD_EXPORTS
    #define ADDAPI __declspec(dllexport)
  #else
    #define ADDAPI __declspec(dllimport)
  #endif

  /* Define calling convention in one place, for convenience. */
  #define ADDCALL __cdecl

#else /* _WIN32 not defined. */

  /* Define with no value on non-Windows OSes. */
  #define ADDAPI
  #define ADDCALL

#endif

enum greatfet_error {
	GREATFET_SUCCESS = 0,
	GREATFET_TRUE = 1,
	GREATFET_ERROR_INVALID_PARAM = -2,
	GREATFET_ERROR_NOT_FOUND = -5,
	GREATFET_ERROR_BUSY = -6,
	GREATFET_ERROR_NO_MEM = -11,
	GREATFET_ERROR_LIBUSB = -1000,
	GREATFET_ERROR_THREAD = -1001,
	GREATFET_ERROR_STREAMING_THREAD_ERR = -1002,
	GREATFET_ERROR_STREAMING_STOPPED = -1003,
	GREATFET_ERROR_STREAMING_EXIT_CALLED = -1004,
	GREATFET_ERROR_OTHER = -9999,
};

enum greatfet_board_id {
	BOARD_ID_AZALEA  = 0,
	BOARD_ID_INVALID = 0xFF,
};

enum greatfet_usb_board_id {
	USB_BOARD_ID_AZALEA = 0x60E6,
	USB_BOARD_ID_INVALID = 0xFFFF,
};

enum rf_path_filter {
	RF_PATH_FILTER_BYPASS = 0,
	RF_PATH_FILTER_LOW_PASS = 1,
	RF_PATH_FILTER_HIGH_PASS = 2,
};

typedef enum {
	TRANSCEIVER_MODE_OFF = 0,
	TRANSCEIVER_MODE_RX = 1,
	TRANSCEIVER_MODE_TX = 2,
	TRANSCEIVER_MODE_SS = 3,
	TRANSCEIVER_MODE_CPLD_UPDATE = 4
} transceiver_mode_t;

typedef struct greatfet_device greatfet_device;

typedef struct {
	greatfet_device* device;
	uint8_t* buffer;
	int buffer_length;
	int valid_length;
	void* rx_ctx;
	void* tx_ctx;
} greatfet_transfer;

typedef struct {
	uint32_t part_id[2];
	uint32_t serial_no[4];
} read_partid_serialno_t;


struct greatfet_device_list {
	char **serial_numbers;
	enum greatfet_usb_board_id *usb_board_ids;
	int *usb_device_index;
	int devicecount;
	
	void **usb_devices;
	int usb_devicecount;
};
typedef struct greatfet_device_list greatfet_device_list_t;

typedef int (*greatfet_sample_block_cb_fn)(greatfet_transfer* transfer);

#ifdef __cplusplus
extern "C"
{
#endif

extern ADDAPI int ADDCALL greatfet_init();
extern ADDAPI int ADDCALL greatfet_exit();

extern ADDAPI greatfet_device_list_t* ADDCALL greatfet_device_list();
extern ADDAPI int ADDCALL greatfet_device_list_open(greatfet_device_list_t *list, int idx, greatfet_device** device);
extern ADDAPI void ADDCALL greatfet_device_list_free(greatfet_device_list_t *list);
 
extern ADDAPI int ADDCALL greatfet_open(greatfet_device** device);
extern ADDAPI int ADDCALL greatfet_open_by_serial(const char* const desired_serial_number, greatfet_device** device);
extern ADDAPI int ADDCALL greatfet_close(greatfet_device* device);
 
extern ADDAPI int ADDCALL greatfet_start_rx(greatfet_device* device, greatfet_sample_block_cb_fn callback, void* rx_ctx);
extern ADDAPI int ADDCALL greatfet_stop_rx(greatfet_device* device);
 
extern ADDAPI int ADDCALL greatfet_start_tx(greatfet_device* device, greatfet_sample_block_cb_fn callback, void* tx_ctx);
extern ADDAPI int ADDCALL greatfet_stop_tx(greatfet_device* device);

/* return GREATFET_TRUE if success */
extern ADDAPI int ADDCALL greatfet_is_streaming(greatfet_device* device);
 
extern ADDAPI int ADDCALL greatfet_max2837_read(greatfet_device* device, uint8_t register_number, uint16_t* value);
extern ADDAPI int ADDCALL greatfet_max2837_write(greatfet_device* device, uint8_t register_number, uint16_t value);
 
extern ADDAPI int ADDCALL greatfet_si5351c_read(greatfet_device* device, uint16_t register_number, uint16_t* value);
extern ADDAPI int ADDCALL greatfet_si5351c_write(greatfet_device* device, uint16_t register_number, uint16_t value);
 
extern ADDAPI int ADDCALL greatfet_set_baseband_filter_bandwidth(greatfet_device* device, const uint32_t bandwidth_hz);
 
extern ADDAPI int ADDCALL greatfet_rffc5071_read(greatfet_device* device, uint8_t register_number, uint16_t* value);
extern ADDAPI int ADDCALL greatfet_rffc5071_write(greatfet_device* device, uint8_t register_number, uint16_t value);
 
extern ADDAPI int ADDCALL greatfet_spiflash_erase(greatfet_device* device);
extern ADDAPI int ADDCALL greatfet_spiflash_write(greatfet_device* device, const uint32_t address, const uint16_t length, unsigned char* const data);
extern ADDAPI int ADDCALL greatfet_spiflash_read(greatfet_device* device, const uint32_t address, const uint16_t length, unsigned char* data);

/* device will need to be reset after greatfet_cpld_write */
extern ADDAPI int ADDCALL greatfet_cpld_write(greatfet_device* device,
		unsigned char* const data, const unsigned int total_length);
		
extern ADDAPI int ADDCALL greatfet_board_id_read(greatfet_device* device, uint8_t* value);
extern ADDAPI int ADDCALL greatfet_version_string_read(greatfet_device* device, char* version, uint8_t length);

extern ADDAPI int ADDCALL greatfet_set_freq(greatfet_device* device, const uint64_t freq_hz);
extern ADDAPI int ADDCALL greatfet_set_freq_explicit(greatfet_device* device,
		const uint64_t if_freq_hz, const uint64_t lo_freq_hz,
		const enum rf_path_filter path);

/* currently 8-20Mhz - either as a fraction, i.e. freq 20000000hz divider 2 -> 10Mhz or as plain old 10000000hz (double)
	preferred rates are 8, 10, 12.5, 16, 20Mhz due to less jitter */
extern ADDAPI int ADDCALL greatfet_set_sample_rate_manual(greatfet_device* device, const uint32_t freq_hz, const uint32_t divider);
extern ADDAPI int ADDCALL greatfet_set_sample_rate(greatfet_device* device, const double freq_hz);

/* external amp, bool on/off */
extern ADDAPI int ADDCALL greatfet_set_amp_enable(greatfet_device* device, const uint8_t value);

extern ADDAPI int ADDCALL greatfet_board_partid_serialno_read(greatfet_device* device, read_partid_serialno_t* read_partid_serialno);

/* range 0-40 step 8d, IF gain in osmosdr  */
extern ADDAPI int ADDCALL greatfet_set_lna_gain(greatfet_device* device, uint32_t value);

/* range 0-62 step 2db, BB gain in osmosdr */
extern ADDAPI int ADDCALL greatfet_set_vga_gain(greatfet_device* device, uint32_t value);

/* range 0-47 step 1db */
extern ADDAPI int ADDCALL greatfet_set_txvga_gain(greatfet_device* device, uint32_t value);

/* antenna port power control */
extern ADDAPI int ADDCALL greatfet_set_antenna_enable(greatfet_device* device, const uint8_t value);

extern ADDAPI const char* ADDCALL greatfet_error_name(enum greatfet_error errcode);
extern ADDAPI const char* ADDCALL greatfet_board_id_name(enum greatfet_board_id board_id);
extern ADDAPI const char* ADDCALL greatfet_usb_board_id_name(enum greatfet_usb_board_id usb_board_id);
extern ADDAPI const char* ADDCALL greatfet_filter_path_name(const enum rf_path_filter path);

/* Compute nearest freq for bw filter (manual filter) */
extern ADDAPI uint32_t ADDCALL greatfet_compute_baseband_filter_bw_round_down_lt(const uint32_t bandwidth_hz);
/* Compute best default value depending on sample rate (auto filter) */
extern ADDAPI uint32_t ADDCALL greatfet_compute_baseband_filter_bw(const uint32_t bandwidth_hz);

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif /*__GREATFET_H__*/
