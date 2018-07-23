/*
 * This file is part of GreatFET
 */

#ifndef __DAC_H
#define __DAC_H

/**@{*/

#include <libopencm3/cm3/common.h>
#include <libopencm3/lpc43xx/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Convenience macros -------------------------------------------------- */

/* DAC port base address (for convenience) */
#define DAC                             DAC_BASE

/* --- DAC registers ------------------------------------------------------- */

#define DAC_CR							MMIO32(DAC_BASE + 0x000)
#define DAC_CTRL						MMIO32(DAC_BASE + 0x004)
#define DAC_CNTVAL						MMIO32(DAC_BASE + 0x008)

/* --- DAC_CR values ------------------------------------------- */

/* VALUE: ... */
#define DAC_CR_VALUE_SHIFT (6)
#define DAC_CR_VALUE_MASK (0x3ff << DAC_CR_VALUE_SHIFT)
#define DAC_CR_VALUE(x) ((x) << DAC_CR_VALUE_SHIFT)

/* BIAS: ... */
#define DAC_CR_BIAS_SHIFT (16)
#define DAC_CR_BIAS (1 << DAC_CR_BIAS_SHIFT)

/* --- DAC_CTRL values ----------------------------------------- */

/* INT_DMA_REQ: ... */
#define DAC_CTRL_INT_DMA_REQ_SHIFT (0)
#define DAC_CTRL_INT_DMA_REQ (1 << DAC_CTRL_INT_DMA_REQ_SHIFT)

/* DBLBUF_ENA: ... */
#define DAC_CTRL_DBLBUF_ENA_SHIFT (1)
#define DAC_CTRL_DBLBUF_ENA (1 << DAC_CTRL_DBLBUF_ENA_SHIFT)

/* CNT_ENA: ... */
#define DAC_CTRL_CNT_ENA_SHIFT (2)
#define DAC_CTRL_CNT_ENA (1 << DAC_CTRL_CNT_ENA_SHIFT)

/* DMA_ENA: ... */
#define DAC_CTRL_DMA_ENA_SHIFT (3)
#define DAC_CTRL_DMA_ENA (1 << DAC_CTRL_DMA_ENA_SHIFT)

/* --- DAC_CNTVAL values --------------------------------------- */

/* VALUE: ... */
#define DAC_CNTVAL_VALUE_SHIFT (0)
#define DAC_CNTVAL_VALUE_MASK (0xffff << DAC_CNTVAL_VALUE_SHIFT)
#define DAC_CNTVAL_VALUE(x) ((x) << DAC_CNTVAL_VALUE_SHIFT)

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
