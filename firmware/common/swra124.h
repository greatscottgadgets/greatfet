#ifndef __SWRA124_H__
#define __SWRA124_H__

#include <stddef.h>
#include <stdint.h>

#define SWRA124_MAX_INSTR_SIZE 4

void swra124_setup();
void swra124_debug_init();
uint8_t swra124_read_status();
uint16_t swra124_get_chip_id();
void swra124_halt();
void swra124_resume();
uint8_t swra124_debug_instr(const uint8_t *instr, const size_t size);
void swra124_step_instr();
uint16_t swra124_get_pc();

#endif
