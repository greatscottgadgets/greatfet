/*
 * This file is part of GreatFET
 */

#include "usb_api_operacake.h"
#include "usb_queue.h"

#include <gpio_dma.h>
#include <operacake.h>
#include <greatfet_core.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/timer.h>

static uint32_t tx_samplerate = 100000000;
static uint8_t dma_phase;

void operacake_dma_isr() {
	// gpio_dma_irq_tc_acknowledge();
	// Experimental
	GPDMA_INTTCCLEAR = GPDMA_INTTCCLEAR_INTTCCLEAR(0x20);
	led_toggle(LED3);
	led_toggle(LED4);
}

#define TIMER_CLK_SPEED 204000000
#define TIMER_PRESCALER 0
#define LLI_COUNT 4
#define SYMBOL_BUFFER_SIZE 4

gpdma_lli_t oc_dma_lli[LLI_COUNT];

uint32_t symbol_buffer[] = {
	0xFFFFFFFF, 0x00, 0xFFFFFFFF, 0x00
};

void operacake_tx_stop(void) {
	timer_disable_counter(TIMER1);  /* Disable timers */
	timer_disable_counter(TIMER2);
	nvic_disable_irq(NVIC_DMA_IRQ); /* Disable DMA interrupt */
	gpio_dma_stop();                /* Disable DMA config */
	gpio_dma_irq_tc_acknowledge();
	gpio_dma_irq_err_clear();
}

void operacake_tx_start(void) {
	/* Make sure nothing is running before we configure it */
	operacake_tx_stop();
	led_toggle(LED4);

	vector_table.irq[NVIC_DMA_IRQ] = operacake_dma_isr;
	nvic_set_priority(NVIC_DMA_IRQ, 0);
	nvic_enable_irq(NVIC_DMA_IRQ);
	/*
	 * TIMER1 produces the TX DAC clock signal
	 * TIMER2 is derived from TIMER1 and triggers DAC data signals
	 */
	TIMER1_MCR = TIMER_MCR_MR0R;
	TIMER1_MR0 = ((TIMER_CLK_SPEED / (2*(TIMER_PRESCALER + 1))) / tx_samplerate) - 1;
	TIMER1_MR3 = TIMER1_MR0;
	TIMER1_EMR = (TIMER_EMR_EMC_TOGGLE << TIMER_EMR_EMC0_SHIFT) | (TIMER_EMR_EMC_TOGGLE << TIMER_EMR_EMC3_SHIFT);
	timer_set_prescaler(TIMER1, TIMER_PRESCALER);
	timer_set_mode(TIMER1, TIMER_CTCR_MODE_TIMER);
	timer_reset(TIMER1);
	/* T1_MAT3 is connected to T2_CAP3 through the GIMA by default */
	TIMER2_MCR = (TIMER_MCR_MR0I | TIMER_MCR_MR0R);
	TIMER2_MR0 = 2;
	TIMER2_CCR = 0;
	timer_set_prescaler(TIMER2, TIMER_PRESCALER);
	timer_set_mode(TIMER2, (TIMER_CTCR_MODE_COUNTER_BOTH | TIMER_CTCR_CINSEL_CAPN_3));
	timer_reset(TIMER2);
	void* const target = (void*)&(GPIO_LPC_PORT(2)->pin);
	gpio_dma_config_lli(oc_dma_lli, LLI_COUNT, symbol_buffer, target, SYMBOL_BUFFER_SIZE>>1);
	gpio_dma_tx_start(oc_dma_lli);
	gpio_dma_init();
	timer_enable_counter(TIMER2);
	timer_enable_counter(TIMER1);
}

usb_request_status_t usb_vendor_request_operacake(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		operacake_init();
		operacake_gpio();
		operacake_tx_start();
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
