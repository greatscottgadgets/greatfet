/*
 * This file is part of GreatFET
 */

#ifndef __SWRA124_H__
#define __SWRA124_H__

#include <stddef.h>
#include <stdint.h>

#define SWRA124_MAX_INSTR_SIZE 3

void swra124_setup(void);
void swra124_debug_init(void);
void swra124_chip_erase(void);
uint8_t swra124_read_status(void);
uint16_t swra124_get_chip_id(void);
void swra124_halt(void);
void swra124_resume(void);
uint8_t swra124_debug_instr(const uint8_t *instr, const size_t size);
void swra124_step_instr(void);
uint16_t swra124_get_pc(void);

#endif
