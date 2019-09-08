/*
 * This file is part of GreatFET
 */

#include "greatfet_core.h"
#include "pins.h"
#include "spi_ssp.h"
#include "spiflash.h"
#include "spiflash_target.h"
#include "i2c_bus.h"
#include "i2c_lpc.h"
#include <libopencm3/cm3/scb.h>
#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/cgu.h>
#include <libopencm3/lpc43xx/rtc.h>
#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/ssp.h>

#include <drivers/timer.h>

#include "time.h"
#include "gpio_lpc.h"

#include "debug.h"

#define RTC_BRINGUP_TIMEOUT_US (1024 * 100)


/* TODO: Consolidate ARRAY_SIZE declarations */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define WAIT_CPU_CLOCK_INIT_DELAY   (10000)

/* USB Target interface */
// FIXME: move these to board-specific functions so we don't assume a load switch
// for all boards
#ifdef BOARD_CAPABILITY_USB1_SENSE_VBUS
struct gpio_t gpio_usb1_sense	= GPIO(SCU_PINMUX_USB1_SENSE_PORT, SCU_PINMUX_USB1_SENSE_PIN);
#endif
#ifdef BOARD_CAPABILITY_USB1_PROVIDE_VBUS
struct gpio_t gpio_usb1_en	= GPIO(SCU_PINMUX_USB1_EN_PORT, SCU_PINMUX_USB1_EN_PIN);
#endif

i2c_bus_t i2c0 = {
	.obj = (void*)I2C0_BASE,
	.start = i2c_lpc_start,
	.stop = i2c_lpc_stop,
	.read = i2c_lpc_read,
	.write = i2c_lpc_write
};

i2c_bus_t i2c1 = {
	.obj = (void*)I2C1_BASE,
	.start = i2c_lpc_start,
	.stop = i2c_lpc_stop,
	.read = i2c_lpc_read,
	.write = i2c_lpc_write
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
	.transfer_data = spi_ssp_transfer_data,
	.transfer_gather = spi_ssp_transfer_gather,
	.transfer_gather_partial = spi_ssp_transfer_gather_partial,
};

const ssp_config_t ssp1_config_spi = {
	.data_bits = SSP_DATA_8BITS,
	.serial_clock_rate = 8,
	.clock_prescale_rate = 8,
};

spi_bus_t spi_bus_ssp1 = {
	.obj = (void*)SSP1_BASE,
	.config = &ssp1_config_spi,
	.start = spi_ssp_start,
	.stop = spi_ssp_stop,
	.transfer = spi_ssp_transfer,
	.transfer_data = spi_ssp_transfer_data,
	.transfer_gather = spi_ssp_transfer_gather,
	.transfer_gather_partial = spi_ssp_transfer_gather_partial,
};

#define DELAY_CLK_SPEED 204000000
#define DELAY_PRESCALER 0
void delay(uint32_t duration)
{
	uint32_t i;

	for (i = 0; i < duration; i++)
		__asm__("nop");
}

void rtc_init(void)
{
        pr_info("Board does not advertise an RTC. Not bringing up RTC oscillator.");
}

void pin_setup(void) {
	int i;

	pr_info("Configuring board pins...\n");

	/* Configure all GPIO as Input (safe state) */
	gpio_init();

	/* GreatFET SPI pins / SSP1 pins. */
	scu_pinmux(SCU_SSP1_SCK,  SCU_SSP_IO | SCU_SSP1_SCK_FUNC);
	scu_pinmux(SCU_SSP1_MISO, SCU_SSP_IO | SCU_SSP1_MISO_FUNC);
	scu_pinmux(SCU_SSP1_MOSI, SCU_SSP_IO | SCU_SSP1_MOSI_FUNC);
	scu_pinmux(SCU_SSP1_SSEL, SCU_SSP_IO | SCU_SSP1_SSEL_FUNC);

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

