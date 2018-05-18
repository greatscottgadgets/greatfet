#include "clocks.h"
#include "pins.h"
#include "greatfet_core.h"
#include <libopencm3/lpc43xx/cgu.h>
#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/rtc.h>
#include <libopencm3/lpc43xx/scu.h>

#define WAIT_CPU_CLOCK_INIT_DELAY   (10000)

/**
 * The clock source for the main system oscillators.
 */
uint32_t main_clock_source = CGU_SRC_XTAL;


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

	/* Use the GP input clock to drive the clock out; but disable it initially. */
	CGU_BASE_OUT_CLK = CGU_BASE_OUT_CLK_AUTOBLOCK(1)
			| CGU_BASE_OUT_CLK_CLK_SEL(CGU_SRC_GP_CLKIN) | CGU_BASE_OUT_CLK_PD(1);

	/* use IDIVB as clock source for USB1 */
	CGU_BASE_USB1_CLK = CGU_BASE_USB1_CLK_AUTOBLOCK(1)
			| CGU_BASE_USB1_CLK_CLK_SEL(CGU_SRC_IDIVB);

	/* Disable PLL0AUDIO clock at startup */
	CGU_PLL0AUDIO_CTRL &= ~CGU_PLL0AUDIO_CTRL_CLKEN(1);

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

#define PLL0_MSEL_MAX (1<<15)
/* multiplier: compute mdec from msel */
uint32_t mdec(uint16_t msel) {
	uint32_t x=0x4000, im;
	switch (msel) {
	case 0:
		x = 0xFFFFFFFF;
		break;
	case 1:
		x = 0x18003;
		break;
	case 2:
		x = 0x10003;
		break;
	default:
		for (im = msel; im <= PLL0_MSEL_MAX; im++)
			x = ((x ^ x>>1) & 1) << 14 | (x>>1 & 0xFFFF);
	}
	return x;
}

#define PLL0_NSEL_MAX (1<<8)
/* pre-divider: compute ndec from nsel */
uint32_t ndec(uint8_t nsel) {
	uint32_t x=0x80, in;
	switch (nsel) {
	case 0: return 0xFFFFFFFF;
		break;
	case 1: return 0x302;
		break;
	case 2: return 0x202;
		break;
	default:
		for (in = nsel; in <= PLL0_NSEL_MAX; in++)
			x = ((x ^ x>>2 ^ x>>3 ^ x>>4) & 1) << 7 | (x>>1 & 0xFF);
	}
	return x;
}

/* PLL0AUDIO */
uint8_t pll0audio_config(void)
{
	/* use XTAL_OSC as clock source for PLL0AUDIO */
	CGU_PLL0AUDIO_CTRL = CGU_PLL0AUDIO_CTRL_PD(1)
			| CGU_PLL0AUDIO_CTRL_AUTOBLOCK(1)
			| CGU_PLL0AUDIO_CTRL_CLK_SEL(CGU_SRC_XTAL);
	while (CGU_PLL0AUDIO_STAT & CGU_PLL0AUDIO_STAT_LOCK_MASK);

	/* configure PLL0AUDIO to produce 433.90MHz clock from 12 MHz XTAL_OSC */
	/* nsel = 240 - gives us a frequency step of 50 kHz
     * msel = 8678
     * Trying to work this out? See the CGU section of the user manual
     * We're using mode 1c (input pre-divider, direct output)
     * That means we set the N pre-divider and the M multiplier
     * We do not use the P post-divider
    */
	CGU_PLL0AUDIO_MDIV = mdec(8678);
	CGU_PLL0AUDIO_NP_DIV = ndec(240);
	CGU_PLL0AUDIO_CTRL |= (CGU_PLL0AUDIO_CTRL_PD(1)
			| CGU_PLL0AUDIO_CTRL_DIRECTI(0)
			| CGU_PLL0AUDIO_CTRL_DIRECTO(1)
			| CGU_PLL0AUDIO_CTRL_SEL_EXT(1));

	/* power on PLL0AUDIO and wait until stable */
	CGU_PLL0AUDIO_CTRL &= ~CGU_PLL0AUDIO_CTRL_PD_MASK;
	//while (!(CGU_PLL0AUDIO_STAT & CGU_PLL0AUDIO_STAT_LOCK_MASK));

	/* Use PLL0AUDIO for CLKOUT */
	CGU_BASE_OUT_CLK = CGU_BASE_OUT_CLK_AUTOBLOCK(1)
			| CGU_BASE_OUT_CLK_CLK_SEL(CGU_SRC_PLL0AUDIO);

	/* Enable PLL0AUDIO and blast out CLKOUT */
	CGU_PLL0AUDIO_CTRL |= CGU_PLL0AUDIO_CTRL_CLKEN(1);
	return 0;
}

void pll0audio_on(void)
{
    /* Enable PLL0AUDIO and blast out CLKOUT */
	CGU_PLL0AUDIO_CTRL |= CGU_PLL0AUDIO_CTRL_CLKEN(1);
}

void pll0audio_off(void)
{
	CGU_PLL0AUDIO_CTRL |= CGU_PLL0AUDIO_CTRL_CLKEN(0);
}

uint8_t pll0audio_tune(uint16_t msel)
{
    if(msel < PLL0_MSEL_MAX)
        return 1;
	CGU_PLL0AUDIO_MDIV = mdec(msel);
    return 0;
}
