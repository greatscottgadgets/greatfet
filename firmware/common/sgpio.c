/*
 * This file is part of GreatFET
 */

#include <pins.h>

#include <libopencm3/lpc43xx/scu.h>
#include <libopencm3/lpc43xx/sgpio.h>

#include <sgpio.h>

void sgpio_configure_pin_functions(const sgpio_config_t* const config) {
	(void)config;

	scu_pinmux(SCU_PINMUX_SGPIO0, SCU_GPIO_FAST | SCU_CONF_FUNCTION3);
	scu_pinmux(SCU_PINMUX_SGPIO1, SCU_GPIO_FAST | SCU_CONF_FUNCTION3);
	scu_pinmux(SCU_PINMUX_SGPIO2, SCU_GPIO_FAST | SCU_CONF_FUNCTION2);
	scu_pinmux(SCU_PINMUX_SGPIO3, SCU_GPIO_FAST | SCU_CONF_FUNCTION2);
	scu_pinmux(SCU_PINMUX_SGPIO4, SCU_GPIO_FAST | SCU_CONF_FUNCTION2);
	scu_pinmux(SCU_PINMUX_SGPIO5, SCU_GPIO_FAST | SCU_CONF_FUNCTION2);
	scu_pinmux(SCU_PINMUX_SGPIO6, SCU_GPIO_FAST | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_SGPIO7, SCU_GPIO_FAST | SCU_CONF_FUNCTION6);
}

void sgpio_configure(
	const sgpio_config_t* const config,
	const sgpio_direction_t direction
) {
	// Disable all counters during configuration
	SGPIO_CTRL_ENABLE = 0;

	// Enable SGPIO pin outputs.
	const uint_fast16_t sgpio_gpio_data_direction =
		((direction == SGPIO_DIRECTION_OUTPUT) ? 0xff : 0x00) << 0;
	SGPIO_GPIO_OENREG = sgpio_gpio_data_direction
		;

	const uint_fast8_t output_multiplexing_mode =
		config->slice_mode_multislice ? 0xb : 0x9;
	/* SGPIO0 to SGPIO7 */
	for(uint_fast8_t i=0; i<8; i++) {
		// SGPIO pin 0 outputs slice A bit "i".
		SGPIO_OUT_MUX_CFG(i) =
		      SGPIO_OUT_MUX_CFG_P_OE_CFG(0)
		    | SGPIO_OUT_MUX_CFG_P_OUT_CFG(output_multiplexing_mode) /* 11/0xB=dout_doutm8c (8-bit mode 8c)(multislice L0/7, N0/7), 9=dout_doutm8a (8-bit mode 8a)(A0/7,B0/7) */
			;
	}

	const uint_fast8_t slice_indices[] = {
		SGPIO_SLICE_A,
		SGPIO_SLICE_I,
		SGPIO_SLICE_E,
		SGPIO_SLICE_J,
		SGPIO_SLICE_C,
		SGPIO_SLICE_K,
		SGPIO_SLICE_F,
		SGPIO_SLICE_L,
	};
	
	const uint_fast8_t pos = config->slice_mode_multislice ? 0x1f : 0x03;
	const bool single_slice = !config->slice_mode_multislice;
	const uint_fast8_t slice_count = config->slice_mode_multislice ? 8 : 1;
	
	uint32_t slice_enable_mask = 0;
	/* Configure Slice A, I, E, J, C, K, F, L (sgpio_slice_mode_multislice mode) */
	for(uint_fast8_t i=0; i<slice_count; i++)
	{
		const uint_fast8_t slice_index = slice_indices[i];
		const bool input_slice = (i == 0) && (direction != SGPIO_DIRECTION_OUTPUT); /* Only for slice0/A and RX mode set input_slice to 1 */
		const uint_fast8_t concat_order = (input_slice || single_slice) ? 0 : 3; /* 0x0=Self-loop(slice0/A RX mode), 0x3=8 slices */
		const uint_fast8_t concat_enable = (input_slice || single_slice) ? 0 : 1; /* 0x0=External data pin(slice0/A RX mode), 0x1=Concatenate data */
		
		SGPIO_MUX_CFG(slice_index) =
		      SGPIO_MUX_CFG_CONCAT_ORDER(concat_order)
		    | SGPIO_MUX_CFG_CONCAT_ENABLE(concat_enable)
		    | SGPIO_MUX_CFG_QUALIFIER_SLICE_MODE(0) /* Don't care */
		    | SGPIO_MUX_CFG_QUALIFIER_PIN_MODE(0) /* Don't care */
		    | SGPIO_MUX_CFG_QUALIFIER_MODE(0) /* Always enabled */
		    | SGPIO_MUX_CFG_CLK_SOURCE_SLICE_MODE(0) /* Select clock source slice D(0x0) */
		    | SGPIO_MUX_CFG_CLK_SOURCE_PIN_MODE(0) /* Don't care */
			| SGPIO_MUX_CFG_EXT_CLK_ENABLE(0) /* Internal clock signal (slice) */
			;

		SGPIO_SLICE_MUX_CFG(slice_index) =
		      SGPIO_SLICE_MUX_CFG_INV_QUALIFIER(0) /* Don't care */
		    | SGPIO_SLICE_MUX_CFG_PARALLEL_MODE(3) /* Shift 1 byte(8bits) per clock. */
		    | SGPIO_SLICE_MUX_CFG_DATA_CAPTURE_MODE(0) /* Don't care */
		    | SGPIO_SLICE_MUX_CFG_INV_OUT_CLK(0) /* Normal clock. */
		    | SGPIO_SLICE_MUX_CFG_CLKGEN_MODE(0) /* Use internal clock from COUNTER */
		    | SGPIO_SLICE_MUX_CFG_CLK_CAPTURE_MODE(0) /* Don't care */
		    | SGPIO_SLICE_MUX_CFG_MATCH_MODE(0) /* Do not match data */
			;

		SGPIO_PRESET(slice_index) = config->clock_divider-1;	// Internal clock, determines sampling rate, derived from SGPIO_CLK
		SGPIO_COUNT(slice_index) = 0;		// Init to 0
		SGPIO_POS(slice_index) =
			  SGPIO_POS_POS_RESET(pos)
			| SGPIO_POS_POS(pos)
			;
		SGPIO_REG(slice_index) = 0x00000000;     // Primary output data register
		SGPIO_REG_SS(slice_index) = 0x00000000;  // Shadow output data register
		
		slice_enable_mask |= (1 << slice_index);
	}
	SGPIO_CTRL_ENABLE = slice_enable_mask;	
}

void sgpio_clock_out_configure(uint16_t clock_divider) {
	/* Use slice B as clock output
	 * This toggles SGPIO8 as aclock
	 */
	scu_pinmux(SCU_PINMUX_SGPIO8, SCU_GPIO_FAST | SCU_CONF_FUNCTION6);

	const uint_fast8_t slice_index = SGPIO_SLICE_B;
	SGPIO_OUT_MUX_CFG(8) =		// SGPIO8: Input: clock
	  SGPIO_OUT_MUX_CFG_P_OE_CFG(0) /* 0x0 gpio_oe (state set by GPIO_OEREG) */
	| SGPIO_OUT_MUX_CFG_P_OUT_CFG(0) /* 0x0 dout_doutm1 (1-bit mode) */ 
	;
	SGPIO_GPIO_OENREG |= (1L<<8);
	SGPIO_MUX_CFG(slice_index) =
	      SGPIO_MUX_CFG_CONCAT_ORDER(0)
	    | SGPIO_MUX_CFG_CONCAT_ENABLE(1)
	    | SGPIO_MUX_CFG_QUALIFIER_SLICE_MODE(0) /* Don't care */
	    | SGPIO_MUX_CFG_QUALIFIER_PIN_MODE(0) /* Don't care */
	    | SGPIO_MUX_CFG_QUALIFIER_MODE(0) /* Always enabled */
	    | SGPIO_MUX_CFG_CLK_SOURCE_SLICE_MODE(0) /* Select clock source slice D(0x0) */
	    | SGPIO_MUX_CFG_CLK_SOURCE_PIN_MODE(0) /* Don't care */
		| SGPIO_MUX_CFG_EXT_CLK_ENABLE(0) /* Internal clock signal (slice) */
		;
	SGPIO_SLICE_MUX_CFG(slice_index) =
	      SGPIO_SLICE_MUX_CFG_INV_QUALIFIER(0) /* Don't care */
	    | SGPIO_SLICE_MUX_CFG_PARALLEL_MODE(0) /* Shift 1 bit per clock. */
	    | SGPIO_SLICE_MUX_CFG_DATA_CAPTURE_MODE(0) /* Don't care */
	    | SGPIO_SLICE_MUX_CFG_INV_OUT_CLK(0) /* Normal clock. */
	    | SGPIO_SLICE_MUX_CFG_CLKGEN_MODE(0) /* Use internal clock from COUNTER */
	    | SGPIO_SLICE_MUX_CFG_CLK_CAPTURE_MODE(0) /* Don't care */
	    | SGPIO_SLICE_MUX_CFG_MATCH_MODE(0) /* Do not match data */
		;
	/* clock period is halved as we shift 1/0 bits out */
	SGPIO_PRESET(slice_index) = (clock_divider>>1)-1;	// Internal clock, determines sampling rate, derived from SGPIO_CLK
	SGPIO_COUNT(slice_index) = 0;		// Init to 0
	SGPIO_POS(slice_index) =
		  SGPIO_POS_POS_RESET(0x03)
		| SGPIO_POS_POS(0x03)
		;
	SGPIO_REG(slice_index) = 0xAAAAAAAA;     // Primary output data register
	SGPIO_REG_SS(slice_index) = 0xAAAAAAAA;  // Shadow output data register
	
	// Start SGPIO operation by enabling slice clocks.
	SGPIO_CTRL_ENABLE |= (1 << slice_index);
}
