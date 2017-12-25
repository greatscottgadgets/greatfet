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

/* CPLD JTAG interface GPIO pins */
static struct gpio_t gpio_tdo			= GPIO(5, 18);
static struct gpio_t gpio_tck			= GPIO(3,  0);
static struct gpio_t gpio_tms			= GPIO(3,  4);
static struct gpio_t gpio_tdi			= GPIO(3,  1);

/* This special variable is preserved across soft resets by a little bit of
 * reset handler magic. It allows us to pass a Reason across resets. */
volatile uint32_t reset_reason;

/**
 * The clock source for the main system oscillators.
 */
uint32_t main_clock_source = CGU_SRC_XTAL;


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



/* clock startup for Jellybean with Lemondrop attached
Configure PLL1 to max speed (204MHz).
Note: PLL1 clock is used by M4/M0 core, Peripheral, APB1. */
void cpu_clock_init(void)
{

	/* If we've been asked to reset in order to switch to using an external
	 * clock (e.g. for synchronization with other systems), use the GP_CLKIN
	 * instead of the XTAL as the main system clock source. */
	if(reset_reason == RESET_REASON_USE_EXTCLOCK) {
		// Switch the clock input pin into clock input mode.
		scu_pinmux(SCU_PINMUX_GP_CLKIN, SCU_GPIO_NOPULL | SCU_CONF_FUNCTION1);

		// And set our main clock source to the extclk.
		main_clock_source = CGU_SRC_GP_CLKIN;
	}

	// TODO: Figure out a place to do this explicitly?
	// We're done using the reset reason. Clear it so we don't grab a stale
	// reason in the future.
	reset_reason = RESET_REASON_UNKNOWN;

	/* use IRC as clock source for APB1 (including I2C0) */
	CGU_BASE_APB1_CLK = CGU_BASE_APB1_CLK_CLK_SEL(CGU_SRC_IRC);

	/* use IRC as clock source for APB3 (including ADC0) */
	CGU_BASE_APB3_CLK = CGU_BASE_APB3_CLK_CLK_SEL(CGU_SRC_IRC);

	//FIXME a lot of the details here should be in a CGU driver

	/* set xtal oscillator to low frequency mode */
	if(main_clock_source == CGU_SRC_XTAL) {
			CGU_XTAL_OSC_CTRL &= ~CGU_XTAL_OSC_CTRL_HF_MASK;

			/* power on the oscillator and wait until stable */
			CGU_XTAL_OSC_CTRL &= ~CGU_XTAL_OSC_CTRL_ENABLE_MASK;

			/* Wait about 100us after Crystal Power ON */
			delay(WAIT_CPU_CLOCK_INIT_DELAY);
	}

	/* use XTAL_OSC as clock source for BASE_M4_CLK (CPU) */
	CGU_BASE_M4_CLK = (CGU_BASE_M4_CLK_CLK_SEL(main_clock_source) | CGU_BASE_M4_CLK_AUTOBLOCK(1));

	/* use XTAL_OSC as clock source for APB1 */
	CGU_BASE_APB1_CLK = CGU_BASE_APB1_CLK_AUTOBLOCK(1)
			| CGU_BASE_APB1_CLK_CLK_SEL(main_clock_source);

	/* use XTAL_OSC as clock source for APB3 */
	CGU_BASE_APB3_CLK = CGU_BASE_APB3_CLK_AUTOBLOCK(1)
			| CGU_BASE_APB3_CLK_CLK_SEL(main_clock_source);

	cpu_clock_pll1_low_speed();

	/* use PLL1 as clock source for BASE_M4_CLK (CPU) */
	CGU_BASE_M4_CLK = (CGU_BASE_M4_CLK_CLK_SEL(CGU_SRC_PLL1) | CGU_BASE_M4_CLK_AUTOBLOCK(1));

	/* use XTAL_OSC as clock source for PLL0USB */
	CGU_PLL0USB_CTRL = CGU_PLL0USB_CTRL_PD(1)
			| CGU_PLL0USB_CTRL_AUTOBLOCK(1)
			| CGU_PLL0USB_CTRL_CLK_SEL(main_clock_source);
	while (CGU_PLL0USB_STAT & CGU_PLL0USB_STAT_LOCK_MASK);

	/* configure PLL0USB to produce 480 MHz clock from 12 MHz XTAL_OSC */
	/* Values from User Manual v1.4 Table 94, for 12MHz oscillator. */
	CGU_PLL0USB_MDIV = 0x06167FFA;
	CGU_PLL0USB_NP_DIV = 0x00302062;
	CGU_PLL0USB_CTRL |= (CGU_PLL0USB_CTRL_PD(1)
			| CGU_PLL0USB_CTRL_DIRECTI(1)
			| CGU_PLL0USB_CTRL_DIRECTO(1)
			| CGU_PLL0USB_CTRL_CLKEN(1));

	/* power on PLL0USB and wait until stable */
	CGU_PLL0USB_CTRL &= ~CGU_PLL0USB_CTRL_PD_MASK;
	while (!(CGU_PLL0USB_STAT & CGU_PLL0USB_STAT_LOCK_MASK));

	/* use PLL0USB as clock source for USB0 */
	CGU_BASE_USB0_CLK = CGU_BASE_USB0_CLK_AUTOBLOCK(1)
			| CGU_BASE_USB0_CLK_CLK_SEL(CGU_SRC_PLL0USB);

	/* use PLL0USB as clock source for IDIVA */
	/* divide by 4 */
	CGU_IDIVA_CTRL = CGU_IDIVA_CTRL_IDIV(3)
			| CGU_IDIVA_CTRL_AUTOBLOCK(1)
			| CGU_IDIVA_CTRL_CLK_SEL(CGU_SRC_PLL0USB);

	/* use IDIVA as clock source for IDIVB */
	/* divide by 2 */
	CGU_IDIVB_CTRL = CGU_IDIVB_CTRL_IDIV(1)
			| CGU_IDIVB_CTRL_AUTOBLOCK(1)
			| CGU_IDIVA_CTRL_CLK_SEL(CGU_SRC_IDIVA);

	/* Use IDIVB for CLKOUT */
	CGU_BASE_OUT_CLK = CGU_BASE_OUT_CLK_AUTOBLOCK(1)
			| CGU_BASE_OUT_CLK_CLK_SEL(CGU_SRC_IDIVB);

	/* use IDIVB as clock source for USB1 */
	CGU_BASE_USB1_CLK = CGU_BASE_USB1_CLK_AUTOBLOCK(1)
			| CGU_BASE_USB1_CLK_CLK_SEL(CGU_SRC_IDIVB);

	/* Switch peripheral clock over to use PLL1 (204MHz) */
	CGU_BASE_PERIPH_CLK = CGU_BASE_PERIPH_CLK_AUTOBLOCK(1)
			| CGU_BASE_PERIPH_CLK_CLK_SEL(CGU_SRC_PLL1);

	/* Switch APB1 clock over to use PLL1 (204MHz) */
	CGU_BASE_APB1_CLK = CGU_BASE_APB1_CLK_AUTOBLOCK(1)
			| CGU_BASE_APB1_CLK_CLK_SEL(CGU_SRC_PLL1);

	/* Switch APB3 clock over to use PLL1 (204MHz) */
	CGU_BASE_APB3_CLK = CGU_BASE_APB3_CLK_AUTOBLOCK(1)
			| CGU_BASE_APB3_CLK_CLK_SEL(CGU_SRC_PLL1);

	CGU_BASE_SSP0_CLK = CGU_BASE_SSP0_CLK_AUTOBLOCK(1)
			| CGU_BASE_SSP0_CLK_CLK_SEL(CGU_SRC_PLL1);

	CGU_BASE_SSP1_CLK = CGU_BASE_SSP1_CLK_AUTOBLOCK(1)
			| CGU_BASE_SSP1_CLK_CLK_SEL(CGU_SRC_PLL1);
}


/*
Configure PLL1 to low speed (48MHz).
Note: PLL1 clock is used by M4/M0 core, Peripheral, APB1.
This function shall be called after cpu_clock_init().
This function is mainly used to lower power consumption.
*/
void cpu_clock_pll1_low_speed(void)
{
	uint32_t pll_reg;

	/* Configure PLL1 Clock (48MHz) */
	/* Integer mode:
		FCLKOUT = M*(FCLKIN/N)
		FCCO = 2*P*FCLKOUT = 2*P*M*(FCLKIN/N)
	*/
	pll_reg = CGU_PLL1_CTRL;
	/* Clear PLL1 bits */
	pll_reg &= ~( CGU_PLL1_CTRL_CLK_SEL_MASK | CGU_PLL1_CTRL_PD_MASK | CGU_PLL1_CTRL_FBSEL_MASK |  /* CLK SEL, PowerDown , FBSEL */
				  CGU_PLL1_CTRL_BYPASS_MASK | /* BYPASS */
				  CGU_PLL1_CTRL_DIRECT_MASK | /* DIRECT */
				  CGU_PLL1_CTRL_PSEL_MASK | CGU_PLL1_CTRL_MSEL_MASK | CGU_PLL1_CTRL_NSEL_MASK ); /* PSEL, MSEL, NSEL- divider ratios */
	/* Set PLL1 up to 12MHz * 4 = 48MHz. */
	pll_reg |= CGU_PLL1_CTRL_CLK_SEL(main_clock_source)
				| CGU_PLL1_CTRL_PSEL(0)
				| CGU_PLL1_CTRL_NSEL(0)
				| CGU_PLL1_CTRL_MSEL(3)
				| CGU_PLL1_CTRL_FBSEL(1)
				| CGU_PLL1_CTRL_DIRECT(1);
	CGU_PLL1_CTRL = pll_reg;
	/* wait until stable */
	while (!(CGU_PLL1_STAT & CGU_PLL1_STAT_LOCK_MASK));

	/* Wait a delay after switch to new frequency with Direct mode */
	delay(WAIT_CPU_CLOCK_INIT_DELAY);
}

/*
Configure PLL1 (Main MCU Clock) to max speed (204MHz).
Note: PLL1 clock is used by M4/M0 core, Peripheral, APB1.
This function shall be called after cpu_clock_init().
*/
void cpu_clock_pll1_max_speed(void)
{
	uint32_t pll_reg;

	/* Configure PLL1 to Intermediate Clock (between 90 MHz and 110 MHz) */
	/* Integer mode:
		FCLKOUT = M*(FCLKIN/N)
		FCCO = 2*P*FCLKOUT = 2*P*M*(FCLKIN/N)
	*/
	pll_reg = CGU_PLL1_CTRL;
	/* Clear PLL1 bits */
	pll_reg &= ~( CGU_PLL1_CTRL_CLK_SEL_MASK | CGU_PLL1_CTRL_PD_MASK | CGU_PLL1_CTRL_FBSEL_MASK |  /* CLK SEL, PowerDown , FBSEL */
				  CGU_PLL1_CTRL_BYPASS_MASK | /* BYPASS */
				  CGU_PLL1_CTRL_DIRECT_MASK | /* DIRECT */
				  CGU_PLL1_CTRL_PSEL_MASK | CGU_PLL1_CTRL_MSEL_MASK | CGU_PLL1_CTRL_NSEL_MASK ); /* PSEL, MSEL, NSEL- divider ratios */
	/* Set PLL1 up to 12MHz * 8 = 96MHz. */
	pll_reg |= CGU_PLL1_CTRL_CLK_SEL(main_clock_source)
				| CGU_PLL1_CTRL_PSEL(0)
				| CGU_PLL1_CTRL_NSEL(0)
				| CGU_PLL1_CTRL_MSEL(7)
				| CGU_PLL1_CTRL_FBSEL(1);
	CGU_PLL1_CTRL = pll_reg;
	/* wait until stable */
	while (!(CGU_PLL1_STAT & CGU_PLL1_STAT_LOCK_MASK));

	/* Wait before to switch to max speed */
	delay(WAIT_CPU_CLOCK_INIT_DELAY);

	/* Configure PLL1 Max Speed */
	/* Direct mode: FCLKOUT = FCCO = M*(FCLKIN/N) */
	pll_reg = CGU_PLL1_CTRL;
	/* Clear PLL1 bits */
	pll_reg &= ~( CGU_PLL1_CTRL_CLK_SEL_MASK | CGU_PLL1_CTRL_PD_MASK | CGU_PLL1_CTRL_FBSEL_MASK |  /* CLK SEL, PowerDown , FBSEL */
				  CGU_PLL1_CTRL_BYPASS_MASK | /* BYPASS */
				  CGU_PLL1_CTRL_DIRECT_MASK | /* DIRECT */
				  CGU_PLL1_CTRL_PSEL_MASK | CGU_PLL1_CTRL_MSEL_MASK | CGU_PLL1_CTRL_NSEL_MASK ); /* PSEL, MSEL, NSEL- divider ratios */
	/* Set PLL1 up to 12MHz * 17 = 204MHz. */
	pll_reg |= CGU_PLL1_CTRL_CLK_SEL(main_clock_source)
			| CGU_PLL1_CTRL_PSEL(0)
			| CGU_PLL1_CTRL_NSEL(0)
			| CGU_PLL1_CTRL_MSEL(16)
			| CGU_PLL1_CTRL_FBSEL(1)
			| CGU_PLL1_CTRL_DIRECT(1);
	CGU_PLL1_CTRL = pll_reg;
	/* wait until stable */
	while (!(CGU_PLL1_STAT & CGU_PLL1_STAT_LOCK_MASK));

}

void rtc_init(void) {
#ifdef BOARD_CAPABILITY_RTC
		/* Enable power to 32 KHz oscillator */
		CREG_CREG0 &= ~CREG_CREG0_PD32KHZ;
		/* Release 32 KHz oscillator reset */
		CREG_CREG0 &= ~CREG_CREG0_RESET32KHZ;
		/* Enable 1 KHz output (required per LPC43xx user manual section 37.2) */
		CREG_CREG0 |= CREG_CREG0_EN1KHZ;
		/* Release CTC Reset */
		RTC_CCR &= ~RTC_CCR_CTCRST(1);
		/* Disable calibration counter */
		RTC_CCR &= ~RTC_CCR_CCALEN(1);
		/* Enable clock */
		RTC_CCR |= RTC_CCR_CLKEN(1);
#endif
}

void pin_setup(void) {
	int i;

	/* Release CPLD JTAG pins */
	scu_pinmux(SCU_PINMUX_TDO, SCU_GPIO_NOPULL | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_PINMUX_TCK, SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_TMS, SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_TDI, SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0);

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

	/* Configure external clock in */
	scu_pinmux(CLK0, SCU_CONF_FUNCTION1 | SCU_CLK_OUT);

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

