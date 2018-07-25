/*
 * This file is part of GreatFET
 */

#ifndef LPC43XX_ADC_H
#define LPC43XX_ADC_H

/**@{*/

#include <libopencm3/cm3/common.h>
#include <libopencm3/lpc43xx/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Convenience macros -------------------------------------------------- */

/* ADC port base addresses (for convenience) */
#define ADC0                            ADC0_BASE
#define ADC1                            ADC1_BASE


/* --- ADC registers ------------------------------------------------------- */

/* A/D Control Register */
#define ADC_CR(port)                    MMIO32(port + 0x000)
#define ADC0_CR                         ADC_CR(ADC0)
#define ADC1_CR                         ADC_CR(ADC1)

/* A/D Global Data Register */
#define ADC_GDR(port)                   MMIO32(port + 0x004)
#define ADC0_GDR                        ADC_GDR(ADC0)
#define ADC1_GDR                        ADC_GDR(ADC1)

/* A/D Interrupt Enable Register */
#define ADC_INTEN(port)                 MMIO32(port + 0x00C)
#define ADC0_INTEN                      ADC_INTEN(ADC0)
#define ADC1_INTEN                      ADC_INTEN(ADC1)

/* A/D Channel 0 Data Register */
#define ADC_DR0(port)                   MMIO32(port + 0x010)
#define ADC0_DR0                        ADC_DR0(ADC0)
#define ADC1_DR0                        ADC_DR0(ADC1)

/* A/D Channel 1 Data Register */
#define ADC_DR1(port)                   MMIO32(port + 0x014)
#define ADC0_DR1                        ADC_DR1(ADC0)
#define ADC1_DR1                        ADC_DR1(ADC1)

/* A/D Channel 2 Data Register */
#define ADC_DR2(port)                   MMIO32(port + 0x018)
#define ADC0_DR2                        ADC_DR2(ADC0)
#define ADC1_DR2                        ADC_DR2(ADC1)

/* A/D Channel 3 Data Register */
#define ADC_DR3(port)                   MMIO32(port + 0x01C)
#define ADC0_DR3                        ADC_DR3(ADC0)
#define ADC1_DR3                        ADC_DR3(ADC1)

/* A/D Channel 4 Data Register */
#define ADC_DR4(port)                   MMIO32(port + 0x020)
#define ADC0_DR4                        ADC_DR4(ADC0)
#define ADC1_DR4                        ADC_DR4(ADC1)

/* A/D Channel 5 Data Register */
#define ADC_DR5(port)                   MMIO32(port + 0x024)
#define ADC0_DR5                        ADC_DR5(ADC0)
#define ADC1_DR5                        ADC_DR5(ADC1)

/* A/D Channel 6 Data Register */
#define ADC_DR6(port)                   MMIO32(port + 0x028)
#define ADC0_DR6                        ADC_DR6(ADC0)
#define ADC1_DR6                        ADC_DR6(ADC1)

/* A/D Channel 7 Data Register */
#define ADC_DR7(port)                   MMIO32(port + 0x02C)
#define ADC0_DR7                        ADC_DR7(ADC0)
#define ADC1_DR7                        ADC_DR7(ADC1)

/* A/D Status Register */
#define ADC_STAT(port)                  MMIO32(port + 0x030)
#define ADC0_STAT                       ADC_STAT(ADC0)
#define ADC1_STAT                       ADC_STAT(ADC1)

/* --- ADCx_CR values ------------------------------------------------------ */

/* SEL: ... */
#define ADC_CR_SEL_SHIFT (0)
#define ADC_CR_SEL_MASK (0xff << ADC_CR_SEL_SHIFT)
#define ADC_CR_SEL(x) ((x) << ADC_CR_SEL_SHIFT)

/* CLKDIV: ... */
#define ADC_CR_CLKDIV_SHIFT (8)
#define ADC_CR_CLKDIV_MASK (0xff << ADC_CR_CLKDIV_SHIFT)
#define ADC_CR_CLKDIV(x) ((x) << ADC_CR_CLKDIV_SHIFT)

/* BURST: ... */
#define ADC_CR_BURST_SHIFT (16)
#define ADC_CR_BURST (1 << ADC_CR_BURST_SHIFT)

/* CLKS: ... */
#define ADC_CR_CLKS_SHIFT (17)
#define ADC_CR_CLKS_MASK (0x7 << ADC_CR_CLKS_SHIFT)
#define ADC_CR_CLKS(x) ((x) << ADC_CR_CLKS_SHIFT)

/* PDN: ... */
#define ADC_CR_PDN_SHIFT (21)
#define ADC_CR_PDN (1 << ADC_CR_PDN_SHIFT)

/* START: ... */
#define ADC_CR_START_SHIFT (24)
#define ADC_CR_START_MASK (0x7 << ADC_CR_START_SHIFT)
#define ADC_CR_START(x) ((x) << ADC_CR_START_SHIFT)

/* EDGE: ... */
#define ADC_CR_EDGE_SHIFT (27)
#define ADC_CR_EDGE (1 << ADC_CR_EDGE_SHIFT)

/* --- ADCx_GDR values ----------------------------------------------------- */

/* V_VREF: ... */
#define ADC_GDR_VVREF_SHIFT (6)
#define ADC_GDR_VVREF_MASK (0x3ff << ADC_GDR_V_VREF_SHIFT)
#define ADC_GDR_VVREF(x) ((x) << ADC_GDR_V_VREF_SHIFT)

/* CHN: ... */
#define ADC_GDR_CHN_SHIFT (24)
#define ADC_GDR_CHN_MASK (0x7 << ADC_GDR_CHN_SHIFT)
#define ADC_GDR_CHN(x) ((x) << ADC_GDR_CHN_SHIFT)

/* OVERRUN: ... */
#define ADC_GDR_OVERRUN_SHIFT (30)
#define ADC_GDR_OVERRUN (1 << ADC_GDR_OVERRUN_SHIFT)

/* SHIFT: ... */
#define ADC_GDR_DONE_SHIFT (31)
#define ADC_GDR_DONE (1 << ADC_GDR_DONE_SHIFT)

/* --- ADCx_INTEN values --------------------------------------------------- */

/* ADINTEN: ... */
#define ADC_INTEN_ADINTEN_SHIFT (0)
#define ADC_INTEN_ADINTEN_MASK (0xff << ADC_INTEN_ADINTEN_SHIFT)
#define ADC_INTEN_ADINTEN(x) ((x) << ADC_INTEN_ADINTEN_SHIFT)

/* ADGINTEN: ... */
#define ADC_INTEN_ADGINTEN_SHIFT (8)
#define ADC_INTEN_ADGINTEN (1 << ADC_INTEN_ADGINTEN_SHIFT)

/* --- ADCx_DRx values ---------------------------------------------------- */

/* V_VREF: ... */
#define ADC_DR_VVREF_SHIFT (6)
#define ADC_DR_VVREF_MASK (0x3ff << ADC_DR_VVREF_SHIFT)
#define ADC_DR_VVREF(x) ((x) << ADC_DR_VVREF_SHIFT)

/* OVERRUN: ... */
#define ADC_DR_OVERRUN_SHIFT (30)
#define ADC_DR_OVERRUN (1 << ADC_DR_OVERRUN_SHIFT)

/* DONE: ... */
#define ADC_DR_DONE_SHIFT (31)
#define ADC_DR_DONE (1 << ADC_DR_DONE_SHIFT)

/* --- ADCx_STAT values --------------------------------------------------- */

/* DONE: ... */
#define ADC_STAT_DONE_SHIFT (0)
#define ADC_STAT_DONE_MASK (0xff << ADC_STAT_DONE_SHIFT)
#define ADC_STAT_DONE(x) ((x) << ADC_STAT_DONE_SHIFT)

/* OVERUN: ... */
#define ADC_STAT_OVERUN_SHIFT (8)
#define ADC_STAT_OVERUN_MASK (0xff << ADC_STAT_OVERUN_SHIFT)
#define ADC_STAT_OVERUN(x) ((x) << ADC_STAT_OVERUN_SHIFT)

/* ADINT: ... */
#define ADC_STAT_ADINT_SHIFT (16)
#define ADC_STAT_ADINT (1 << ADC_STAT_ADINT_SHIFT)

typedef enum {
	ADC0_NUM = 0x0,
	ADC1_NUM = 0x1
} adc_num_t;

BEGIN_DECLS

void adc_disable(adc_num_t adc_num);
void adc_init(adc_num_t adc_num, uint8_t pins, uint8_t clkdiv, uint8_t clks);
void adc_start(adc_num_t adc_num);
void adc_read_to_buffer(adc_num_t adc_num, uint8_t pin, uint8_t *buf,
						uint16_t buf_len);

END_DECLS

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
