/*
 * This file is part of GreatFET
 */

#include "greatfet_core.h"
#include "pins.h"
#include <libopencm3/cm3/scb.h>
#include <libopencm3/lpc43xx/creg.h>

#include "gpio_lpc.h"

/* Symbols exported by the linker script(s): */
extern unsigned _data_loadaddr, _data, _edata, _bss, _ebss, _stack;
typedef void (*funcp_t) (void);
extern funcp_t __preinit_array_start, __preinit_array_end;
extern funcp_t __init_array_start, __init_array_end;
extern funcp_t __fini_array_start, __fini_array_end;
extern unsigned _etext_ram, _text_ram, _etext_rom;

void main(void);

/* TODO: Consolidate ARRAY_SIZE declarations */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* USB Target interface */
// FIXME: move these to board-specific functions so we don't assume a load switch
// for all boards
#ifdef BOARD_CAPABILITY_USB1_SENSE_VBUS
struct gpio_t gpio_usb1_sense	= GPIO(SCU_PINMUX_USB1_SENSE_PORT, SCU_PINMUX_USB1_SENSE_PIN);
#endif
#ifdef BOARD_CAPABILITY_USB1_PROVIDE_VBUS
struct gpio_t gpio_usb1_en	= GPIO(SCU_PINMUX_USB1_EN_PORT, SCU_PINMUX_USB1_EN_PIN);
#endif

/* CPLD JTAG interface GPIO pins */
static struct gpio_t gpio_tdo			= GPIO(5, 18);
static struct gpio_t gpio_tck			= GPIO(3,  0);
static struct gpio_t gpio_tms			= GPIO(3,  4);
static struct gpio_t gpio_tdi			= GPIO(3,  1);

/* This special variable is preserved across soft resets by a little bit of
 * reset handler magic. It allows us to pass a Reason across resets. */
volatile uint32_t reset_reason;


i2c_bus_t i2c0 = {
	.obj = (void*)I2C0_BASE,
	.start = i2c_lpc_start,
	.stop = i2c_lpc_stop,
	.transfer = i2c_lpc_transfer,
};

i2c_bus_t i2c1 = {
	.obj = (void*)I2C1_BASE,
	.start = i2c_lpc_start,
	.stop = i2c_lpc_stop,
	.transfer = i2c_lpc_transfer,
};

const i2c_lpc_config_t i2c_config_slow_clock = {
	.duty_cycle_count = 15,
};

const i2c_lpc_config_t i2c_config_fast_clock = {
	.duty_cycle_count = 255,
};

const ssp_config_t ssp_config_spi = {
	.data_bits = SSP_DATA_8BITS,
	.serial_clock_rate = 2,
	.clock_prescale_rate = 2,
};

spi_bus_t spi_bus_ssp0 = {
	.obj = (void*)SSP0_BASE,
	.config = &ssp_config_spi,
	.start = spi_ssp_start,
	.stop = spi_ssp_stop,
	.transfer = spi_ssp_transfer,
	.transfer_gather = spi_ssp_transfer_gather,
};

const ssp_config_t ssp1_config_spi = {
	.data_bits = SSP_DATA_8BITS,
	.serial_clock_rate = 2,
	.clock_prescale_rate = 100,
};

spi_bus_t spi_bus_ssp1 = {
	.obj = (void*)SSP1_BASE,
	.config = &ssp1_config_spi,
	.start = spi_ssp_start,
	.stop = spi_ssp_stop,
	.transfer = spi_ssp_transfer,
	.transfer_gather = spi_ssp_transfer_gather,
};

#define DELAY_CLK_SPEED 204000000
#define DELAY_PRESCALER 0
void delay(uint32_t duration)
{
	uint32_t i;

	for (i = 0; i < duration; i++)
		__asm__("nop");
}



static void pre_main(void)
{
	volatile unsigned *src, *dest;

	/* Copy the code from ROM to Real RAM (if enabled) */
	if ((&_etext_ram-&_text_ram) > 0) {
		src = &_etext_rom-(&_etext_ram-&_text_ram);
		/* Change Shadow memory to ROM (for Debug Purpose in case Boot
		 * has not set correctly the M4MEMMAP because of debug)
		 */
		CREG_M4MEMMAP = (unsigned long)src;

		for (dest = &_text_ram; dest < &_etext_ram; ) {
			*dest++ = *src++;
		}

		/* Change Shadow memory to Real RAM */
		CREG_M4MEMMAP = (unsigned long)&_text_ram;

		/* Continue Execution in RAM */
	}

	/* Enable access to Floating-Point coprocessor. */
	SCB_CPACR |= SCB_CPACR_FULL * (SCB_CPACR_CP10 | SCB_CPACR_CP11);
}


/**
 * Startup code for the processor.
 */
void __attribute__ ((naked)) reset_handler(void)
{
	volatile unsigned *src, *dest;
	funcp_t *fp;

	uint32_t stored_reset_reason = reset_reason;

	for (src = &_data_loadaddr, dest = &_data;
		dest < &_edata;
		src++, dest++) {
		*dest = *src;
	}

	for (dest = &_bss; dest < &_ebss; ) {
		*dest++ = 0;
	}

	/* Constructors. */
	for (fp = &__preinit_array_start; fp < &__preinit_array_end; fp++) {
		(*fp)();
	}
	for (fp = &__init_array_start; fp < &__init_array_end; fp++) {
		(*fp)();
	}

	/* might be provided by platform specific vector.c */
	pre_main();

	/* Restore our stored reset reason. */
	reset_reason = stored_reset_reason;

	/* Call the application's entry point. */
	main();

	/* Destructors. */
	for (fp = &__fini_array_start; fp < &__fini_array_end; fp++) {
		(*fp)();
	}

}


void pin_setup(void) {
	int i;

	/* Release CPLD JTAG pins */
	scu_pinmux(SCU_PINMUX_TDO, SCU_GPIO_NOPULL | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_PINMUX_TCK, SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_TMS, SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_TDI, SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0);

	/* By default, use CLK0 as an external clock. */
	scu_pinmux(CLK0, SCU_CLK_OUT | SCU_CONF_FUNCTION1);

	gpio_input(&gpio_tdo);
	gpio_input(&gpio_tck);
	gpio_input(&gpio_tms);
	gpio_input(&gpio_tdi);

	/* Configure all GPIO as Input (safe state) */
	gpio_init();

	/* Configure each of the LEDs. */
	for (i = 0; i < NUM_LEDS; ++i) {
		scu_pinmux(pinmux_led[i], scu_type_led[i]);
		gpio_output(&gpio_led[i]);
		gpio_set(&gpio_led[i]); /* led off */
	}

	/* enable input on SCL and SDA pins */
	SCU_SFSI2C0 = SCU_I2C0_NOMINAL;

#ifdef BOARD_CAPABILITY_USB1_PROVIDE_VBUS
	/* Set up the load switch that we'll use if we want to play host on USB1. */
	/* Default to off, as we don't want to dual-drive VBUS. */
	scu_pinmux(SCU_PINMUX_USB1_EN, SCU_CONF_FUNCTION0);
	gpio_output(&gpio_usb1_en);
	gpio_clear(&gpio_usb1_en);
#endif

#ifdef BOARD_CAPABILITY_USB1_SENSE_VBUS
	/* Set up the GPIO we'll be using to sense the presence of USB1 VBUS. */
	scu_pinmux(SCU_PINMUX_USB1_SENSE, SCU_CONF_FUNCTION0);
	gpio_input(&gpio_usb1_sense);
#endif
}

void led_on(const led_t led) {
	if(led >= NUM_LEDS)
		return;

	gpio_clear(&gpio_led[led]);
}

void led_off(const led_t led) {
	if(led >= NUM_LEDS)
		return;

	gpio_set(&gpio_led[led]);
}

void led_toggle(const led_t led) {
	if(led >= NUM_LEDS)
		return;

	gpio_toggle(&gpio_led[led]);
}

/* Temporary LED based debugging */
void debug_led(uint8_t val) {
	if(val & 0x1)
		led_on(LED1);
	else
		led_off(LED1);

	if(val & 0x2)
		led_on(LED2);
	else
		led_off(LED2);

	if(val & 0x4)
		led_on(LED3);
	else
		led_off(LED3);

	if(val & 0x8)
		led_on(LED4);
	else
		led_off(LED4);
}

