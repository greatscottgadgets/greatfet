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
	GLITCHKIT_SIMPLE_COUNT_REACHED = 0x001,
  
  // TODO: Consolidate me
	GLITCHKIT_USBHOST_START_TD     = 0x002,
	GLITCHKIT_USBHOST_START_SETUP  = 0x004,
	GLITCHKIT_USBHOST_START_IN     = 0x008,
	GLITCHKIT_USBHOST_START_OUT    = 0x010,

	GLITCHKIT_USBHOST_FINISH_TD    = 0x020,
	GLITCHKIT_USBHOST_FINISH_SETUP = 0x040,
	GLITCHKIT_USBHOST_FINISH_OUT   = 0x080,
	GLITCHKIT_USBHOST_FINISH_IN    = 0x100,

	GLITCHKIT_USBDEVICE_FINISH_TD  = 0x200

	// GLITCHKIT_USBHOST_START_PACKET = 0x400, // TODO: manual monitoring with M0?
	// GLITCHKIT_USBHOST_END_PACKET   = 0x800, // TODO: manual monitoring with M0?


} glitchkit_event_t;


// The interrupt priority to use for GlitchKit interrupts. Usually, we want this
// to be the highest possible in order to ensure our timing is consistent. (We
// don't want the variation of interrupt skid.)
#define GLITCHKIT_INTERRUPT_PRIORITY 0


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


/**
 * Adds a GlitchKit event to a list of events that have occurred,
 * but does not issue any relevant tirggers until glitchkit_apply_defferred_events()
 * is called.
 *
 * This is good if events occur outside of a context with reproducible timing
 * (e.g. there's some skew in when a given event happen). The
 * glitchkit_apply_defferred_events method can be called from the next 
 * synchronized context.
 *
 * @param event The type of event(s) observed. An OR'ing of multiple
 *     events is acceptable, as well.
 */
void glitchkit_notify_event_deferred(glitchkit_event_t event);


/**
 * Issues a trigger if any of the events in the allowed_events mask have
 * been previously deferred. See the documentation if glitchkit_notify_event_deferred.
 *
 * @param allowed_events The bitwise OR of all events of interest.
 */
void glitchkit_apply_deferred_events(glitchkit_event_t allowed_events);


/**
 * Specifies to use a given type of event for synchronization, but does not
 * block. This will be used on a later call to glitchkit_wait_for_events().
 *
 * @param event_to_sync_on  The event or events to wait on. Multiple events can be OR'd.
 */
void glitchkit_use_event_for_synchronization(glitchkit_event_t event_to_sync_on);


/**
 * Waits for an event or events that should occur in an interrupt context.
 *
 * @param event_to_wait_for The event or events to wait for. If zero,
 *		this method will use the values previously set by used_event_for_synchronization.
 */
void glitchkit_wait_for_events(glitchkit_event_t event_to_wait_for);


#endif

