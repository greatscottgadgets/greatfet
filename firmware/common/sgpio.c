/*
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2013 Benjamin Vernoux <titanmkd@gmail.com>
 *
 * This file is part of HackRF.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
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
	// scu_pinmux(SCU_PINMUX_SGPIO8, SCU_GPIO_FAST | SCU_CONF_FUNCTION6);
	// scu_pinmux(SCU_PINMUX_SGPIO9, SCU_GPIO_FAST | SCU_CONF_FUNCTION7);
	// scu_pinmux(SCU_PINMUX_SGPIO10, SCU_GPIO_FAST | SCU_CONF_FUNCTION6);
	// scu_pinmux(SCU_PINMUX_SGPIO11, SCU_GPIO_FAST | SCU_CONF_FUNCTION6);
	// scu_pinmux(SCU_PINMUX_SGPIO12, SCU_GPIO_FAST | SCU_CONF_FUNCTION0); /* GPIO0[13] */
	// scu_pinmux(SCU_PINMUX_SGPIO13, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);	/* GPIO5[12] */
	// scu_pinmux(SCU_PINMUX_SGPIO14, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);	/* GPIO5[13] */
	// scu_pinmux(SCU_PINMUX_SGPIO15, SCU_GPIO_FAST | SCU_CONF_FUNCTION4);	/* GPIO5[14] */
}

void sgpio_set_slice_mode(
	sgpio_config_t* const config,
	const bool multi_slice
) {
	config->slice_mode_multislice = multi_slice;
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

		SGPIO_PRESET(slice_index) = 0x003;	// Internal clock, determines sampling rate, derived from SGPIO_CLK
		SGPIO_COUNT(slice_index) = 0;		// Init to 0
		SGPIO_POS(slice_index) =
			  SGPIO_POS_POS_RESET(pos)
			| SGPIO_POS_POS(pos)
			;
		SGPIO_REG(slice_index) = 0x00000000;     // Primary output data register
		SGPIO_REG_SS(slice_index) = 0x00000000;  // Shadow output data register
		
		slice_enable_mask |= (1 << slice_index);
	}

	// Start SGPIO operation by enabling slice clocks.
	SGPIO_CTRL_ENABLE = slice_enable_mask;
}
