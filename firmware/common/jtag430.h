/*
 * This file is part of GreatFET
 * derived from GoodFET (thanks Travis)
 */

#ifndef __JTAG430_H__
#define __JTAG430_H__

extern unsigned int drwidth;

#define MSP430MODE 0
#define MSP430XMODE 1
#define MSP430X2MODE 2
extern unsigned int jtag430mode;

// JTAG430 Commands

//! Start JTAG, unique to the '430.  Deprecated.
void jtag430_start();
//! Same thing, but also for '430X2.
unsigned char jtag430x2_start();
//! Reset the TAP state machine, check the fuse.
void jtag430_resettap();

//! Defined in jtag430asm.S
void jtag430_tclk_flashpulses(uint8_t);

//High-level Macros follow
//! Write data to address.
void jtag430_writemem(unsigned int adr, unsigned int data);
//! Read data from address
unsigned int jtag430_readmem(unsigned int adr);
//! Halt the CPU
void jtag430_haltcpu();
//! Release the CPU
void jtag430_releasecpu();
//! Set CPU to Instruction Fetch
void jtag430_setinstrfetch();
//! Set the program counter.
void jtag430_setpc(unsigned int adr);
//! Write data to address.
void jtag430_writeflash(unsigned int adr, unsigned int data);
//! Shift an address width of data
uint32_t jtag430_shift_addr( uint32_t addr );


//16-bit MSP430 JTAG commands, bit-swapped
#define IR_CNTRL_SIG_16BIT         0xC8   // 0x13
#define IR_CNTRL_SIG_CAPTURE       0x28   // 0x14
#define IR_CNTRL_SIG_RELEASE       0xA8   // 0x15
// Instructions for the JTAG Fuse
#define IR_PREPARE_BLOW            0x44   // 0x22
#define IR_EX_BLOW                 0x24   // 0x24
// Instructions for the JTAG data register
#define IR_DATA_16BIT              0x82   // 0x41
#define IR_DATA_QUICK              0xC2   // 0x43
// Instructions for the JTAG PSA mode
#define IR_DATA_PSA                0x22   // 0x44
#define IR_SHIFT_OUT_PSA           0x62   // 0x46
// Instructions for the JTAG address register
#define IR_ADDR_16BIT              0xC1   // 0x83
#define IR_ADDR_CAPTURE            0x21   // 0x84
#define IR_DATA_TO_ADDR            0xA1   // 0x85
// Bypass instruction
#define IR_BYPASS                  0xFF   // 0xFF

//MSP430X2 unique
#define IR_COREIP_ID               0xE8   // 0x17 
#define IR_DEVICE_ID               0xE1   // 0x87

//MSP430 or MSP430X
#define MSP430JTAGID 0x89
//MSP430X2 only
#define MSP430X2JTAGID 0x91

//! Syncs a POR.
unsigned int jtag430x2_syncpor();
//! Executes an MSP430X2 POR
unsigned int jtag430x2_por();
//! Power-On Reset
void jtag430_por();


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

#endif /* __JTAG430_H__ */
