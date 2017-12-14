/*
 * This file is part of GreatFET
 *
 * GlitchKit common functionality
 */

#include <stdbool.h>
#include "glitchkit.h"

#include <greatfet_core.h>
#include <gpio_lpc.h>


// Start off with glitchkit functionality disabled.
volatile bool glitchkit_enabled = false;

// Start off with no trigger set up.
volatile struct gpio_t trigger_gpio = GPIO(5, 14);

// FIXME: temporary, replace me with a timer!
volatile bool glitchkit_triggered = false;

/**
 * Enables GlitchKit functionality. This should be called by any GlitchKit
 * module before it sees used. This is idempotent, and can be called multiple times
 * without ill effect.
 */
void glitchkit_enable(void) {
    if (glitchkit_enabled)
      return;

    glitchkit_enabled = true;

    // Set up our default trigger GPIO, if none has been set up already.
    // TODO: Allow this to be customizable, instead of fixed to Indigo?
    gpio_input(&trigger_gpio);
}


/**
 * Disables GlitchKit functionality, but does not disable any GlitchKit modueles.
 * Disable them before calling this function.
 */
void glitchkit_disable(void) {
    if (!glitchkit_enabled)
      return;

    glitchkit_enabled = false;
}


/**
 * Function that causes the GlitchKit trigger signal to rise, triggering the
 * ChipWhisperer to e.g. induce a glitch.
 */
void glitchkit_trigger() {

    // FIXME: Remove when we're no longer debugging.
    led_toggle(LED4);

    // Set the GPIO pin high, immediately...
    gpio_write(&trigger_gpio, true);

    //... and then schedule it to turn off later.
    // FIXME: This should really be on a timer.
    glitchkit_triggered = true;
}

/**
 * Main loop service routine for GlitchKit.
 */
void service_glitchkit() {

    // Temporary implementation: hold the trigger high for >1ms.
    // FIXME: Replace me with a timer!
    if(glitchkit_triggered) {

        // Wait 1 ms...
        delay(1000);

        // ... and then de-assert the trigger.
        gpio_write(&trigger_gpio, true);
        glitchkit_triggered = false;
    }
}
