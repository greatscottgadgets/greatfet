/*
 * This file is part of GreatFET
 * direct port from GoodFET
 * Glory / failures go to Travis
 */

#include <libopencm3/lpc43xx/scu.h>
#include <greatfet_core.h>
#include <pins.h>
#include "jtag.h"

//JTAG commands
#define JTAG_IR_SHIFT 0x80
#define JTAG_DR_SHIFT 0x81
#define JTAG_RESET_TAP 0x82
#define JTAG_RESET_TARGET 0x83
#define JTAG_DETECT_IR_WIDTH 0x84
#define JTAG_DETECT_CHAIN_LENGTH 0x85
#define JTAG_GET_DEVICE_ID 0x86
#define JTAG_DR_SHIFT_MORE 0x87 // used for shiftings > 32bits.  assumes JTAG_DR_SHIFT with NOEND first
//#define JTAG_DR_SHIFT20 0x91


//! Set up the pins for JTAG mode.
void jtag_setup(void)
{
	scu_pinmux(SCU_PINMUX_GPIO0_10, SCU_GPIO_FAST | SCU_GPIO_PUP | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO0_11, SCU_GPIO_FAST | SCU_GPIO_PUP | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO0_15, SCU_GPIO_FAST | SCU_GPIO_PUP | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_14, SCU_GPIO_FAST | SCU_GPIO_PUP | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_3, SCU_GPIO_FAST | SCU_GPIO_PUP | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_2, SCU_GPIO_FAST | SCU_GPIO_PUP | SCU_CONF_FUNCTION0);

	gpio_write(&tdi, 0);
	gpio_write(&tms, 1);
	gpio_write(&tck, 1);
	gpio_write(&rst, 1);
	gpio_write(&tst, 0);

	gpio_input(&tdo);
	gpio_output(&tst);
	gpio_output(&tdi);
	gpio_output(&tms);
	gpio_output(&tck);
	gpio_output(&rst);
	jtag_state = UNKNOWN;
}

//! Stop JTAG, release pins
void jtag_stop(void)
{
	gpio_write(&tst, 0);
}

uint8_t savedtclk;
#define SAVETCLK savedtclk=gpio_read(&tdi)
#define RESTORETCLK gpio_write(&tdi, savedtclk)

//! JTAG device ID.
unsigned char jtagid=0;

//! Remembers what the current JTAG state is
enum eTAPState jtag_state = UNKNOWN;

//! Returns true if we're in any of the data register states
int in_dr()
{
	return (int)(jtag_state & (SELECT_DR_SCAN | CAPTURE_DR |
							   SHIFT_DR | EXIT1_DR | PAUSE_DR |
							   EXIT2_DR | UPDATE_DR));
}

//! Returns true if we're in any of the instruction register states
int in_ir()
{
	return (int)(jtag_state & (SELECT_IR_SCAN | CAPTURE_IR |
							   SHIFT_IR | EXIT1_IR | PAUSE_IR |
							   EXIT2_IR | UPDATE_IR));
}

//! Returns true if we're in run-test-idle state
int in_run_test_idle()
{
	return (int)(jtag_state & RUN_TEST_IDLE);
}

//! Check the state
int in_state(enum eTAPState state)
{
	return (int)(jtag_state & state);
}


//! Reset the target device
void jtag_reset_target()
{
	SETRST;
	led_toggle(LED4);
	CLRRST;
	led_toggle(LED4);
}

//! Clock the JTAG clock line
void jtag_tcktock() 
{
	CLRTCK;
	delay_us(1);
	SETTCK;
	delay_us(1);
}

//! Goes through test-logic-reset and ends in run-test-idle
void jtag_reset_tap()
{
	CLRTDI;
	SETTMS;
	jtag_tcktock();
	jtag_tcktock();
	jtag_tcktock();
	jtag_tcktock();
	jtag_tcktock();  // now in Reset state
	CLRTMS;
	jtag_tcktock();  // now in Run-Test/Idle state
	jtag_state = RUN_TEST_IDLE;
}

//! Get into Shift-IR or Shift-DR state
void jtag_shift_register()
{
	// assumes we're in any state that can transition to Shift-IR or Shift-DR
	if (!in_state(CAPTURE_DR | CAPTURE_IR | SHIFT_DR | 
				  SHIFT_IR | EXIT2_DR | EXIT2_IR ))
	{
		// debugstr("Invalid JTAG state");
		return;
	}

	CLRTDI;
	CLRTMS;
	jtag_tcktock();

	if (in_dr())
		jtag_state = SHIFT_DR;
	else
		jtag_state = SHIFT_IR;
}

//! Get into Capture-IR state
void jtag_capture_ir()
{
	// assumes you're in Run-Test/Idle, Update-DR or Update-IR
	if (!in_state(RUN_TEST_IDLE | UPDATE_DR | UPDATE_IR))
	{
		// debugstr("Invalid JTAG state");
		return;
	}

	CLRTDI;
	SETTMS;
	jtag_tcktock(); // Select-DR-Scan
	jtag_tcktock(); // Select-IR-Scan
	CLRTMS;
	jtag_tcktock(); // Capture-IR

	jtag_state = CAPTURE_IR;
}

//! Get into Capture-DR state
void jtag_capture_dr()
{
	// assumes you're in Run-Test/Idle, Update-DR or Update-IR
	if (!in_state(RUN_TEST_IDLE | UPDATE_DR | UPDATE_IR))
	{
		// debugstr("Invalid JTAG state");
		return;
	}

	CLRTDI;
	SETTMS;
	jtag_tcktock(); // Select-DR-Scan
	CLRTMS;
	jtag_tcktock(); // Capture-IR

	jtag_state = CAPTURE_DR;
}

//! Gets back to run-test-idle without going through the test-logic-reset
void jtag_run_test_idle()
{
	CLRTDI;

	if (in_state(SELECT_DR_SCAN | SELECT_IR_SCAN))
	{
		CLRTMS;
		jtag_tcktock();
		jtag_state <<= 1; //CAPTURE_DR or CAPTURE_IR
	}

	if (in_state(CAPTURE_DR | CAPTURE_IR))
	{
		SETTMS;
		jtag_tcktock();
		jtag_state <<= 2; //EXIT1_DR or EXIT1_IR
	}

	if (in_state(SHIFT_DR | SHIFT_IR))
	{
		SETTMS;
		jtag_tcktock();
		jtag_state <<= 1; //EXIT1_DR or EXIT1_IR
	}

	if (in_state(EXIT1_DR | EXIT1_IR))
	{
		SETTMS;
		jtag_tcktock();
		jtag_state <<=3; //UPDATE_DR or UPDATE_IR
	}

	if (in_state(PAUSE_DR | PAUSE_IR))
	{
		SETTMS;
		jtag_tcktock();
		jtag_state <<= 1; // EXIT2_DR or EXIT2_IR
	}

	if (in_state(EXIT2_DR | EXIT2_IR))
	{
		SETTMS;
		jtag_tcktock();
		jtag_state <<= 1; // UPDATE_DR or UPDATE_IR
	}

	if (in_state(UPDATE_DR | UPDATE_IR | TEST_LOGIC_RESET))
	{
		CLRTMS;
		jtag_tcktock();
		jtag_state = RUN_TEST_IDLE;
	}
}

//	NOTE: important: THIS MODULE REVOLVES AROUND RETURNING TO RUNTEST/IDLE, OR 
//	THE FUNCTIONAL EQUIVALENT
//! Shift N bits over TDI/TDO.	May choose LSB or MSB, and select whether to 
//	terminate (TMS-high on last bit) and whether to return to RUNTEST/IDLE
//		flags should be 0 for most uses.  
//		for the extreme case, flags should be  (NOEND|NORETDLE|LSB)
//		other edge cases can involve a combination of those three flags
//
//		the max bit-size that can be be shifted is 32-bits.  
//		for longer shifts, use the NOEND flag (which infers NORETIDLE so the 
//		additional flag is unnecessary)
//
//		NORETIDLE is used for special cases where (as with arm) the debug 
//		subsystem does not want to return to the RUN-TEST/IDLE state between 
//		setting IR and DR
uint32_t jtag_trans_n(uint32_t word, 
		      uint8_t bitcount, 
		      enum eTransFlags flags) 
{
	uint8_t bit;
	uint32_t high = (1L << (bitcount - 1));
	uint32_t mask = high - 1;

	if (!in_state(SHIFT_IR | SHIFT_DR))
	{
		// debugstr("jtag_trans_n from invalid TAP state");
		return 0;
	}

	SAVETCLK;

	if (flags & LSB) 
	{
		for (bit = bitcount; bit > 0; bit--) 
		{
			/* write MOSI on trailing edge of previous clock */
			if (word & 1)
			{
				SETTDI;
			}
			else
			{
				CLRTDI;
			}
			word >>= 1;

			if ((bit == 1) && !(flags & NOEND))
				SETTMS; //TMS high on last bit to exit.

			jtag_tcktock();

			if ((bit == 1) && !(flags & NOEND))
				jtag_state <<= 1; // Exit1-DR or Exit1-IR

			/* read MISO on trailing edge */
			if (READTDO)
			{
				word += (high);
			}
		}
	} 
	else 
	{
		for (bit = bitcount; bit > 0; bit--) 
		{
			/* write MOSI on trailing edge of previous clock */
			if (word & high)
			{
				SETTDI;
			}
			else
			{
				CLRTDI;
			}
			word = (word & mask) << 1;

			if ((bit==1) && !(flags & NOEND))
				SETTMS; //TMS high on last bit to exit.

			jtag_tcktock();

			if ((bit == 1) && !(flags & NOEND))
				jtag_state <<= 1; // Exit1-DR or Exit1-IR

			/* read MISO on trailing edge */
			word |= (READTDO);
		}
	}
	
	//This is needed for 20-bit MSP430 chips.
	//Might break another 20-bit chip, if one exists.
	if(bitcount==20){
	  word = ((word << 16) | (word >> 4)) & 0x000FFFFF;
	}
	
	RESTORETCLK;

	if (!(flags & NOEND))
	{
		// exit state
		jtag_tcktock();

		jtag_state <<= 3; // Update-DR or Update-IR

		// update state
		if (!(flags & NORETIDLE))
		{
			CLRTMS;
			jtag_tcktock();

			jtag_state = RUN_TEST_IDLE;
		}
	}

	return word;
}

//! Detects the width of the IR register
uint16_t jtag_detect_ir_width()
{
	int i;
	uint16_t width = 0;

	if (!in_run_test_idle())
	{
		// debugstr("Not in run-test-idle state");
		return 0;
	}

	// get to shift-ir state
	jtag_capture_ir();
	jtag_shift_register();

	// first we shift in 1024 zeros
	CLRTDI;
	for(i = 0; i < 1024; i++)
	{
		jtag_tcktock();
	}

	// now we'll clock in ones until we see one
	SETTDI;
	for(i = 0; i < 1024; i++)
	{
		jtag_tcktock();
		if(READTDO)
			break;
		width++;
	}

	// now get back to run-test-idle
	jtag_run_test_idle();

	return width;
}

//! Detects how many TAPs are in the JTAG chain
uint16_t jtag_detect_chain_length()
{
	int i;
	int bit;
	uint16_t length = 0;

	if (!in_run_test_idle())
	{
		// debugstr("detect chain length must start from run-test-idle");
		return 0;
	}

	// The standard JTAG instruction for "bypass" mode is to set all 1's in the
	// instruction register.  When in "bypass" mode, the DR acts like a 1-bit
	// shift regiser.  So we can use that to detect how many TAPs are in the 
	// chain.
	
	// first get into shift IR mode
	jtag_capture_ir();
	jtag_shift_register();

	// then we flood the IR chain with all 1's to put all device's DRs
	// into bypass mode
	CLRTMS;
	SETTDI;
	for (i = 0; i < 1024; i++)
	{
		if (i == 1023)
			SETTMS; // exit on last bit
		jtag_tcktock();
	}
	jtag_state = EXIT1_IR;

	// go to Update-IR
	CLRTDI;
	jtag_tcktock();
	jtag_state = UPDATE_IR;

	// go to Shift-DR state
	jtag_capture_dr();
	jtag_shift_register();

	// flush the DR's with zeros
	CLRTMS;
	CLRTDI;
	for (i = 0; i < 1024; i++)
	{
		jtag_tcktock();
	}

	// send 1's into the DRs until we see one come out the other side
	SETTDI;
	for (i = 0; i < 1024; i++)
	{
		jtag_tcktock();
		bit = READTDO;
		if (bit)
			break;
		length++;
	}

	// now get back to run-test-idle
	jtag_run_test_idle();

	return length;
}

//! Gets device ID for specified chip in the chain
uint32_t jtag_get_device_id(int chip)
{
	int i;
	uint16_t chain_length;
	uint32_t id = 0;

	// reset everything
	jtag_reset_tap();

	// figure out how many devices are in the chain
	chain_length = jtag_detect_chain_length();

	if (chip >= chain_length)
	{
		// debugstr("invalid part index");
		return 0;
	}

	// reset everything again because going through test-logic-reset forces
	// all IR's to have their manufacturer specific instruction for IDCODE
	// and loads the DR's with the chip's ID code.
	jtag_reset_tap();

	// get into shift DR state
	jtag_capture_dr();
	jtag_shift_register();

	// read out the 32-bit ID codes for each device
	CLRTMS;
	CLRTDI;
	for (i = 0; i < (chip + 1); i++)
	{
		id = jtag_trans_n(0xFFFFFFFF, 32, LSB | NOEND | NORETIDLE);
	}

	jtag_run_test_idle();

	return id;
}

//! Shift 8 bits in/out of selected register
uint8_t jtag_trans_8(uint8_t in)
{
	uint32_t out = jtag_trans_n((uint32_t)in, 8, MSB);
	return (uint8_t)(0x000000FF & out);
}

//! Shift 16 bits in/out of selected register
uint16_t jtag_trans_16(uint16_t in)
{
	uint32_t out = jtag_trans_n((uint32_t)in, 16, MSB);
	return (uint16_t)(0x0000FFFF & out);
}

//! Shift 8 bits of the IR.
uint8_t jtag_ir_shift_8(uint8_t in)
{
  /* Huseby's code, which breaks MSP430 support.
     The code is broken because either the invalid jtag state error
     causes the client to give up, or because it adds an extra clock edge.
     
	if (!in_run_test_idle())
	{
		debugstr("Not in run-test-idle state");
		return 0;
	}

	// get intot the right state
	jtag_capture_ir();
	jtag_shift_register();
  */
  // idle
  SETTMS;
  delay_us(1);
  jtag_tcktock();
  // select DR
  jtag_tcktock();
  // select IR
  CLRTMS;
  jtag_tcktock();
  // capture IR
  jtag_tcktock();
  // shift IR
  jtag_state = SHIFT_IR;
  // shift IR bits
  return jtag_trans_8(in);
}

//! Shift 16 bits of the DR.
uint16_t jtag_dr_shift_16(uint16_t in)
{

	if (!in_run_test_idle())
	{
		// debugstr("Not in run-test-idle state");
		return 0;
	}

	// get intot the right state
	jtag_capture_dr();
	jtag_shift_register();

	// shift DR, then idle
	return jtag_trans_16(in);
}
