/*
 * This file is part of GreatFET
 */

#ifndef LPC43XX_SPI_H
#define LPC43XX_SPI_H

/**@{*/

#include <libopencm3/cm3/common.h>
#include <libopencm3/lpc43xx/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Convenience macros -------------------------------------------------- */

/* SPI port base addresses (for convenience) */
#define SPI                             (SPI_PORT_BASE)

/* --- SPI registers ----------------------------------------------------- */

/* Control Register */
#define SPI_CR                          MMIO32(SPI + 0x000)

/* Status Register */
#define SPI_SR                          MMIO32(SPI + 0x004)

/* Data Register */
#define SPI_DR                          MMIO32(SPI + 0x008)

/* Clock Counter Register */
#define SPI_CCR                         MMIO32(SPI + 0x00C)

/* Test Control Register */
#define SPI_TCR                         MMIO32(SPI + 0x010)

/* Test Status Register */
#define SPI_TSR                         MMIO32(SPI + 0x014)

/* Interrupt Flag */
#define SPI_INT                         MMIO32(SPI + 0x01C)

/* --- SPI_CR values -------------------------------------------- */

/* BITENABLE: Bit length enable */
#define SPI_CR_BITENABLE_SHIFT (2)
#define SPI_CR_BITENABLE (1 << SPI_CR_BITENABLE_SHIFT)

/* CPHA: Clock phase control */
#define SPI_CR_CPHA_SHIFT (3)
#define SPI_CR_CPHA (1 << SPI_CR_CPHA_SHIFT)

/* CPOL: Clock polarity control */
#define SPI_CR_CPOL_SHIFT (4)
#define SPI_CR_CPOL (1 << SPI_CR_CPOL_SHIFT)

/* MSTR: Master mode select */
#define SPI_CR_MSTR_SHIFT (5)
#define SPI_CR_MSTR (1 << SPI_CR_MSTR_SHIFT)

/* LSBF: LSB first */
#define SPI_CR_LSBF_SHIFT (6)
#define SPI_CR_LSBF (1 << SPI_CR_LSBF_SHIFT)

/* SPIE: Serial peripheral interrupt enable */
#define SPI_CR_SPIE_SHIFT (7)
#define SPI_CR_SPIE (1 << SPI_CR_SPIE_SHIFT)

/* BITS: Bits per transfer */
#define SPI_CR_BITS_SHIFT (8)
#define SPI_CR_BITS_MASK (0xf << SPI_CR_BITS_SHIFT)
#define SPI_CR_BITS(x) ((x) << SPI_CR_BITS_SHIFT)

/* SPIF: Interrupt */
#define SPI_CR_SPIF_SHIFT (0)
#define SPI_CR_SPIF (1 << SPI_CR_SPIF_SHIFT)

/* --- SPI_SR values -------------------------------------------- */

/* ABRT: Slave abort */
#define SPI_SR_ABRT_SHIFT (3)
#define SPI_SR_ABRT (1 << SPI_SR_ABRT_SHIFT)

/* MODF: Mode fault */
#define SPI_SR_MODF_SHIFT (4)
#define SPI_SR_MODF (1 << SPI_SR_MODF_SHIFT)

/* ROVR: Read overrun */
#define SPI_SR_ROVR_SHIFT (5)
#define SPI_SR_ROVR (1 << SPI_SR_ROVR_SHIFT)

/* WCOL: Write collision */
#define SPI_SR_WCOL_SHIFT (6)
#define SPI_SR_WCOL (1 << SPI_SR_WCOL_SHIFT)

/* SPIF: Transfer complete */
#define SPI_SR_SPIF_SHIFT (7)
#define SPI_SR_SPIF (1 << SPI_SR_SPIF_SHIFT)

/* --- SPI_DR values -------------------------------------------- */

/* DATA: Bi-directional data port */
#define SPI_DR_DATA_SHIFT (0)
#define SPI_DR_DATA_MASK (0xffff << SPI_DR_DATA_SHIFT)
#define SPI_DR_DATA(x) ((x) << SPI_DR_DATA_SHIFT)

/* --- SPI_CCR values ------------------------------------------- */

/* COUNTER: Clock counter setting */
#define SPI_CCR_COUNTER_SHIFT (0)
#define SPI_CCR_COUNTER_MASK (0xff << SPI_CCR_COUNTER_SHIFT)
#define SPI_CCR_COUNTER(x) ((x) << SPI_CCR_COUNTER_SHIFT)

/* --- SPI_TCR values ------------------------------------------- */

/* TEST: Test mode */
#define SPI_TCR_TEST_SHIFT (1)
#define SPI_TCR_TEST_MASK (0x7f << SPI_TCR_TEST_SHIFT)
#define SPI_TCR_TEST(x) ((x) << SPI_TCR_TEST_SHIFT)

/* --- SPI_TSR values ------------------------------------------- */

/* ABRT: Slave abort */
#define SPI_TSR_ABRT_SHIFT (3)
#define SPI_TSR_ABRT (1 << SPI_TSR_ABRT_SHIFT)

/* MODF: Mode fault */
#define SPI_TSR_MODF_SHIFT (4)
#define SPI_TSR_MODF (1 << SPI_TSR_MODF_SHIFT)

/* ROVR: Read overrun */
#define SPI_TSR_ROVR_SHIFT (5)
#define SPI_TSR_ROVR (1 << SPI_TSR_ROVR_SHIFT)

/* WCOL: Write collision */
#define SPI_TSR_WCOL_SHIFT (6)
#define SPI_TSR_WCOL (1 << SPI_TSR_WCOL_SHIFT)

/* SPIF: Transfer complete */
#define SPI_TSR_SPIF_SHIFT (7)
#define SPI_TSR_SPIF (1 << SPI_TSR_SPIF_SHIFT)

/* --- SPI_INT values ------------------------------------------- */

/* SPIF: SPI interrupt flag */
#define SPI_INT_SPIF_SHIFT (0)
#define SPI_INT_SPIF (1 << SPI_INT_SPIF_SHIFT)

BEGIN_DECLS

/*****/

END_DECLS

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
