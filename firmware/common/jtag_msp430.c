/*
 * This file is part of GreatFET
 * direct port from GoodFET
 * Glory / failures go to Travis
 */
#include <stdbool.h>
#include <greatfet_core.h>
#include "jtag_msp430.h"
#include "jtag.h"

uint8_t jtag430mode=MSP430X2MODE;
uint8_t drwidth=16;

//! Shift an address width of data
uint32_t jtag430_shift_addr( uint32_t addr )
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
	return jtag_trans_n(addr, drwidth, MSB);
}

//! Set a register.
void jtag430_setr(uint8_t reg, uint16_t val)
{
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift_16(0x3401);// release low byte
  jtag_ir_shift_8(IR_DATA_16BIT);
  
  //0x4030 is "MOV #foo, r0"
  //Right-most field is register, so 0x4035 loads r5
  jtag_dr_shift_16(0x4030+reg);
  CLRTCLK;
  SETTCLK;
  jtag_dr_shift_16(val);// Value for the register
  CLRTCLK;
  jtag_ir_shift_8(IR_ADDR_CAPTURE);
  SETTCLK;
  CLRTCLK ;// Now reg is set to new value.
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift_16(0x2401);// low byte controlled by JTAG
}

//! Set the program counter.
void jtag430_setpc(uint16_t adr)
{
  jtag430_setr(0,adr);
}

//! Halt the CPU
void jtag430_haltcpu()
{
  //jtag430_setinstrfetch();
  
  jtag_ir_shift_8(IR_DATA_16BIT);
  jtag_dr_shift_16(0x3FFF);//JMP $+0
  
  CLRTCLK;
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift_16(0x2409);//set JTAG_HALT bit
  SETTCLK;
}

//! Release the CPU
void jtag430_releasecpu()
{
  CLRTCLK;
  // debugstr("Releasing target MSP430.");
  
  /*
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift_16(0x2C01); //Apply reset.
  jtag_dr_shift_16(0x2401); //Release reset.
  */
  jtag_ir_shift_8(IR_CNTRL_SIG_RELEASE);
  SETTCLK;
}

//! Read data from address
uint16_t jtag430_readmem(uint16_t adr)
{
  uint16_t toret;
  jtag430_haltcpu();
  
  CLRTCLK;
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  
  if(adr>0xFF)
    jtag_dr_shift_16(0x2409);//word read
  else
    jtag_dr_shift_16(0x2419);//byte read
  jtag_ir_shift_8(IR_ADDR_16BIT);
  jtag430_shift_addr(adr);//address
  jtag_ir_shift_8(IR_DATA_TO_ADDR);
  SETTCLK;

  CLRTCLK;
  toret=jtag_dr_shift_16(0x0000);//16 bit return
  
  return toret;
}

//! Write data to address.
void jtag430_writemem(uint16_t adr, uint16_t data)
{
  CLRTCLK;
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  if(adr>0xFF)
    jtag_dr_shift_16(0x2408);//word write
  else
    jtag_dr_shift_16(0x2418);//byte write
  jtag_ir_shift_8(IR_ADDR_16BIT);
  jtag430_shift_addr(adr);
  jtag_ir_shift_8(IR_DATA_TO_ADDR);
  jtag_dr_shift_16(data);
  SETTCLK;
}

void jtag430_tclk_flashpulses(uint16_t count)
{
  int i;
  for(i=0; i < count; i++) {
    SETTCLK;
    delay(80);
    CLRTCLK;
    delay(10);
  }
}

//! Write data to flash memory.  Must be preconfigured.
void jtag430_writeflashword(uint16_t adr, uint16_t data)
{
  CLRTCLK;
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift_16(0x2408);//word write
  jtag_ir_shift_8(IR_ADDR_16BIT);
  jtag430_shift_addr(adr);
  jtag_ir_shift_8(IR_DATA_TO_ADDR);
  jtag_dr_shift_16(data);
  SETTCLK;
  
  //Return to read mode.
  CLRTCLK;
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift_16(0x2409);
  
  //Pulse TCLK
  jtag430_tclk_flashpulses(35); //35 standard
}

//! Configure flash, then write a word.
void jtag430_writeflash(uint16_t adr, uint16_t data)
{
  jtag430_haltcpu();
  
  //FCTL1=0xA540, enabling flash write
  jtag430_writemem(0x0128, 0xA540);
  //FCTL2=0xA540, selecting MCLK as source, DIV=1
  jtag430_writemem(0x012A, 0xA540);
  //FCTL3=0xA500, should be 0xA540 for Info Seg A on 2xx chips.
  jtag430_writemem(0x012C, 0xA500); //all but info flash.
  //if(jtag430_readmem(0x012C));
  
  //Write the word itself.
  jtag430_writeflashword(adr,data);
  
  //FCTL1=0xA500, disabling flash write
  jtag430_writemem(0x0128, 0xA500);
  
  //jtag430_releasecpu();
}

//! Write a buffer to flash a word at a time
void jtag430_writeflash_bulk(uint16_t adr, uint16_t len, uint16_t *data)
{
  int i;
  for(i = 0; i < len; i++) {
    //// debugstr("Poking flash memory.");
    jtag430_writeflash(adr+(i*2), data[i]);
  }
}

//! Power-On Reset
void jtag430_por()
{
  // Perform Reset
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift_16(0x2C01); // apply
  jtag_dr_shift_16(0x2401); // remove
  CLRTCLK;
  SETTCLK;
  CLRTCLK;
  SETTCLK;
  CLRTCLK;
  jtagid = jtag_ir_shift_8(IR_ADDR_CAPTURE); // get JTAG identifier
  SETTCLK;
  
  jtag430_writemem(0x0120, 0x5A80);   // Diabled Watchdog
}



#define ERASE_GLOB 0xA50E
#define ERASE_ALLMAIN 0xA50C
#define ERASE_MASS 0xA506
#define ERASE_MAIN 0xA504
#define ERASE_SGMT 0xA502

//! Configure flash, then write a word.
void jtag430_eraseflash(uint16_t mode, uint16_t adr, uint16_t count, bool info)
{
  jtag430_haltcpu();
  
  //FCTL1= erase mode
  jtag430_writemem(0x0128, mode);
  //FCTL2=0xA540, selecting MCLK as source, DIV=1
  jtag430_writemem(0x012A, 0xA540);
  //FCTL3=0xA500, should be 0xA540 for Info Seg A on 2xx chips.
  if(info)
    jtag430_writemem(0x012C, 0xA540);
  else
    jtag430_writemem(0x012C, 0xA500);
  
  //Write the erase word.
  jtag430_writemem(adr, 0x55AA);
  //Return to read mode.
  CLRTCLK;
  jtag_ir_shift_8(IR_CNTRL_SIG_16BIT);
  jtag_dr_shift_16(0x2409);
  
  //Send the pulses.
  jtag430_tclk_flashpulses(count);
  
  //FCTL1=0xA500, disabling flash write
  jtag430_writemem(0x0128, 0xA500);
  
  //jtag430_releasecpu();
}

void jtag430_erase_entire_flash()
{
  jtag430_eraseflash(ERASE_MASS, 0xFFFE, 0x3000, 0);
}

void jtag430_erase_info()
{
  jtag430_eraseflash(ERASE_SGMT, 0x1000, 0x3000, 1);
}

//! Reset the TAP state machine.
void jtag430_resettap()
{
  int i;
  // Settle output
  SETTDI; //430X2
  SETTMS;
  //SETTDI; //classic
  jtag_tcktock();

  // Navigate to reset state.
  // Should be at least six.
  for(i=0;i<4;i++){
    jtag_tcktock();
  }

  // test-logic-reset
  CLRTMS;
  jtag_tcktock();
  SETTMS;
  // idle

    
  /* sacred, by spec.
     Sometimes this isn't necessary.  */
  // fuse check
  delay_us(1);
  CLRTMS;
  delay_us(1);
  SETTMS;
  delay_us(1);
  CLRTMS;
  delay_us(1);
  SETTMS;
  /**/
  
}

//! Get the JTAG ID
uint8_t jtag430x2_jtagid()
{
  jtagid = jtag_ir_shift_8(IR_BYPASS);
  // if(jtagid!=0x89 && jtagid!=0x91){
  //   // debugstr("Unknown JTAG ID");
  //   // debughex(jtagid);
  // }
  return jtagid;
}

void jtag430_entry_sequence()
{
   //Entry sequence from Page 67 of SLAU265A for 4-wire MSP430 JTAG
  CLRRST;
  delay_us(10);
  CLRTST;
  delay_us(10);
  SETTST;
  delay_us(1000);
  SETRST;
  delay_us(10000);
  jtag430_resettap();
}

//! Start JTAG, take pins
uint8_t jtag430x2_start()
{
  jtag_setup();
  delay_us(1000);
  SETTST;
  delay_us(6000);
  jtag430_entry_sequence();
  delay_us(10000);
  jtag430_entry_sequence();
  
  //Perform a reset and disable watchdog.
  return jtag430x2_jtagid();
}


//! Stop JTAG.
void jtag430_stop()
{
  // debugstr("Exiting JTAG.");
  jtag_setup();
  
  //Known-good starting position.
  //Might be unnecessary.
  CLRTST;
  SETRST;
  delay(0xFFFF);
  
  //Entry sequence from Page 67 of SLAU265A for 4-wire MSP430 JTAG
  CLRRST;
  delay(0xFFFF);
  SETRST;
  
}

//! Set CPU to Instruction Fetch
void jtag430_setinstrfetch()
{
  jtag_ir_shift_8(IR_CNTRL_SIG_CAPTURE);

  // Wait until instruction fetch state.
  while(1){
    if (jtag_dr_shift_16(0x0000) & 0x0080)
      return;
    CLRTCLK;
    SETTCLK;
  }
}


//JTAG430 commands
#define JTAG430_HALTCPU 0xA0
#define JTAG430_RELEASECPU 0xA1
#define JTAG430_SETINSTRFETCH 0xC1
#define JTAG430_SETPC 0xC2
#define JTAG430_SETREG 0xD2
#define JTAG430_GETREG 0xD3

#define JTAG430_WRITEMEM 0xE0
#define JTAG430_WRITEFLASH 0xE1
#define JTAG430_READMEM 0xE2
#define JTAG430_ERASEFLASH 0xE3
#define JTAG430_ERASECHECK 0xE4
#define JTAG430_VERIFYMEM 0xE5
#define JTAG430_BLOWFUSE 0xE6
#define JTAG430_ISFUSEBLOWN 0xE7
#define JTAG430_ERASEINFO 0xE8
#define JTAG430_COREIP_ID 0xF0
#define JTAG430_DEVICE_ID 0xF1


uint8_t jtag430_start_reset_halt()
{
  jtag430x2_start();
  jtag430mode=MSP430MODE;
  drwidth=20;
  jtag430_por();
  jtag430_writemem(0x120,0x5a80);//disable watchdog
  jtag430_haltcpu();
  jtag430_resettap();
  return jtagid;

}

void jtag430_check_init()
{
	/* FIXME from GoodFET
	 * Sometimes JTAG doesn't init correctly.
	 * This restarts the connection if the masked-rom
	 * chip ID cannot be read.  Should print warning
	 */
	int i;
	if (jtagid!=0) {
		while((i=jtag430_readmem(0xff0))==0xFFFF) {
			jtag430x2_start();
		}
	}
}
