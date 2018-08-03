/*
 * This file is part of GreatFET
 */

#ifndef LPC43XX_GIMA_H
#define LPC43XX_GIMA_H

/**@{*/

#include <libopencm3/cm3/common.h>
#include <libopencm3/lpc43xx/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- GIMA registers ----------------------------------------------------- */

/* Timer 0 CAP0_0 capture input multiplexer (GIMA output 0) */
#define GIMA_CAP0_0_IN                  MMIO32(GIMA_BASE + 0x000)

/* Timer 0 CAP0_1 capture input multiplexer (GIMA output 1) */
#define GIMA_CAP0_1_IN                  MMIO32(GIMA_BASE + 0x004)

/* Timer 0 CAP0_2 capture input multiplexer (GIMA output 2) */
#define GIMA_CAP0_2_IN                  MMIO32(GIMA_BASE + 0x008)

/* Timer 0 CAP0_3 capture input multiplexer (GIMA output 3) */
#define GIMA_CAP0_3_IN                  MMIO32(GIMA_BASE + 0x00C)

/* Timer 1 CAP1_0 capture input multiplexer (GIMA output 4) */
#define GIMA_CAP1_0_IN                  MMIO32(GIMA_BASE + 0x010)

/* Timer 1 CAP1_1 capture input multiplexer (GIMA output 5) */
#define GIMA_CAP1_1_IN                  MMIO32(GIMA_BASE + 0x014)

/* Timer 1 CAP1_2 capture input multiplexer (GIMA output 6) */
#define GIMA_CAP1_2_IN                  MMIO32(GIMA_BASE + 0x018)

/* Timer 1 CAP1_3 capture input multiplexer (GIMA output 7) */
#define GIMA_CAP1_3_IN                  MMIO32(GIMA_BASE + 0x01C)

/* Timer 2 CAP2_0 capture input multiplexer (GIMA output 8) */
#define GIMA_CAP2_0_IN                  MMIO32(GIMA_BASE + 0x020)

/* Timer 2 CAP2_1 capture input multiplexer (GIMA output 9) */
#define GIMA_CAP2_1_IN                  MMIO32(GIMA_BASE + 0x024)

/* Timer 2 CAP2_2 capture input multiplexer (GIMA output 10) */
#define GIMA_CAP2_2_IN                  MMIO32(GIMA_BASE + 0x028)

/* Timer 2 CAP2_3 capture input multiplexer (GIMA output 11) */
#define GIMA_CAP2_3_IN                  MMIO32(GIMA_BASE + 0x02C)

/* Timer 3 CAP3_0 capture input multiplexer (GIMA output 12) */
#define GIMA_CAP3_0_IN                  MMIO32(GIMA_BASE + 0x030)

/* Timer 3 CAP3_1 capture input multiplexer (GIMA output 13) */
#define GIMA_CAP3_1_IN                  MMIO32(GIMA_BASE + 0x034)

/* Timer 3 CAP3_2 capture input multiplexer (GIMA output 14) */
#define GIMA_CAP3_2_IN                  MMIO32(GIMA_BASE + 0x038)

/* Timer 3 CAP3_3 capture input multiplexer (GIMA output 15) */
#define GIMA_CAP3_3_IN                  MMIO32(GIMA_BASE + 0x03C)

/* SCT CTIN_0 capture input multiplexer (GIMA output 16) */
#define GIMA_CTIN_0_IN                  MMIO32(GIMA_BASE + 0x040)

/* SCT CTIN_1 capture input multiplexer (GIMA output 17) */
#define GIMA_CTIN_1_IN                  MMIO32(GIMA_BASE + 0x044)

/* SCT CTIN_2 capture input multiplexer (GIMA output 18) */
#define GIMA_CTIN_2_IN                  MMIO32(GIMA_BASE + 0x048)

/* SCT CTIN_3 capture input multiplexer (GIMA output 19) */
#define GIMA_CTIN_3_IN                  MMIO32(GIMA_BASE + 0x04C)

/* SCT CTIN_4 capture input multiplexer (GIMA output 20) */
#define GIMA_CTIN_4_IN                  MMIO32(GIMA_BASE + 0x050)

/* SCT CTIN_5 capture input multiplexer (GIMA output 21) */
#define GIMA_CTIN_5_IN                  MMIO32(GIMA_BASE + 0x054)

/* SCT CTIN_6 capture input multiplexer (GIMA output 22) */
#define GIMA_CTIN_6_IN                  MMIO32(GIMA_BASE + 0x058)

/* SCT CTIN_7 capture input multiplexer (GIMA output 23) */
#define GIMA_CTIN_7_IN                  MMIO32(GIMA_BASE + 0x05C)

/* ADCHS trigger input multiplexer (GIMA output 24) */
#define GIMA_ADCHS_TRIGGER_IN           MMIO32(GIMA_BASE + 0x060)

/* Event router input 13 multiplexer (GIMA output 25) */
#define GIMA_EVENTROUTER_13_IN          MMIO32(GIMA_BASE + 0x064)

/* Event router input 14 multiplexer (GIMA output 26) */
#define GIMA_EVENTROUTER_14_IN          MMIO32(GIMA_BASE + 0x068)

/* Event router input 16 multiplexer (GIMA output 27) */
#define GIMA_EVENTROUTER_16_IN          MMIO32(GIMA_BASE + 0x06C)

/* ADC start0 input multiplexer (GIMA output 28) */
#define GIMA_ADCSTART0_IN               MMIO32(GIMA_BASE + 0x070)

/* ADC start1 input multiplexer (GIMA output 29) */
#define GIMA_ADCSTART1_IN               MMIO32(GIMA_BASE + 0x074)

/* --- GIMA CAP0_0_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP0_0_IN_INV_SHIFT (0)
#define GIMA_CAP0_0_IN_INV (1 << GIMA_CAP0_0_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP0_0_IN_EDGE_SHIFT (1)
#define GIMA_CAP0_0_IN_EDGE (1 << GIMA_CAP0_0_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP0_0_IN_SYNCH_SHIFT (2)
#define GIMA_CAP0_0_IN_SYNCH (1 << GIMA_CAP0_0_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP0_0_IN_PULSE_SHIFT (3)
#define GIMA_CAP0_0_IN_PULSE (1 << GIMA_CAP0_0_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP0_0_IN_SELECT_SHIFT (4)
#define GIMA_CAP0_0_IN_SELECT_MASK (0xf << GIMA_CAP0_0_IN_SELECT_SHIFT)
#define GIMA_CAP0_0_IN_SELECT(x) ((x) << GIMA_CAP0_0_IN_SELECT_SHIFT)

/* --- GIMA CAP0_1_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP0_1_IN_INV_SHIFT (0)
#define GIMA_CAP0_1_IN_INV (1 << GIMA_CAP0_1_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP0_1_IN_EDGE_SHIFT (1)
#define GIMA_CAP0_1_IN_EDGE (1 << GIMA_CAP0_1_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP0_1_IN_SYNCH_SHIFT (2)
#define GIMA_CAP0_1_IN_SYNCH (1 << GIMA_CAP0_1_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP0_1_IN_PULSE_SHIFT (3)
#define GIMA_CAP0_1_IN_PULSE (1 << GIMA_CAP0_1_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP0_1_IN_SELECT_SHIFT (4)
#define GIMA_CAP0_1_IN_SELECT_MASK (0xf << GIMA_CAP0_1_IN_SELECT_SHIFT)
#define GIMA_CAP0_1_IN_SELECT(x) ((x) << GIMA_CAP0_1_IN_SELECT_SHIFT)

/* --- GIMA CAP0_2_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP0_2_IN_INV_SHIFT (0)
#define GIMA_CAP0_2_IN_INV (1 << GIMA_CAP0_2_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP0_2_IN_EDGE_SHIFT (1)
#define GIMA_CAP0_2_IN_EDGE (1 << GIMA_CAP0_2_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP0_2_IN_SYNCH_SHIFT (2)
#define GIMA_CAP0_2_IN_SYNCH (1 << GIMA_CAP0_2_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP0_2_IN_PULSE_SHIFT (3)
#define GIMA_CAP0_2_IN_PULSE (1 << GIMA_CAP0_2_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP0_2_IN_SELECT_SHIFT (4)
#define GIMA_CAP0_2_IN_SELECT_MASK (0xf << GIMA_CAP0_2_IN_SELECT_SHIFT)
#define GIMA_CAP0_2_IN_SELECT(x) ((x) << GIMA_CAP0_2_IN_SELECT_SHIFT)

/* --- GIMA CAP0_3_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP0_3_IN_INV_SHIFT (0)
#define GIMA_CAP0_3_IN_INV (1 << GIMA_CAP0_3_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP0_3_IN_EDGE_SHIFT (1)
#define GIMA_CAP0_3_IN_EDGE (1 << GIMA_CAP0_3_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP0_3_IN_SYNCH_SHIFT (2)
#define GIMA_CAP0_3_IN_SYNCH (1 << GIMA_CAP0_3_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP0_3_IN_PULSE_SHIFT (3)
#define GIMA_CAP0_3_IN_PULSE (1 << GIMA_CAP0_3_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP0_3_IN_SELECT_SHIFT (4)
#define GIMA_CAP0_3_IN_SELECT_MASK (0xf << GIMA_CAP0_3_IN_SELECT_SHIFT)
#define GIMA_CAP0_3_IN_SELECT(x) ((x) << GIMA_CAP0_3_IN_SELECT_SHIFT)

/* --- GIMA CAP1_0_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP1_0_IN_INV_SHIFT (0)
#define GIMA_CAP1_0_IN_INV (1 << GIMA_CAP1_0_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP1_0_IN_EDGE_SHIFT (1)
#define GIMA_CAP1_0_IN_EDGE (1 << GIMA_CAP1_0_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP1_0_IN_SYNCH_SHIFT (2)
#define GIMA_CAP1_0_IN_SYNCH (1 << GIMA_CAP1_0_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP1_0_IN_PULSE_SHIFT (3)
#define GIMA_CAP1_0_IN_PULSE (1 << GIMA_CAP1_0_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP1_0_IN_SELECT_SHIFT (4)
#define GIMA_CAP1_0_IN_SELECT_MASK (0xf << GIMA_CAP1_0_IN_SELECT_SHIFT)
#define GIMA_CAP1_0_IN_SELECT(x) ((x) << GIMA_CAP1_0_IN_SELECT_SHIFT)

/* --- GIMA CAP1_1_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP1_1_IN_INV_SHIFT (0)
#define GIMA_CAP1_1_IN_INV (1 << GIMA_CAP1_1_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP1_1_IN_EDGE_SHIFT (1)
#define GIMA_CAP1_1_IN_EDGE (1 << GIMA_CAP1_1_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP1_1_IN_SYNCH_SHIFT (2)
#define GIMA_CAP1_1_IN_SYNCH (1 << GIMA_CAP1_1_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP1_1_IN_PULSE_SHIFT (3)
#define GIMA_CAP1_1_IN_PULSE (1 << GIMA_CAP1_1_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP1_1_IN_SELECT_SHIFT (4)
#define GIMA_CAP1_1_IN_SELECT_MASK (0xf << GIMA_CAP1_1_IN_SELECT_SHIFT)
#define GIMA_CAP1_1_IN_SELECT(x) ((x) << GIMA_CAP1_1_IN_SELECT_SHIFT)

/* --- GIMA CAP1_2_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP1_2_IN_INV_SHIFT (0)
#define GIMA_CAP1_2_IN_INV (1 << GIMA_CAP1_2_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP1_2_IN_EDGE_SHIFT (1)
#define GIMA_CAP1_2_IN_EDGE (1 << GIMA_CAP1_2_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP1_2_IN_SYNCH_SHIFT (2)
#define GIMA_CAP1_2_IN_SYNCH (1 << GIMA_CAP1_2_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP1_2_IN_PULSE_SHIFT (3)
#define GIMA_CAP1_2_IN_PULSE (1 << GIMA_CAP1_2_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP1_2_IN_SELECT_SHIFT (4)
#define GIMA_CAP1_2_IN_SELECT_MASK (0xf << GIMA_CAP1_2_IN_SELECT_SHIFT)
#define GIMA_CAP1_2_IN_SELECT(x) ((x) << GIMA_CAP1_2_IN_SELECT_SHIFT)

/* --- GIMA CAP1_3_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP1_3_IN_INV_SHIFT (0)
#define GIMA_CAP1_3_IN_INV (1 << GIMA_CAP1_3_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP1_3_IN_EDGE_SHIFT (1)
#define GIMA_CAP1_3_IN_EDGE (1 << GIMA_CAP1_3_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP1_3_IN_SYNCH_SHIFT (2)
#define GIMA_CAP1_3_IN_SYNCH (1 << GIMA_CAP1_3_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP1_3_IN_PULSE_SHIFT (3)
#define GIMA_CAP1_3_IN_PULSE (1 << GIMA_CAP1_3_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP1_3_IN_SELECT_SHIFT (4)
#define GIMA_CAP1_3_IN_SELECT_MASK (0xf << GIMA_CAP1_3_IN_SELECT_SHIFT)
#define GIMA_CAP1_3_IN_SELECT(x) ((x) << GIMA_CAP1_3_IN_SELECT_SHIFT)

/* --- GIMA CAP2_0_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP2_0_IN_INV_SHIFT (0)
#define GIMA_CAP2_0_IN_INV (1 << GIMA_CAP2_0_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP2_0_IN_EDGE_SHIFT (1)
#define GIMA_CAP2_0_IN_EDGE (1 << GIMA_CAP2_0_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP2_0_IN_SYNCH_SHIFT (2)
#define GIMA_CAP2_0_IN_SYNCH (1 << GIMA_CAP2_0_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP2_0_IN_PULSE_SHIFT (3)
#define GIMA_CAP2_0_IN_PULSE (1 << GIMA_CAP2_0_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CAP2_0_IN_SELECT_SHIFT (4)
#define GIMA_CAP2_0_IN_SELECT_MASK (0xf << GIMA_CAP2_0_IN_SELECT_SHIFT)
#define GIMA_CAP2_0_IN_SELECT(x) ((x) << GIMA_CAP2_0_IN_SELECT_SHIFT)

/* --- GIMA CAP2_1_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP2_1_IN_INV_SHIFT (0)
#define GIMA_CAP2_1_IN_INV (1 << GIMA_CAP2_1_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP2_1_IN_EDGE_SHIFT (1)
#define GIMA_CAP2_1_IN_EDGE (1 << GIMA_CAP2_1_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP2_1_IN_SYNCH_SHIFT (2)
#define GIMA_CAP2_1_IN_SYNCH (1 << GIMA_CAP2_1_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP2_1_IN_PULSE_SHIFT (3)
#define GIMA_CAP2_1_IN_PULSE (1 << GIMA_CAP2_1_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CAP2_1_IN_SELECT_SHIFT (4)
#define GIMA_CAP2_1_IN_SELECT_MASK (0xf << GIMA_CAP2_1_IN_SELECT_SHIFT)
#define GIMA_CAP2_1_IN_SELECT(x) ((x) << GIMA_CAP2_1_IN_SELECT_SHIFT)

/* --- GIMA CAP2_2_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP2_2_IN_INV_SHIFT (0)
#define GIMA_CAP2_2_IN_INV (1 << GIMA_CAP2_2_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP2_2_IN_EDGE_SHIFT (1)
#define GIMA_CAP2_2_IN_EDGE (1 << GIMA_CAP2_2_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP2_2_IN_SYNCH_SHIFT (2)
#define GIMA_CAP2_2_IN_SYNCH (1 << GIMA_CAP2_2_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP2_2_IN_PULSE_SHIFT (3)
#define GIMA_CAP2_2_IN_PULSE (1 << GIMA_CAP2_2_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CAP2_2_IN_SELECT_SHIFT (4)
#define GIMA_CAP2_2_IN_SELECT_MASK (0xf << GIMA_CAP2_2_IN_SELECT_SHIFT)
#define GIMA_CAP2_2_IN_SELECT(x) ((x) << GIMA_CAP2_2_IN_SELECT_SHIFT)

/* --- GIMA CAP2_3_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP2_3_IN_INV_SHIFT (0)
#define GIMA_CAP2_3_IN_INV (1 << GIMA_CAP2_3_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP2_3_IN_EDGE_SHIFT (1)
#define GIMA_CAP2_3_IN_EDGE (1 << GIMA_CAP2_3_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP2_3_IN_SYNCH_SHIFT (2)
#define GIMA_CAP2_3_IN_SYNCH (1 << GIMA_CAP2_3_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP2_3_IN_PULSE_SHIFT (3)
#define GIMA_CAP2_3_IN_PULSE (1 << GIMA_CAP2_3_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP2_3_IN_SELECT_SHIFT (4)
#define GIMA_CAP2_3_IN_SELECT_MASK (0xf << GIMA_CAP2_3_IN_SELECT_SHIFT)
#define GIMA_CAP2_3_IN_SELECT(x) ((x) << GIMA_CAP2_3_IN_SELECT_SHIFT)

/* --- GIMA CAP3_0_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP3_0_IN_INV_SHIFT (0)
#define GIMA_CAP3_0_IN_INV (1 << GIMA_CAP3_0_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP3_0_IN_EDGE_SHIFT (1)
#define GIMA_CAP3_0_IN_EDGE (1 << GIMA_CAP3_0_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP3_0_IN_SYNCH_SHIFT (2)
#define GIMA_CAP3_0_IN_SYNCH (1 << GIMA_CAP3_0_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP3_0_IN_PULSE_SHIFT (3)
#define GIMA_CAP3_0_IN_PULSE (1 << GIMA_CAP3_0_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CAP3_0_IN_SELECT_SHIFT (4)
#define GIMA_CAP3_0_IN_SELECT_MASK (0xf << GIMA_CAP3_0_IN_SELECT_SHIFT)
#define GIMA_CAP3_0_IN_SELECT(x) ((x) << GIMA_CAP3_0_IN_SELECT_SHIFT)

/* --- GIMA CAP3_1_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP3_1_IN_INV_SHIFT (0)
#define GIMA_CAP3_1_IN_INV (1 << GIMA_CAP3_1_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP3_1_IN_EDGE_SHIFT (1)
#define GIMA_CAP3_1_IN_EDGE (1 << GIMA_CAP3_1_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP3_1_IN_SYNCH_SHIFT (2)
#define GIMA_CAP3_1_IN_SYNCH (1 << GIMA_CAP3_1_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP3_1_IN_PULSE_SHIFT (3)
#define GIMA_CAP3_1_IN_PULSE (1 << GIMA_CAP3_1_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CAP3_1_IN_SELECT_SHIFT (4)
#define GIMA_CAP3_1_IN_SELECT_MASK (0xf << GIMA_CAP3_1_IN_SELECT_SHIFT)
#define GIMA_CAP3_1_IN_SELECT(x) ((x) << GIMA_CAP3_1_IN_SELECT_SHIFT)

/* --- GIMA CAP3_2_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP3_2_IN_INV_SHIFT (0)
#define GIMA_CAP3_2_IN_INV (1 << GIMA_CAP3_2_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP3_2_IN_EDGE_SHIFT (1)
#define GIMA_CAP3_2_IN_EDGE (1 << GIMA_CAP3_2_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP3_2_IN_SYNCH_SHIFT (2)
#define GIMA_CAP3_2_IN_SYNCH (1 << GIMA_CAP3_2_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP3_2_IN_PULSE_SHIFT (3)
#define GIMA_CAP3_2_IN_PULSE (1 << GIMA_CAP3_2_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CAP3_2_IN_SELECT_SHIFT (4)
#define GIMA_CAP3_2_IN_SELECT_MASK (0xf << GIMA_CAP3_2_IN_SELECT_SHIFT)
#define GIMA_CAP3_2_IN_SELECT(x) ((x) << GIMA_CAP3_2_IN_SELECT_SHIFT)

/* --- GIMA CAP3_3_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CAP3_3_IN_INV_SHIFT (0)
#define GIMA_CAP3_3_IN_INV (1 << GIMA_CAP3_3_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CAP3_3_IN_EDGE_SHIFT (1)
#define GIMA_CAP3_3_IN_EDGE (1 << GIMA_CAP3_3_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CAP3_3_IN_SYNCH_SHIFT (2)
#define GIMA_CAP3_3_IN_SYNCH (1 << GIMA_CAP3_3_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CAP3_3_IN_PULSE_SHIFT (3)
#define GIMA_CAP3_3_IN_PULSE (1 << GIMA_CAP3_3_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CAP3_3_IN_SELECT_SHIFT (4)
#define GIMA_CAP3_3_IN_SELECT_MASK (0xf << GIMA_CAP3_3_IN_SELECT_SHIFT)
#define GIMA_CAP3_3_IN_SELECT(x) ((x) << GIMA_CAP3_3_IN_SELECT_SHIFT)

/* --- GIMA CTIN_0_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CTIN_0_IN_INV_SHIFT (0)
#define GIMA_CTIN_0_IN_INV (1 << GIMA_CTIN_0_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CTIN_0_IN_EDGE_SHIFT (1)
#define GIMA_CTIN_0_IN_EDGE (1 << GIMA_CTIN_0_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CTIN_0_IN_SYNCH_SHIFT (2)
#define GIMA_CTIN_0_IN_SYNCH (1 << GIMA_CTIN_0_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CTIN_0_IN_PULSE_SHIFT (3)
#define GIMA_CTIN_0_IN_PULSE (1 << GIMA_CTIN_0_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CTIN_0_IN_SELECT_SHIFT (4)
#define GIMA_CTIN_0_IN_SELECT_MASK (0xf << GIMA_CTIN_0_IN_SELECT_SHIFT)
#define GIMA_CTIN_0_IN_SELECT(x) ((x) << GIMA_CTIN_0_IN_SELECT_SHIFT)

/* --- GIMA CTIN_1_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CTIN_1_IN_INV_SHIFT (0)
#define GIMA_CTIN_1_IN_INV (1 << GIMA_CTIN_1_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CTIN_1_IN_EDGE_SHIFT (1)
#define GIMA_CTIN_1_IN_EDGE (1 << GIMA_CTIN_1_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CTIN_1_IN_SYNCH_SHIFT (2)
#define GIMA_CTIN_1_IN_SYNCH (1 << GIMA_CTIN_1_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CTIN_1_IN_PULSE_SHIFT (3)
#define GIMA_CTIN_1_IN_PULSE (1 << GIMA_CTIN_1_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CTIN_1_IN_SELECT_SHIFT (4)
#define GIMA_CTIN_1_IN_SELECT_MASK (0xf << GIMA_CTIN_1_IN_SELECT_SHIFT)
#define GIMA_CTIN_1_IN_SELECT(x) ((x) << GIMA_CTIN_1_IN_SELECT_SHIFT)

/* --- GIMA CTIN_2_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CTIN_2_IN_INV_SHIFT (0)
#define GIMA_CTIN_2_IN_INV (1 << GIMA_CTIN_2_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CTIN_2_IN_EDGE_SHIFT (1)
#define GIMA_CTIN_2_IN_EDGE (1 << GIMA_CTIN_2_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CTIN_2_IN_SYNCH_SHIFT (2)
#define GIMA_CTIN_2_IN_SYNCH (1 << GIMA_CTIN_2_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CTIN_2_IN_PULSE_SHIFT (3)
#define GIMA_CTIN_2_IN_PULSE (1 << GIMA_CTIN_2_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CTIN_2_IN_SELECT_SHIFT (4)
#define GIMA_CTIN_2_IN_SELECT_MASK (0xf << GIMA_CTIN_2_IN_SELECT_SHIFT)
#define GIMA_CTIN_2_IN_SELECT(x) ((x) << GIMA_CTIN_2_IN_SELECT_SHIFT)

/* --- GIMA CTIN_3_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CTIN_3_IN_INV_SHIFT (0)
#define GIMA_CTIN_3_IN_INV (1 << GIMA_CTIN_3_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CTIN_3_IN_EDGE_SHIFT (1)
#define GIMA_CTIN_3_IN_EDGE (1 << GIMA_CTIN_3_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CTIN_3_IN_SYNCH_SHIFT (2)
#define GIMA_CTIN_3_IN_SYNCH (1 << GIMA_CTIN_3_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CTIN_3_IN_PULSE_SHIFT (3)
#define GIMA_CTIN_3_IN_PULSE (1 << GIMA_CTIN_3_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CTIN_3_IN_SELECT_SHIFT (4)
#define GIMA_CTIN_3_IN_SELECT_MASK (0xf << GIMA_CTIN_3_IN_SELECT_SHIFT)
#define GIMA_CTIN_3_IN_SELECT(x) ((x) << GIMA_CTIN_3_IN_SELECT_SHIFT)

/* --- GIMA CTIN_4_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CTIN_4_IN_INV_SHIFT (0)
#define GIMA_CTIN_4_IN_INV (1 << GIMA_CTIN_4_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CTIN_4_IN_EDGE_SHIFT (1)
#define GIMA_CTIN_4_IN_EDGE (1 << GIMA_CTIN_4_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CTIN_4_IN_SYNCH_SHIFT (2)
#define GIMA_CTIN_4_IN_SYNCH (1 << GIMA_CTIN_4_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CTIN_4_IN_PULSE_SHIFT (3)
#define GIMA_CTIN_4_IN_PULSE (1 << GIMA_CTIN_4_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CTIN_4_IN_SELECT_SHIFT (4)
#define GIMA_CTIN_4_IN_SELECT_MASK (0xf << GIMA_CTIN_4_IN_SELECT_SHIFT)
#define GIMA_CTIN_4_IN_SELECT(x) ((x) << GIMA_CTIN_4_IN_SELECT_SHIFT)

/* --- GIMA CTIN_5_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CTIN_5_IN_INV_SHIFT (0)
#define GIMA_CTIN_5_IN_INV (1 << GIMA_CTIN_5_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CTIN_5_IN_EDGE_SHIFT (1)
#define GIMA_CTIN_5_IN_EDGE (1 << GIMA_CTIN_5_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CTIN_5_IN_SYNCH_SHIFT (2)
#define GIMA_CTIN_5_IN_SYNCH (1 << GIMA_CTIN_5_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CTIN_5_IN_PULSE_SHIFT (3)
#define GIMA_CTIN_5_IN_PULSE (1 << GIMA_CTIN_5_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_CTIN_5_IN_SELECT_SHIFT (4)
#define GIMA_CTIN_5_IN_SELECT_MASK (0xf << GIMA_CTIN_5_IN_SELECT_SHIFT)
#define GIMA_CTIN_5_IN_SELECT(x) ((x) << GIMA_CTIN_5_IN_SELECT_SHIFT)

/* --- GIMA CTIN_6_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CTIN_6_IN_INV_SHIFT (0)
#define GIMA_CTIN_6_IN_INV (1 << GIMA_CTIN_6_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CTIN_6_IN_EDGE_SHIFT (1)
#define GIMA_CTIN_6_IN_EDGE (1 << GIMA_CTIN_6_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CTIN_6_IN_SYNCH_SHIFT (2)
#define GIMA_CTIN_6_IN_SYNCH (1 << GIMA_CTIN_6_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CTIN_6_IN_PULSE_SHIFT (3)
#define GIMA_CTIN_6_IN_PULSE (1 << GIMA_CTIN_6_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CTIN_6_IN_SELECT_SHIFT (4)
#define GIMA_CTIN_6_IN_SELECT_MASK (0xf << GIMA_CTIN_6_IN_SELECT_SHIFT)
#define GIMA_CTIN_6_IN_SELECT(x) ((x) << GIMA_CTIN_6_IN_SELECT_SHIFT)

/* --- GIMA CTIN_7_IN values -------------------------------------------- */

/* INV: Invert Input */
#define GIMA_CTIN_7_IN_INV_SHIFT (0)
#define GIMA_CTIN_7_IN_INV (1 << GIMA_CTIN_7_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_CTIN_7_IN_EDGE_SHIFT (1)
#define GIMA_CTIN_7_IN_EDGE (1 << GIMA_CTIN_7_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_CTIN_7_IN_SYNCH_SHIFT (2)
#define GIMA_CTIN_7_IN_SYNCH (1 << GIMA_CTIN_7_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_CTIN_7_IN_PULSE_SHIFT (3)
#define GIMA_CTIN_7_IN_PULSE (1 << GIMA_CTIN_7_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x4 to 0xF are reserved */
#define GIMA_CTIN_7_IN_SELECT_SHIFT (4)
#define GIMA_CTIN_7_IN_SELECT_MASK (0xf << GIMA_CTIN_7_IN_SELECT_SHIFT)
#define GIMA_CTIN_7_IN_SELECT(x) ((x) << GIMA_CTIN_7_IN_SELECT_SHIFT)

/* --- GIMA ADCHS_TRIGGER_IN values -------------------------------------- */

/* INV: Invert Input */
#define GIMA_ADCHS_TRIGGER_IN_INV_SHIFT (0)
#define GIMA_ADCHS_TRIGGER_IN_INV (1 << GIMA_ADCHS_TRIGGER_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_ADCHS_TRIGGER_IN_EDGE_SHIFT (1)
#define GIMA_ADCHS_TRIGGER_IN_EDGE (1 << GIMA_ADCHS_TRIGGER_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_ADCHS_TRIGGER_IN_SYNCH_SHIFT (2)
#define GIMA_ADCHS_TRIGGER_IN_SYNCH (1 << GIMA_ADCHS_TRIGGER_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_ADCHS_TRIGGER_IN_PULSE_SHIFT (3)
#define GIMA_ADCHS_TRIGGER_IN_PULSE (1 << GIMA_ADCHS_TRIGGER_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0xA to 0xF are reserved */
#define GIMA_ADCHS_TRIGGER_IN_SELECT_SHIFT (4)
#define GIMA_ADCHS_TRIGGER_IN_SELECT_MASK (0xf << GIMA_ADCHS_TRIGGER_IN_SELECT_SHIFT)
#define GIMA_ADCHS_TRIGGER_IN_SELECT(x) ((x) << GIMA_ADCHS_TRIGGER_IN_SELECT_SHIFT)

/* --- GIMA EVENTROUTER_13_IN values -------------------------------------- */

/* INV: Invert Input */
#define GIMA_EVENTROUTER_13_IN_INV_SHIFT (0)
#define GIMA_EVENTROUTER_13_IN_INV (1 << GIMA_EVENTROUTER_13_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_EVENTROUTER_13_IN_EDGE_SHIFT (1)
#define GIMA_EVENTROUTER_13_IN_EDGE (1 << GIMA_EVENTROUTER_13_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_EVENTROUTER_13_IN_SYNCH_SHIFT (2)
#define GIMA_EVENTROUTER_13_IN_SYNCH (1 << GIMA_EVENTROUTER_13_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_EVENTROUTER_13_IN_PULSE_SHIFT (3)
#define GIMA_EVENTROUTER_13_IN_PULSE (1 << GIMA_EVENTROUTER_13_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_EVENTROUTER_13_IN_SELECT_SHIFT (4)
#define GIMA_EVENTROUTER_13_IN_SELECT_MASK (0xf << GIMA_EVENTROUTER_13_IN_SELECT_SHIFT)
#define GIMA_EVENTROUTER_13_IN_SELECT(x) ((x) << GIMA_EVENTROUTER_13_IN_SELECT_SHIFT)

/* --- GIMA EVENTROUTER_14_IN values -------------------------------------- */

/* INV: Invert Input */
#define GIMA_EVENTROUTER_14_IN_INV_SHIFT (0)
#define GIMA_EVENTROUTER_14_IN_INV (1 << GIMA_EVENTROUTER_14_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_EVENTROUTER_14_IN_EDGE_SHIFT (1)
#define GIMA_EVENTROUTER_14_IN_EDGE (1 << GIMA_EVENTROUTER_14_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_EVENTROUTER_14_IN_SYNCH_SHIFT (2)
#define GIMA_EVENTROUTER_14_IN_SYNCH (1 << GIMA_EVENTROUTER_14_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_EVENTROUTER_14_IN_PULSE_SHIFT (3)
#define GIMA_EVENTROUTER_14_IN_PULSE (1 << GIMA_EVENTROUTER_14_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x3 to 0xF are reserved */
#define GIMA_EVENTROUTER_14_IN_SELECT_SHIFT (4)
#define GIMA_EVENTROUTER_14_IN_SELECT_MASK (0xf << GIMA_EVENTROUTER_14_IN_SELECT_SHIFT)
#define GIMA_EVENTROUTER_14_IN_SELECT(x) ((x) << GIMA_EVENTROUTER_14_IN_SELECT_SHIFT)

/* --- GIMA EVENTROUTER_16_IN values -------------------------------------- */

/* INV: Invert Input */
#define GIMA_EVENTROUTER_16_IN_INV_SHIFT (0)
#define GIMA_EVENTROUTER_16_IN_INV (1 << GIMA_EVENTROUTER_16_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_EVENTROUTER_16_IN_EDGE_SHIFT (1)
#define GIMA_EVENTROUTER_16_IN_EDGE (1 << GIMA_EVENTROUTER_16_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_EVENTROUTER_16_IN_SYNCH_SHIFT (2)
#define GIMA_EVENTROUTER_16_IN_SYNCH (1 << GIMA_EVENTROUTER_16_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_EVENTROUTER_16_IN_PULSE_SHIFT (3)
#define GIMA_EVENTROUTER_16_IN_PULSE (1 << GIMA_EVENTROUTER_16_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x2 to 0xF are reserved */
#define GIMA_EVENTROUTER_16_IN_SELECT_SHIFT (4)
#define GIMA_EVENTROUTER_16_IN_SELECT_MASK (0xf << GIMA_EVENTROUTER_16_IN_SELECT_SHIFT)
#define GIMA_EVENTROUTER_16_IN_SELECT(x) ((x) << GIMA_EVENTROUTER_16_IN_SELECT_SHIFT)

/* --- GIMA ADCSTART0_IN values -------------------------------------- */

/* INV: Invert Input */
#define GIMA_ADCSTART0_IN_INV_SHIFT (0)
#define GIMA_ADCSTART0_IN_INV (1 << GIMA_ADCSTART0_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_ADCSTART0_IN_EDGE_SHIFT (1)
#define GIMA_ADCSTART0_IN_EDGE (1 << GIMA_ADCSTART0_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_ADCSTART0_IN_SYNCH_SHIFT (2)
#define GIMA_ADCSTART0_IN_SYNCH (1 << GIMA_ADCSTART0_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_ADCSTART0_IN_PULSE_SHIFT (3)
#define GIMA_ADCSTART0_IN_PULSE (1 << GIMA_ADCSTART0_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x2 to 0xF are reserved */
#define GIMA_ADCSTART0_IN_SELECT_SHIFT (4)
#define GIMA_ADCSTART0_IN_SELECT_MASK (0xf << GIMA_ADCSTART0_IN_SELECT_SHIFT)
#define GIMA_ADCSTART0_IN_SELECT(x) ((x) << GIMA_ADCSTART0_IN_SELECT_SHIFT)

/* --- GIMA ADCSTART1_IN values -------------------------------------- */

/* INV: Invert Input */
#define GIMA_ADCSTART1_IN_INV_SHIFT (0)
#define GIMA_ADCSTART1_IN_INV (1 << GIMA_ADCSTART1_IN_INV_SHIFT)

/* EDGE: Enable rising edge detection */
#define GIMA_ADCSTART1_IN_EDGE_SHIFT (1)
#define GIMA_ADCSTART1_IN_EDGE (1 << GIMA_ADCSTART1_IN_INV_SHIFT)

/* SYNCH: Enable synchronizatoin */
#define GIMA_ADCSTART1_IN_SYNCH_SHIFT (2)
#define GIMA_ADCSTART1_IN_SYNCH (1 << GIMA_ADCSTART1_IN_SYNCH_SHIFT)

/* PULSE: Enable single pulse generation */
#define GIMA_ADCSTART1_IN_PULSE_SHIFT (3)
#define GIMA_ADCSTART1_IN_PULSE (1 << GIMA_ADCSTART1_IN_PULSE_SHIFT)

/* SELECT: Select input. Values 0x2 to 0xF are reserved */
#define GIMA_ADCSTART1_IN_SELECT_SHIFT (4)
#define GIMA_ADCSTART1_IN_SELECT_MASK (0xf << GIMA_ADCSTART1_IN_SELECT_SHIFT)
#define GIMA_ADCSTART1_IN_SELECT(x) ((x) << GIMA_ADCSTART1_IN_SELECT_SHIFT)

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
