/*
 * This file is part of GreatFET
 *
 * GlitchKit common functionality
 */

#ifndef __GLITCHKIT_H__
#define __GLITCHKIT_H__

#include <stdbool.h>

/**
 * The event values here are _not fixed_; and they should not be used
 * externally. Instead, the USB API should resolve hard event names.
 *
 * This seems like a pain, but it allows us to replace this data structure
 * when we need to add more events.
 */
typedef enum {
	GLITCHKIT_SIMPLE_COUNT_REACHED = 0x01,
} glitchkit_event_t;



// The interrupt priority to use for GlitchKit interrupts. Usually, we want this
// to be the highest possible in order to ensure our timing is consistent. (We
// don't want the variation of interrupt skid.)
#define GLITCHKIT_INTERRUPT_PRIORITY 0

// True iff GlitchKit modules are enabled.
extern volatile bool glitchkit_enabled;


/**
 * Enables GlitchKit functionality. This should be called by any GlitchKit
 * module before it sees used. This is idempotent, and can be called multiple times
 * without ill effect.
 */
void glitchkit_enable(void);


/**
 * Disables GlitchKit functionality, but does not disable any GlitchKit modueles.
 * Disable them before calling this function.
 */
void glitchkit_disable(void);


/**
 * Function that causes the GlitchKit trigger signal to rise, triggering the
 * ChipWhisperer to e.g. induce a glitch.
 */
void glitchkit_trigger();


/**
 * Main loop service routine for GlitchKit.
 */
void service_glitchkit();


/**
 * Requests that GlitchKit issue a trigger to the ChipWhisperer when
 * the given event happens.
 */
void glitchkit_enable_trigger_on(glitchkit_event_t event);


/**
 * Requests that GlitchKit no longer issue a trigger to the ChipWhisperer
 * when the given event happens.
 */
void glitchkit_disable_trigger_on(glitchkit_event_t event);


/**
 * Notify GlitchKit of a GlitchKit-observable event.
 *
 * @param event The type of event observed.
 */
void glitchkit_notify_event(glitchkit_event_t event);


#endif

