/*
 * This file is part of GreatFET
 * direct port from GoodFET
 * Glory / failures go to Travis
 */

#ifndef __JTAG_H__
#define __JTAG_H__

#include <gpio.h>
#include <gpio_lpc.h>
#include <stdint.h>

// All states in the JTAG TAP
enum eTAPState
{
	UNKNOWN				= 0x0000,
	TEST_LOGIC_RESET	= 0x0001,
	RUN_TEST_IDLE		= 0x0002,
	SELECT_DR_SCAN		= 0x0004,
	CAPTURE_DR			= 0x0008,
	SHIFT_DR			= 0x0010,
	EXIT1_DR			= 0x0020,
	PAUSE_DR			= 0x0040,
	EXIT2_DR			= 0x0080,
	UPDATE_DR			= 0x0100,
	SELECT_IR_SCAN		= 0x0200,
	CAPTURE_IR			= 0x0400,
	SHIFT_IR			= 0x0800,
	EXIT1_IR			= 0x1000,
	PAUSE_IR			= 0x2000,
	EXIT2_IR			= 0x4000,
	UPDATE_IR			= 0x8000
};

extern unsigned char jtagid;

//! the global state of the JTAG TAP
extern enum eTAPState jtag_state;

//! Returns true if we're in any of the data register states
int in_dr();
//! Returns true if we're in any of the instruction register states
int in_ir();
//! Returns true if we're in run-test-idle state
int in_run_test_idle();
//! Check the state
int in_state(enum eTAPState state);

//! jtag_trans_n flags
enum eTransFlags
{
	MSB					= 0x0,
	LSB					= 0x1,
	NOEND				= 0x2,
	NORETIDLE			= 0x4
};

static struct gpio_t tdo = GPIO(0, 10);
static struct gpio_t tdi = GPIO(0, 11);
static struct gpio_t tms = GPIO(0, 15);
static struct gpio_t tck = GPIO(1, 14);
static struct gpio_t rst = GPIO(2, 3);
static struct gpio_t tst = GPIO(2, 2);

#define SETTDI gpio_write(&tdi, 1)
#define CLRTDI gpio_write(&tdi, 0)
#define READTDO gpio_read(&tdo)
#define SETTMS gpio_write(&tms, 1)
#define CLRTMS gpio_write(&tms, 0)
#define SETTCK gpio_write(&tck, 1)
#define CLRTCK gpio_write(&tck, 0)

#define SETTST gpio_write(&tst, 1)
#define CLRTST gpio_write(&tst, 0)
#define SETRST gpio_write(&rst, 1)
#define CLRRST gpio_write(&rst, 0)

#define SETTCLK SETTDI
#define CLRTCLK CLRTDI

//! Shift n bytes.
uint32_t jtag_trans_n(uint32_t word, 
					  uint8_t bitcount, 
					  enum eTransFlags flags);
//! Shift 8 bits in/out of selected register
uint8_t jtag_trans_8(uint8_t in);
//! Shift 16 bits in/out of selected register
uint16_t jtag_trans_16(uint16_t in);
//! Shift 8 bits of the IR.
uint8_t jtag_ir_shift_8(uint8_t in);
//! Shift 16 bits of the DR.
uint16_t jtag_dr_shift_16(uint16_t in);
//! Stop JTAG, release pins
void jtag_stop(void);
//! Setup the JTAG pin directions.
void jtag_setup(void);
//! Ratchet Clock Down and Up
void jtag_tcktock();
//! Reset the target device
void jtag_reset_target();
//! TAP RESET
void jtag_reset_tap();
//! Get into the Shift-IR or Shift-DR
void jtag_shift_register();
//! Get into Capture-IR state
void jtag_capture_ir();
//! Get into Capture-DR state
void jtag_capture_dr();
//! Get to Run-Test-Idle without going through Test-Logic-Reset
void jtag_run_test_idle();
//! Detect instruction register width
uint16_t jtag_detect_ir_width();
//! Detects how many TAPs are in the JTAG chain
uint16_t jtag_detect_chain_length();
//! Gets device ID for specified chip in the chain
uint32_t jtag_get_device_id(int chip);

#endif /* __JTAG_H__ */
