/*
 * This file is part of GreatFET
 *
 * Code for ULPI interfacing via SGPIO.
 */

#ifndef __ULPI_H__
#define __ULPI_H__

#include <toolchain.h>


/**
 * Configuration.
 */


// If you have a v0.2 Rhododendron (or earlier) and haven't made the bodges I have
// (looking at you, @tannewt), you'll probably want to undefine these -- I've bodged in
// some automated test connections, and will probably add them to the design in r0.3. ~KT
//#define RHODODENDRON_SUPPORTS_VOLTAGE_SANITY_CHECKING
//#define RHODODENDRON_SUPPORTS_CLOCK_SANITY_CHECKING

// If this Rhododendron has the its ULPI CLKOUT tied to 3v3, we can provide it with a 60MHz
// reference clock of our own. Undefine the line below to switch the clock frequency to
// that reference clock.
#define RHODODENDRON_USE_USB1_CLK_AS_ULPI_CLOCK


/**
 * Speed constants for USB analysis.
 */
typedef enum {
	USB_CAPTURE_SPEED_HIGH = 0x00,
	USB_CAPTURE_SPEED_FULL = 0x01,
	USB_CAPTURE_SPEED_LOW  = 0x10
} usb_capture_speed_t;



/**
 * Register address constants for the registers we'll commonly need to use in ULPI PHYs.
 */
enum {

	// USB function control register (RW)
	FUNCTION_CONTROL_REGISTER = 0x04,

	// USB IO and power management register (RW)
	USB344X_USB_IO_REGISTER   = 0x39,

};


/**
 * ULPI command prefix masks.
 */
enum {

	ULPI_COMMAND_IDLE                = 0b00000000,

	// Command prefixes.
	ULPI_COMMAND_REGISTER_READ_MASK  = 0b11000000,
	ULPI_COMMAND_REGISTER_WRITE_MASK = 0b10000000,
};


/**
 * GPIO settings for each of the Rhododendron LEDs.
 */
typedef enum {
	LED_VBUS      = 0,
	LED_TRIGGERED = 1,
	LED_STATUS    = 2, // TODO: rename:

	LED_ON  = 0,
	LED_OFF = 1,
} rhododendron_led_t;


/**
 * Initializes a connected Rhododendron board, preparing things for analysis.
 *
 * @return 0 on success, or an error code if the board could not be brought up
 */
int initialize_rhododendron(void);


/**
 * Function that places the GreatFET into ULPI register access mode.
 *
 * Other ULPI operations (e.g. analysis) cannot be performed while in register access mode--
 * you'll need to call ulpi_register_access_stop() to disengage register access mode before
 * running any other code.
 *
 * @returns 0 on success, or an error code on failure
 */
int ulpi_register_access_start(void);


/**
 * Function that ends register access mode, freeing the SGPIO->ULPI bridge for other operations.
 */
void ulpi_register_access_stop(void);


/**
 * Performs a write to a non-extended ('immediate') ULPI register.
 *
 * @returns 0 on success, or an error code on failure.
 */
int ulpi_register_write(uint8_t address, uint8_t value);


/**
 * Turns on the provided Rhododendron LED.
 */
void rhododendron_turn_on_led(rhododendron_led_t led);


/**
 * Turns off the provided Rhododendron LED.
 */
void rhododendron_turn_off_led(rhododendron_led_t led);


/**
 * Toggles the provided Rhododendron LED.
 */
void rhododendron_toggle_led(rhododendron_led_t led);


/**
 * Starts a Rhododendron capture of high-speed USB data.
 */
int rhododendron_start_capture(void);


/**
 * Terminates a Rhododendron capture.
 */
void rhododendron_stop_capture(void);


/**
 * Rhododenron background "thread".
 */
void service_rhododendron(void);


#endif
