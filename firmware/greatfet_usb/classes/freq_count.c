/*
 * This file is part of GreatFET
 */

#include <drivers/comms.h>
#include <debug.h>

#include <libopencm3/lpc43xx/timer.h>
#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/gima.h>
#include <libopencm3/lpc43xx/gpdma.h>
#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/ccu.h>

#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#define CLASS_NUMBER_SELF (0x117)

/* DMA linked list item */
typedef struct {
	uint32_t src;
	uint32_t dest;
	uint32_t next_lli;
	uint32_t control;
} dma_lli;

/* timer control register configuration sequence */
typedef struct {
	uint32_t first_tcr;
	uint32_t second_tcr;
} tcr_sequence;

dma_lli timer_dma_lli;
tcr_sequence reset;

static int setup_counters(struct command_transaction *trans)
{
    /* Get number of cycles to measure over */
    uint32_t measurement_cycles = comms_argument_parse_uint32_t(trans);
    if (!comms_transaction_okay(trans)) {
        return EBADMSG;
    }

    /* Enable timer clocks */
    CCU1_CLK_M4_TIMER1_CFG = 1;
    CCU1_CLK_M4_TIMER2_CFG = 1;

    /* Timer1 triggers periodic measurement */
    timer_set_prescaler(TIMER1, 0);
    timer_set_mode(TIMER1, TIMER_CTCR_MODE_TIMER);
    TIMER1_MCR = TIMER_MCR_MR0R;
    TIMER1_EMR = (TIMER_EMR_EMC_SET << TIMER_EMR_EMC0_SHIFT) |
	    (TIMER_EMR_EMC_TOGGLE << TIMER_EMR_EMC3_SHIFT);
    TIMER1_MR3 = measurement_cycles;
    TIMER1_MR0 = measurement_cycles;

    /* prevent TIMER1_MR3 from interfering with SCT */
    CREG_CREG6 |= CREG_CREG6_CTOUTCTRL;

    /* Timer2 counts CLKIN */
    timer_set_prescaler(TIMER2, 0);
    TIMER2_CCR = TIMER_CCR_CAP3RE;
    GIMA_CAP2_3_IN = 0x20; // T1_MAT3

    /* measure CLKIN_DETECT signal on P1_6, CTIN_5 */
    TIMER2_CTCR = TIMER_CTCR_MODE_COUNTER_RISING | TIMER_CTCR_CINSEL_CAPN_2;
    scu_pinmux(P1_6, SCU_GPIO_PDN | SCU_CONF_FUNCTION1); // CTIN_5
    GIMA_CAP2_2_IN = 0x00;                               // CTIN_5

    reset.first_tcr = TIMER_TCR_CEN | TIMER_TCR_CRST;
    reset.second_tcr = TIMER_TCR_CEN;
    timer_dma_lli.src = (uint32_t) & (reset);
    timer_dma_lli.dest = (uint32_t) & (TIMER2_TCR);
    timer_dma_lli.next_lli = (uint32_t) & (timer_dma_lli);
    timer_dma_lli.control = GPDMA_CCONTROL_TRANSFERSIZE(2) |
	    GPDMA_CCONTROL_SBSIZE(0)   // 1
	    | GPDMA_CCONTROL_DBSIZE(0) // 1
	    | GPDMA_CCONTROL_SWIDTH(2) // 32-bit word
	    | GPDMA_CCONTROL_DWIDTH(2) // 32-bit word
	    | GPDMA_CCONTROL_S(0)      // AHB Master 0
	    | GPDMA_CCONTROL_D(1)      // AHB Master 1
	    | GPDMA_CCONTROL_SI(1)     // increment source
	    | GPDMA_CCONTROL_DI(0)     // do not increment destination
	    | GPDMA_CCONTROL_PROT1(0)  // user mode
	    | GPDMA_CCONTROL_PROT2(0)  // not bufferable
	    | GPDMA_CCONTROL_PROT3(0)  // not cacheable
	    | GPDMA_CCONTROL_I(0);     // interrupt disabled

    /* Enable GPDMA controller */
    GPDMA_CONFIG |= GPDMA_CONFIG_E(1);
    while ((GPDMA_CONFIG & GPDMA_CONFIG_E_MASK) == 0) {}

    GPDMA_C0SRCADDR = timer_dma_lli.src;
    GPDMA_C0DESTADDR = timer_dma_lli.dest;
    GPDMA_C0LLI = timer_dma_lli.next_lli;
    GPDMA_C0CONTROL = timer_dma_lli.control;
    GPDMA_C0CONFIG = GPDMA_CCONFIG_DESTPERIPHERAL(0x3) // T1_MAT0
	    | GPDMA_CCONFIG_FLOWCNTRL(1)               // memory-to-peripheral
	    | GPDMA_CCONFIG_H(0);                      // do not halt

    /* Enable GPDMA channel */
    GPDMA_CCONFIG(0) |= GPDMA_CCONFIG_E(1);

    /* start counting */
    timer_reset(TIMER2);
    timer_reset(TIMER1);
    timer_enable_counter(TIMER2);
    timer_enable_counter(TIMER1);

    return 0;
}

static int count_cycles(struct command_transaction *trans)
{
    uint32_t counted_cycles = TIMER2_CR3;
    
    comms_response_add_uint32_t(trans, counted_cycles);

    return 0;
}

static struct comms_verb freq_verbs[] = {
    {
	.verb_number = 0x0,
	.name = "setup_counters",
	.handler = setup_counters,
	.in_signature = "<I",
	.out_signature = "",
	.in_param_names = "measurement_cycles",
	.out_param_names = "",
        .doc = "Sets up counters to measure frequency.",
    },
    {
	.verb_number = 0x1,
	.name = "count_cycles",
	.handler = count_cycles,
	.in_signature = "",
	.out_signature = "<I",
	.in_param_names = "",
	.out_param_names = "counted_cycles",
        .doc = "Returns cycles within the last measurement period.",
    },
    {}
};
COMMS_DEFINE_SIMPLE_CLASS(freq_count, CLASS_NUMBER_SELF, "freq_count", freq_verbs,
        "API for measuring frequencies by counting cycles of a clock signal");

