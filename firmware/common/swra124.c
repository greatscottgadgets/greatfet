/*
 * This file is part of GreatFET
 */

#include "swra124.h"

#include <gpio.h>
#include <gpio_lpc.h>
#include <greatfet_core.h>
#include <libopencm3/lpc43xx/scu.h>
#include <pins.h>

static struct gpio_t swra124_reset = GPIO(0, 10); // J1 P40 - RST 5
static struct gpio_t swra124_clock = GPIO(0, 11); // J1 P39 - DC 7
static struct gpio_t swra124_data = GPIO(0, 15);  // J1 P37 - DD 1

void swra124_setup()
{
	scu_pinmux(SCU_PINMUX_GPIO0_10, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO0_11, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO0_15, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);

	gpio_write(&swra124_reset, 1);
	gpio_write(&swra124_clock, 0);
	gpio_write(&swra124_data, 0);

	gpio_output(&swra124_reset);
	gpio_output(&swra124_clock);
	gpio_input(&swra124_data);
    delay_us(1);
}

void swra124_debug_init()
{
	struct {
		struct gpio_t *gpio;
		uint8_t value;
	} steps[] = {
		{.gpio = &swra124_reset, .value = 0},
		{.gpio = &swra124_clock, .value = 1},
		{.gpio = &swra124_clock, .value = 0},
		{.gpio = &swra124_clock, .value = 1},
		{.gpio = &swra124_clock, .value = 0},
		{.gpio = &swra124_reset, .value = 1},
		{.gpio = NULL},
	};
	for (int i = 0; steps[i].gpio; i++) {
		gpio_write(steps[i].gpio, steps[i].value);
		delay_us(1);
	}
}

void swra124_debug_stop()
{
    gpio_write(&swra124_reset, 0);
    delay_us(1);
    gpio_write(&swra124_reset, 1);
}

void swra124_write_byte(const uint8_t v)
{
	gpio_output(&swra124_data);
	for (int i = 0; i < 8; i++) {
		gpio_write(&swra124_data, (v >> (7 - i)) & 0x01);
		delay_us(1);
		gpio_write(&swra124_clock, 1);
		delay_us(1);
		gpio_write(&swra124_clock, 0);
		delay_us(1);
	}
}

void swra124_write(const uint8_t *data, const size_t size)
{
	for (size_t i = 0; i < size; i++) {
		swra124_write_byte(data[i]);
	}
}

uint8_t swra124_read()
{
	uint8_t result = 0;
	gpio_input(&swra124_data);
	for (int i = 0; i < 8; i++) {
		gpio_write(&swra124_clock, 1);
		delay_us(1);
		result = (result << 1) | gpio_read(&swra124_data);
		delay_us(1);
		gpio_write(&swra124_clock, 0);
		delay_us(1);
	}
	return result;
}

void swra124_chip_erase()
{
	uint8_t command[] = {0x14};
	swra124_write(command, 1);
	swra124_read();
}

void swra124_write_config(const uint8_t config)
{
	uint8_t command[] = {0x1d, config};
	swra124_write(command, 2);
	swra124_read();
}

uint8_t swra124_read_status()
{
	uint8_t command[] = {0x34};
	swra124_write(command, 1);
	return swra124_read();
}

uint16_t swra124_get_chip_id()
{
	uint8_t command[] = {0x68};
	swra124_write(command, 1);
	uint8_t chip_id = swra124_read();
	uint8_t chip_ver = swra124_read();
	switch(chip_id) {
        case 0x01://CC1110
        case 0x11://CC1111
        case 0x81://CC2510
        case 0x91://CC2511
            //debugstr("2 bytes/flash word");
            flash_word_size=0x02;
            break;
        default:
            //debugstr("Warning: Guessing flash word size.");
            //flash_word_size=0;
            break;
        case 0x85://CC2430
        case 0x89://CC2431
            //debugstr("4 bytes/flash word");
            flash_word_size=0x04;
            break;
	}
	return (chip_id << 8) | chip_ver;
}

void swra124_halt()
{
	uint8_t command[] = {0x44};
	swra124_write(command, 1);
	swra124_read();
}

void swra124_resume()
{
	uint8_t command[] = {0x4c};
	swra124_write(command, 1);
	swra124_read();
}

uint8_t swra124_debug_instr(const uint8_t *instr, const size_t size)
{
	uint8_t command[] = {0x54 | (size & 0x03)};
	swra124_write(command, 1);
	swra124_write(instr, size);
	return swra124_read();
}

void swra124_step_instr()
{
	uint8_t command[] = {0x5c};
	swra124_write(command, 1);
	swra124_read();
}

uint16_t swra124_get_pc()
{
	uint8_t command[] = {0x28};
	swra124_write(command, 1);
	return (swra124_read() << 8) | swra124_read();
}

void swra124_set_pc(const uint16_t v)
{
    uint8_t command[] = {0x02, ((v >> 8) & 0xff), v & 0xff};
    swra124_debug_instr(command, 3);
}

uint8_t swra124_peek_code_byte(const uint32_t adr)
{
    uint8_t bank=adr>>15,
            lb=adr&0xFF,
            hb=(adr>>8)&0x7F,
            toret=0;
    //adr&=0x7FFF;

    //MOV MEMCTR, (bank*16)+1
    //cc_debug(3, 0x75, 0xC7, (bank<<4) + 1);
    uint8_t command1[] = {0x75, 0xC7, (bank<<4) + 1};
    swra124_debug_instr(command1, 3);
    //MOV DPTR, address
    //cc_debug(3, 0x90, hb, lb);
    uint8_t command2[] = {0x90, hb, lb};
    swra124_debug_instr(command2, 3);
    //for each byte
    //CLR A
    //cc_debug(2, 0xE4, 0, 0);
    uint8_t command3[] = {0xE4, 0};
    swra124_debug_instr(command3, 2);
    //MOVC A, @A+DPTR;
    //toret=cc_debug(3, 0x93, 0, 0);
    uint8_t command4[] = {0x93, 0, 0};
    toret = swra124_debug_instr(command4, 3);
    return toret;
}

uint8_t swra124_peek_data_byte(const uint16_t adr)
{
    uint8_t hb=(adr&0xFF00)>>8, lb=adr&0xFF;

    //MOV DPTR, adr
    uint8_t command1[] = {0x90, hb, lb};
    swra124_debug_instr(command1, 3);
    //MOVX A, @DPTR
    //Must be 2, perhaps for clocking?
    uint8_t command2[] = {0xE0, 0, 0};
    return swra124_debug_instr(command2, 3);
}

void swra124_poke_data_byte(const uint16_t adr, const uint8_t val)
{

    uint8_t hb=(adr & 0xFF00)>>8, lb=adr&0xFF;

    //MOV DPTR, adr
    uint8_t command1[] = {0x90, hb, lb};
    swra124_debug_instr(command1, 3);
    //MOV A, val
    uint8_t command2[] = {0x74, val};
    swra124_debug_instr(command2, 2);
    //MOVX @DPTR, A
    uint8_t command3[] = {0xF0};
    swra124_debug_instr(command3, 1);
}

void swra124_write_xdata(const uint16_t adr, const uint8_t *data, const uint16_t len) {
    uint16_t i;
    for (i=0; i < len; i++) {
        swra124_poke_data_byte(adr+i, data[i]);
    }
}

void swra124_write_flash_buffer(uint8_t *data, uint16_t len){
    swra124_write_xdata(0xf000, data, len);
}

//32 bit words on CC2430
//16 bit words on CC1110
//#define FLASH_WORD_SIZE 0x2
uint8_t flash_word_size = 0x02; //0x02;
/* Flash Write Timing
   MHZ | FWT (0xAB)
   12  | 0x10
   13  | 0x11
   16  | 0x15
   24  | 0x20
   26  | 0x23  (IM ME)
   32  | 0x2A  (Modula.si)
*/
//#define FWT 0x23

const uint8_t flash_routine[] = {
        //0:
        //MOV FADDRH, #imm;
        0x75, 0xAD,
        0x00,//#imm=((address >> 8) / FLASH_WORD_SIZE) & 0x7E,

        //0x75, 0xAB, 0x23, //Set FWT per clock
        0x75, 0xAC, 0x00,                                          //                 MOV FADDRL, #00;
        /* Erase page. */
        0x75, 0xAE, 0x01,                                          //                 MOV FLC, #01H; // ERASE
        //                 ; Wait for flash erase to complete
        0xE5, 0xAE,                                                // eraseWaitLoop:  MOV A, FLC;
        0x20, 0xE7, 0xFB,                                          //                 JB ACC_BUSY, eraseWaitLoop;

        /* End erase page. */
        //                 ; Initialize the data pointer
        0x90, 0xF0, 0x00,                                          //                 MOV DPTR, #0F000H;
        //                 ; Outer loops
        0x7F, HIBYTE_WORDS_PER_FLASH_PAGE,                         //                 MOV R7, #imm;
        0x7E, LOBYTE_WORDS_PER_FLASH_PAGE,                         //                 MOV R6, #imm;
        0x75, 0xAE, 0x02,                                          //                 MOV FLC, #02H; // WRITE
        //                     ; Inner loops
        //24:
        0x7D, 0xde /*FLASH_WORD_SIZE*/,                                     // writeLoop:          MOV R5, #imm;
        0xE0,                                                      // writeWordLoop:          MOVX A, @DPTR;
        0xA3,                                                      //                         INC DPTR;
        0xF5, 0xAF,                                                //                         MOV FWDATA, A;
        0xDD, 0xFA,                                                //                     DJNZ R5, writeWordLoop;
        //                     ; Wait for completion
        0xE5, 0xAE,                                                // writeWaitLoop:      MOV A, FLC;
        0x20, 0xE6, 0xFB,                                          //                     JB ACC_SWBSY, writeWaitLoop;
        0xDE, 0xF1,                                                //                 DJNZ R6, writeLoop;
        0xDF, 0xEF,                                                //                 DJNZ R7, writeLoop;
        //                 ; Done, fake a breakpoint
        0xA5                                                       //                 DB 0xA5;
};


//! Copies flash buffer to flash.
void swra124_write_flash_page(const uint32_t adr)
{
    //Assumes that page has already been written to XDATA 0xF000
    //debugstr("Flashing 2kb at 0xF000 to given adr.");

    if(adr&(MINFLASHPAGE_SIZE-1)){
        //debugstr("Flash page address is not on a page boundary.  Aborting.");
        return;
    }

    if(flash_word_size!=2 && flash_word_size!=4){
        //debugstr("Flash word size is wrong, aborting write to");
        //debughex(adr);
        while(1);
    }

    //Routine comes next
    //WRITE_XDATA_MEMORY(IN: 0xF000 + FLASH_PAGE_SIZE, sizeof(routine), routine);
    swra124_write_xdata(0xF000+MAXFLASHPAGE_SIZE,
                   (uint8_t*) flash_routine, sizeof(flash_routine));
    //Patch routine's third byte with
    //((address >> 8) / FLASH_WORD_SIZE) & 0x7E
    swra124_poke_data_byte(0xF000+MAXFLASHPAGE_SIZE+2,
            ((adr>>8)/flash_word_size)&0x7E);
    //Patch routine to define FLASH_WORD_SIZE
    if(flash_routine[25]==0xde) {
        //Ugly patching code
        swra124_poke_data_byte(0xF000 + MAXFLASHPAGE_SIZE + 25,
                               flash_word_size);
    }
    //debugstr("Wrote flash routine.");

    //MOV MEMCTR, (bank * 16) + 1;
    uint8_t command[] = {0x75, 0xc7, 0x51};
    swra124_debug_instr(command,3);
    //cc_debug_instr(3);
    //debugstr("Loaded bank info.");

    swra124_set_pc(0xf000+MAXFLASHPAGE_SIZE);//execute code fragment
    swra124_resume();

    //debugstr("Executing.");


    while(!(swra124_read_status()&SWRA124_STATUS_CPUHALTED)){
        led_toggle(LED2);//blink LED while flashing
    }


    //debugstr("Done flashing.");

    led_off(LED2);
}


