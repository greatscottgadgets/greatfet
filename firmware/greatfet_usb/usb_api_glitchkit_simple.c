/*
 * This file is part of GreatFET
 *
 * GlichKit: simple condition counter
 *
 * Feel free to expand me to support arbitrarily complex conditions, but be
 * aware that runtime in the ISR shouldn't grow too long or we'll miss conditions.
 */

#include "usb_api_glitchkit_simple.h"
#include "glitchkit.h"

#include <greatfet_core.h>
#include <string.h>

#include <drivers/usb/lpc43xx/usb.h>
#include <drivers/usb/lpc43xx/usb_queue.h>
#include "usb_endpoint.h"

#include "gpio_lpc.h"
#include "gpio_int.h"

static void _disable_module_interrupts(void);

void glitchkit_gpio_isr(void);

// TODO: Support a wider counter, if it ever becomes necessary.

/* The actual count of GltichKit events that have occurred. */
static volatile uint32_t event_count = 0;

/* The count necesssary to cause a GlitchKit trigger. */
static volatile uint32_t event_count_target = 0;

/* Struct describing a pin-state condition necessary for a trigger to be counted. */
struct pin_condition {

  /**
   * Expected pin behavior. If this is a LEVEL_SENSITIVE mode, the relevant
   * pin will be required to have the provided value on each interrupt to be
   * counted as an event. If this is a EDGE_SENSITIVE mode, the relevant pin
   * will trigger an interrupt any time the given edge passes.
   *
   * In other words, LEVEL sensitive events are _AND_'d, and edges are _OR'd_.
   * If the GreatFET is executing synchronously to the target's clock, this
   * allows you to build some very simple logic. :)
   *
   * Values should match the values in an gpio_interrupt_sensitivity_t (gpio_int.h).t
   */
  uint8_t mode;

  /** The port and pin number for the given pin. This pin should already have
   *  been configured as an input. */
  uint8_t port_number;
  uint8_t pin_number;

} __attribute__((packed));
typedef struct pin_condition pin_condition_t;

/* All of the pin conditions necessary for a trigger event to be counted. */
volatile bool active_condition_level[MAX_PIN_CONDITIONS];
volatile struct gpio_t active_condition_gpio[MAX_PIN_CONDITIONS];
volatile uint8_t active_condition_count = 0;

/* Count how many interrupts we're using. For now, we assume we get _all_
 * of the GPIO interrupts. */
uint8_t active_interrupt_count = 0;
volatile uint8_t active_interrupt_mask = 0;

/**
 * Adds a level-sensitive condition to the list of active_conditions.
 */
void _gpio_condition_add(pin_condition_t* condition){

    // Build a GPIO object for the relevant GPIO.
    struct gpio_t gpio = GPIO(condition->port_number, condition->pin_number);

    // Determine the level associated with the given condition, as a boolean where true == high.
    active_condition_level[active_condition_count] = (condition->mode == LEVEL_SENSITIVE_HIGH);

    // Copy the condition into our list of active conditions, along with its associated GPIO.
    memcpy(&active_condition_gpio[active_condition_count], &gpio, sizeof(gpio));
    ++active_condition_count;
}


/**
 * Vendor request that sets up (and enables) a GlitchKit trigger.
 * Components of this request include:
 *    'value' value: high byte of the 32-bit counter
 *    'index' value: low byte of the 32-bit counter
 *    data stage: a collection of pin_conditions to be applied; see the
 *        struct definition above
 *
 *  TODO: rename, this no longer enables triggering but just events
 *		(which can be used for synchronization)
 */
usb_request_status_t usb_vendor_request_glitchkit_simple_enable_trigger(
    usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
  // Allow for a comand that provides twice the number of MAX_CONDITIONS,
  // as we want to have half of the conditions allowed to be edge conditions,
  // which we'll set up ISRs for-- and half to be level conditions, which we'll
  // splork into the active_condtions array.
  static volatile pin_condition_t conditions_to_parse[MAX_PIN_CONDITIONS * 2];

  if (stage == USB_TRANSFER_STAGE_SETUP) {

    // Ensure our packet will fit conveniently in our buffer.
    if(endpoint->setup.length > sizeof(conditions_to_parse)) {
      return USB_REQUEST_STATUS_STALL;
    }

    // Ensure we _have_ a body to read and handle.
    if(!endpoint->setup.length) {
      return USB_REQUEST_STATUS_STALL;
    }

    // Read in the configuration command from the host.
    usb_transfer_schedule_block(endpoint->out, conditions_to_parse, endpoint->setup.length, NULL, NULL);

  } else if(stage == USB_TRANSFER_STAGE_DATA) {

    // Ack early.
    usb_transfer_schedule_ack(endpoint->in);

    // Figure out how many conditions we need to parse.
    uint8_t condition_count = endpoint->setup.length / sizeof(pin_condition_t);

    // Ensure GlitchKit functionality is enabled.
    glitchkit_enable();

    // Extract our target value from the setup header.
    uint32_t count_high = endpoint->setup.value;
    uint32_t count_low  = endpoint->setup.index;
    event_count_target = (count_high << 16)  | count_low;

    // Reset our event counter and the number of active conditions and interrupts.
    event_count = 0;
    active_condition_count = 0;
    active_interrupt_count = 0;
    active_interrupt_mask  = 0;

    // Ensure we don't have any interrupts executing while the request is being
    // serviced (and we're configuring interrupts).
    _disable_module_interrupts();

    // Set up our conditions...
    for (int i = 0; i < condition_count; ++i) {
      pin_condition_t *condition = &conditions_to_parse[i];

      // If this a _level_ sensitive condition, add it to the condition list.
      // These are evaluated when _any_ of the edge-sensitive conditions occur.
      if((condition->mode == LEVEL_SENSITIVE_LOW)
            || (condition->mode == LEVEL_SENSITIVE_HIGH)) {

          // Ignore any conditions beyond the eigth.
          if(active_condition_count >= MAX_PIN_CONDITIONS) {
              continue;
          }

          // Add this condition to our list of active conditions.
          _gpio_condition_add(condition);
      } 
      // Otherwise, we have an _edge_ sensitive condition. Use this to set up an interrupt.
      // Note we don't activate the interrupt here, as the condition list is not yet complete
      // and we don't want to start counting. :)
      else {
          // Ignore any conditions beyond the eigth.
          if(active_interrupt_count >= GPIO_NUM_INTERRUPTS) {
              continue;
          }

          gpio_interrupt_configure(active_interrupt_count + 1, condition->port_number,
              condition->pin_number, condition->mode, glitchkit_gpio_isr,
              GLITCHKIT_INTERRUPT_PRIORITY);

          // Mark the current interrupt as used.
          active_interrupt_mask |= (1 << active_interrupt_count + 1);
          ++active_interrupt_count;
      }
    }

    // Finally, once all of our conditions are set up, enable their IRQs.
    for (int i = 0; i < active_interrupt_count; ++i) {
        gpio_interrupt_enable(i + 1);
    }

  }
  return USB_REQUEST_STATUS_OK;
}

/**
 * Disables all interrupts associated with this module.
 * Can be used e.g. when a trigger occurs and we don't want to continuously trigger.
 */
static void _disable_module_interrupts(void) {

  for(int i = 0; i < active_interrupt_count; ++i) {
    gpio_interrupt_disable(i);
  }
}


/**
 * Handle when a countable event occurs.
 */
void handle_glitchkit_event(void) {
  ++event_count;

  // If we've reached our count target, notify GlitchKit.
  if(event_count == event_count_target) {
    glitchkit_notify_event(GLITCHKIT_SIMPLE_COUNT_REACHED);
    _disable_module_interrupts();
  }
}

/**
 * Returns true when a all level-sensitive criteria are met.
 *
 * NOTE: This is on critical path! We ideally:
 *    - Want to take the least time possible. (This can _definitely_ be
 *      improved. This is an inoptimal first stab.)
 *    - Want to ensure that runtime is always equal across runs of this for a
 *      given set of criterias. We keep this constant time even across events
 *      that don't trigger, as this helps to keep timing consistent if we have
 *      another instance of this IRQ that occurs before our service routine
 *      completes.
 *
 */
static bool _event_criteria_met(void) {

    // TODO: Verify this has the timing properties we want.
    volatile bool match_found = true;

    // Iterate over our active conditions.
    for(int i = 0; i < active_condition_count; ++ i) {
        bool pin_value = !!gpio_read(&active_condition_gpio[i]);

        // If this isn't a match, mark this as a conditon that would fail out,
        // but don't break, to maintain our constant timing.
        if(pin_value != active_condition_level[i])
            match_found = false;
    }

    return match_found;
}


/**
 * Interrupt handler for a GlitchKit GPIO event.
 */
void glitchkit_gpio_isr(void) {

  // If this interrupt is one of the GlitchKit interrupts, handle it.
  if(GPIO_PIN_INTERRUPT_IST & active_interrupt_mask) {

      // Mark all of our interrupts as serviced.
      // An interrupt can't be more than one of these at once,
      // so it's safe to affect all three.
      GPIO_PIN_INTERRUPT_IST  |= active_interrupt_mask;
      GPIO_PIN_INTERRUPT_RISE |= active_interrupt_mask;
      GPIO_PIN_INTERRUPT_FALL |= active_interrupt_mask;

      // If this event has met our criteria, count it.
      if(!active_condition_count || _event_criteria_met()) {
        handle_glitchkit_event();
      }
  }
}
