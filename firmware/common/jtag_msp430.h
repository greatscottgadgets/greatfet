/*
 * This file is part of GreatFET
 * direct port from GoodFET
 * Glory / failures go to Travis
 */

#ifndef __JTAG_MSP430_H__
#define __JTAG_MSP430_H__

#define MSP430MODE 0
#define MSP430XMODE 1
#define MSP430X2MODE 2

// JTAG430 Commands

//! Same thing, but also for '430X2.
uint8_t jtag430x2_start();
//! Stop JTAG
void jtag430_stop();
//! Reset the TAP state machine, check the fuse.
void jtag430_resettap();

//! Defined in jtag430asm.S
void jtag430_tclk_flashpulses(uint16_t count);

//High-level Macros follow
//! Write data to address.
void jtag430_writemem(uint16_t adr, uint16_t data);
//! Read data from address
uint16_t jtag430_readmem(uint16_t adr);
//! Halt the CPU
void jtag430_haltcpu();
//! Release the CPU
void jtag430_releasecpu();
//! Set CPU to Instruction Fetch
void jtag430_setinstrfetch();
//! Set register
void jtag430_setr(uint8_t reg, uint16_t val);
//! Set the program counter.
void jtag430_setpc(uint16_t adr);
//! Erase target flash from address
void jtag430_eraseflash(uint16_t mode, uint16_t adr, uint16_t count, bool info);
//! Mass flash erase helper function
void jtag430_erase_entire_flash();
//! Erase info flash helper function
void jtag430_erase_info();
//! Write data to address.
void jtag430_writeflash(uint16_t adr, uint16_t data);
//!
void jtag430_writeflash_bulk(uint16_t adr, uint16_t len, uint16_t *data);
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
uint16_t jtag430x2_syncpor();
//! Executes an MSP430X2 POR
uint16_t jtag430x2_por();
//! Power-On Reset
void jtag430_por();

uint8_t jtag430_start_reset_halt();

// Taken from GoodFET because
// "Sometimes JTAG doesn't init correctly"
void jtag430_check_init();

#endif /* __JTAG_MSP430_H__ */
