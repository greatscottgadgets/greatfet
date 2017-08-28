/*
 * This file is part of GreatFET
 */
 
#include <gpio_dma.h>

#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/gpdma.h>
#include <libopencm3/lpc43xx/scu.h>
#include <greatfet_core.h>
#include <gpio_lpc.h>
#include <gpio.h>
#include <pins.h>
#include <gpdma.h>

void gpio_dma_config_lli(
	gpdma_lli_t* const lli,
	const size_t lli_count,
	void* const buffer,
	void* const target_buffer,
	const size_t transfer_bytes
) {
	gpdma_lli_create_loop(lli, lli_count);

	for(size_t i=0; i<lli_count; i++) {
		void* const peripheral_address = (void*)target_buffer;
		void* const memory_address = buffer + (transfer_bytes * i);
		
		lli[i].csrcaddr =  memory_address;
		lli[i].cdestaddr = peripheral_address;
		lli[i].clli = (lli[i].clli & ~GPDMA_CLLI_LM_MASK) | GPDMA_CLLI_LM(1);
		lli[i].ccontrol =
			GPDMA_CCONTROL_TRANSFERSIZE(transfer_bytes>>2) |
			GPDMA_CCONTROL_SBSIZE(1) |
			GPDMA_CCONTROL_DBSIZE(0) |
			GPDMA_CCONTROL_SWIDTH(2) | // Four bytes
			GPDMA_CCONTROL_DWIDTH(0) | // One byte
			GPDMA_CCONTROL_S(1) |
			GPDMA_CCONTROL_D(1) |
			GPDMA_CCONTROL_SI(1) |
			GPDMA_CCONTROL_DI(0) |
			GPDMA_CCONTROL_PROT1(0) |
			GPDMA_CCONTROL_PROT2(0) |
			GPDMA_CCONTROL_PROT3(0) |
			GPDMA_CCONTROL_I(1)
			;
	}
}

static void gpio_dma_enable(const uint_fast8_t channel, const gpdma_lli_t* const lli) {
	gpdma_channel_disable(channel);
	gpdma_channel_interrupt_tc_clear(channel);
	gpdma_channel_interrupt_error_clear(channel);

	GPDMA_CSRCADDR(channel) = (uint32_t)lli->csrcaddr;
	GPDMA_CDESTADDR(channel) = (uint32_t)lli->cdestaddr;
	GPDMA_CLLI(channel) = (uint32_t)lli->clli;
	GPDMA_CCONTROL(channel) = lli->ccontrol;

	GPDMA_CCONFIG(channel) =
		GPDMA_CCONFIG_E(0) |
		GPDMA_CCONFIG_SRCPERIPHERAL(0) |
		GPDMA_CCONFIG_DESTPERIPHERAL(5) |
		GPDMA_CCONFIG_FLOWCNTRL(1) |  /* 1: Memory -> Peripheral */
		GPDMA_CCONFIG_IE(0) |
		GPDMA_CCONFIG_ITC(1) |
		GPDMA_CCONFIG_L(0) |
		GPDMA_CCONFIG_H(0)
		;

	gpdma_channel_enable(channel);
}

void gpio_dma_init() {
	/* DMA peripheral/source 5, option 0 (TIMER2 MR0) -- BREQ */
	CREG_DMAMUX &= ~(CREG_DMAMUX_DMAMUXPER5_MASK);
	CREG_DMAMUX |= CREG_DMAMUX_DMAMUXPER5(0x0);
	gpdma_controller_enable();
}

static const uint_fast8_t dma_channel_gpio = 5;

void gpio_dma_tx_start(const gpdma_lli_t* const start_lli) {
	gpio_dma_enable(dma_channel_gpio, start_lli);
}

void gpio_dma_irq_err_clear() {
	gpdma_channel_interrupt_error_clear(dma_channel_gpio);
}

void gpio_dma_irq_tc_acknowledge() {
	gpdma_channel_interrupt_tc_clear(dma_channel_gpio);
}

int gpio_dma_irq_is_error() {
	return gpdma_channel_interrupt_is_error(dma_channel_gpio);
}

void gpio_dma_stop() {
	gpdma_channel_disable(dma_channel_gpio);
}

size_t gpio_dma_current_transfer_index(
	const gpdma_lli_t* const lli,
	const size_t lli_count
) {
	const uint32_t next_lli = GPDMA_CLLI(dma_channel_gpio);
	for(size_t i=0; i<lli_count; i++) {
		if( lli[i].clli == next_lli ) {
			return i;
		}
	}
	return 0;
}
