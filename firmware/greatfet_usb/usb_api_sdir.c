/*
 * This file is part of GreatFET
 */

#include "usb_api_sdir.h"

#include "sgpio.h"
#include "sgpio_isr.h"

#include <drivers/usb/lpc43xx/usb.h>
#include <drivers/usb/lpc43xx/usb_queue.h>
#include "usb_endpoint.h"
#include "usb_bulk_buffer.h"

#include <greatfet_core.h>
#include <gpio_lpc.h>
#include <gpio.h>
#include <pins.h>
#include <gpio_dma.h>

#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/lpc43xx/sgpio.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/timer.h>

volatile bool sdir_rx_enabled = false;
volatile bool sdir_tx_enabled = false;

static const sgpio_config_t sgpio_config = {
	.slice_mode_multislice = true,
	.clock_divider = 20,
};

static struct gpio_t ir_tx_sleep = GPIO(2, 9);

static uint32_t tx_samplerate;
static uint8_t dma_phase;

void sdir_dma_isr() {
	// gpio_dma_irq_tc_acknowledge();
	// Experimental
	GPDMA_INTTCCLEAR = GPDMA_INTTCCLEAR_INTTCCLEAR(0x20);
	// Switch phase so MCU can do USB things
	dma_phase = (dma_phase+1) % 4;
}

#define TIMER_CLK_SPEED 204000000
#define TIMER_PRESCALER 0
#define LLI_COUNT 4
#define USB_XFER_SIZE 0x4000

gpdma_lli_t dma_lli[LLI_COUNT];

void sdir_tx_stop() {
	timer_disable_counter(TIMER1);  /* Disable timers */
	timer_disable_counter(TIMER2);
	gpio_write(&ir_tx_sleep, 1);    /* Power down DAC */
	nvic_disable_irq(NVIC_DMA_IRQ); /* Disable DMA interrupt */
	gpio_dma_stop();                /* Disable DMA config */
	gpio_dma_irq_tc_acknowledge();
	gpio_dma_irq_err_clear();
}

void setup_tx_pins() {
	/* GPIO Tx pins */
#ifdef GREATFET_ONE
	int i;
	scu_pinmux(SCU_PINMUX_GPIO1_0, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_1, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_2, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_3, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_4, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_5, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_6, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_7, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);

	scu_pinmux(SCU_PINMUX_GPIO1_8, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_9, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_11, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_12, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO2_14, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(P5_4, (SCU_GPIO_FAST | SCU_CONF_FUNCTION5));

	static struct gpio_t ir_tx_cmode = GPIO(2, 11);
	static struct gpio_t ir_tx_mode = GPIO(2, 12);
	static struct gpio_t ir_tx_pin = GPIO(2, 14);
	static struct gpio_t ir_tx_refio = GPIO(1, 8);
	gpio_input(&ir_tx_cmode);
	gpio_input(&ir_tx_mode);
	gpio_input(&ir_tx_pin);
	gpio_input(&ir_tx_refio);

	static struct gpio_t ir_tx[] = {
		GPIO(1, 0),
		GPIO(1, 1),
		GPIO(1, 2),
		GPIO(1, 3),
		GPIO(1, 4),
		GPIO(1, 5),
		GPIO(1, 6),
		GPIO(1, 7)
	};
	for(i=0; i<8; i++)
		gpio_output(&ir_tx[i]);

	gpio_output(&ir_tx_sleep);
#endif
}

void sdir_tx_start() {
	setup_tx_pins();
	/* Make sure nothing is running before we configure it */
	sdir_tx_stop();

	/* Enable DAC */
	gpio_write(&ir_tx_sleep, 0);

	vector_table.irq[NVIC_DMA_IRQ] = sdir_dma_isr;
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
	void* const target = (void*)&(GPIO_LPC_PORT(1)->pin);
	gpio_dma_config_lli(dma_lli, LLI_COUNT, usb_bulk_buffer, target, USB_XFER_SIZE>>1);
	gpio_dma_tx_start(dma_lli);
	gpio_dma_init();
	timer_enable_counter(TIMER2);
	timer_enable_counter(TIMER1);
}

void sdir_tx_mode(void) {
	usb_endpoint_init(&usb0_endpoint_bulk_out);
	usb_transfer_schedule_block(
 		&usb0_endpoint_bulk_out,
 		&usb_bulk_buffer[0x0000],
 		USB_XFER_SIZE, 0, 0
 		);
	uint8_t usb_phase = 0;
	dma_phase = 0;

	sdir_tx_start();

	while(sdir_tx_enabled) {
 		// Set up OUT transfer of buffer 0.
 		if (((dma_phase == 2) || (dma_phase == 3)) && usb_phase == 1) {
 			usb_transfer_schedule_block(
 				&usb0_endpoint_bulk_out,
 				&usb_bulk_buffer[0x0000],
 				USB_XFER_SIZE, 0, 0
 				);
 			usb_phase = 0;
 		}
 		// Set up OUT transfer of buffer 1.
 		if ( ((dma_phase == 0) || (dma_phase == 1)) && usb_phase == 0) {
 			usb_transfer_schedule_block(
 				&usb0_endpoint_bulk_out,
 				&usb_bulk_buffer[USB_XFER_SIZE],
 				USB_XFER_SIZE, 0, 0
 			);
 			usb_phase = 1;
 		}
		//  delay(1000000);
	}
	sdir_tx_stop();
}

static void sdir_rx_start() {
	sgpio_configure_pin_functions(&sgpio_config);
	sgpio_configure(&sgpio_config, SGPIO_DIRECTION_INPUT);
	sgpio_clock_out_configure(20);

	/* Enable Gladiolus parts */
#ifdef GREATFET_ONE
	scu_pinmux(SCU_PINMUX_GPIO5_3, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_PINMUX_GPIO5_5, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);
#endif

	struct gpio_t gladiolus_powerdown = GPIO(5, 3);
	struct gpio_t gladiolus_enable = GPIO(5, 5);
	gpio_output(&gladiolus_enable);
	gpio_output(&gladiolus_powerdown);
	gpio_write(&gladiolus_enable, 1);
	gpio_write(&gladiolus_powerdown, 0);

	vector_table.irq[NVIC_SGPIO_IRQ] = sgpio_isr_input;

	nvic_set_priority(NVIC_SGPIO_IRQ, 0);
	nvic_enable_irq(NVIC_SGPIO_IRQ);
	SGPIO_SET_EN_1 = (1 << SGPIO_SLICE_A);
}

static void sdir_rx_stop() {
	SGPIO_CLR_EN_1 = (1 << SGPIO_SLICE_A);

	nvic_disable_irq(NVIC_SGPIO_IRQ);
}

void sdir_rx_mode(void) {
	usb_endpoint_init(&usb0_endpoint_bulk_in);

	sdir_rx_start();

	unsigned int phase = 1;
	while(sdir_rx_enabled) {
 		if ( usb_bulk_buffer_offset >= 0x4000
 		     && phase == 1) {
 			usb_transfer_schedule_block(
 				&usb0_endpoint_bulk_in,
 				&usb_bulk_buffer[0x0000],
 				0x4000, 0, 0
 				);
 			phase = 0;
 		}
 		// Set up IN transfer of buffer 1.
 		if ( usb_bulk_buffer_offset < 0x4000
 		     && phase == 0) {
 			usb_transfer_schedule_block(
 				&usb0_endpoint_bulk_in,
 				&usb_bulk_buffer[0x4000],
 				0x4000, 0, 0
 			);
 			phase = 1;
 		}
	}
	sdir_rx_stop();
	// usb_endpoint_disable(&usb0_endpoint_bulk_in);
}

usb_request_status_t usb_vendor_request_sdir_rx_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		sdir_rx_enabled = true;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_sdir_rx_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		sdir_rx_enabled = false;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_sdir_tx_start(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		tx_samplerate = ((uint32_t)endpoint->setup.value) << 16
		             | endpoint->setup.index;
		sdir_tx_enabled = true;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_sdir_tx_stop(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		sdir_tx_enabled = false;
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}
