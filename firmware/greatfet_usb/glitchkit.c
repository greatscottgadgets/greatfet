/*
 * This file is part of GreatFET
 *
 * GlitchKit common functionality
 */

#include <stdbool.h>
#include "glitchkit.h"

#include <greatfet_core.h>


// Start off with glitchkit functionality disabled.
volatile bool glitchkit_enabled = false;


/**
 * Enables GlitchKit functionality. This should be called by any GlitchKit
 * module before it sees used. This is idempotent, and can be called multiple times
 * without ill effect.
 */
void glitchkit_enable(void) {
    if (glitchkit_enabled)
      return;

    glitchkit_enabled = true;
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
    // TODO: implement
    led_toggle(LED4);
}

/**
 * Main loop service routine for GlitchKit.
 */
void service_glitchkit() {
    // TODO: deassert trigger after a period of time?
}
