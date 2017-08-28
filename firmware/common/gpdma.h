/*
 * This file is part of GreatFET
 */

#ifndef __GPDMA_H__
#define __GPDMA_H__

#include <stddef.h>
#include <stdint.h>

#include <libopencm3/lpc43xx/gpdma.h>

void gpdma_controller_enable();

void gpdma_channel_enable(const uint_fast8_t channel);
void gpdma_channel_disable(const uint_fast8_t channel);

void gpdma_channel_interrupt_tc_clear(const uint_fast8_t channel);
void gpdma_channel_interrupt_error_clear(const uint_fast8_t channel);
int gpdma_channel_interrupt_is_error(const uint_fast8_t channel);

void gpdma_lli_enable_interrupt(gpdma_lli_t* const lli);

void gpdma_lli_create_loop(gpdma_lli_t* const lli, const size_t lli_count);
void gpdma_lli_create_oneshot(gpdma_lli_t* const lli, const size_t lli_count);

#endif/*__GPDMA_H__*/
