/*
 * This file is part of GreatFET
 *
 * General pin ownership manager.
 */

#ifndef __PIN_MANAGER_H__
#define __PIN_MANAGER_H__

/**
 * Returns the number of the owning class, or 0 (core) if the pin
 * isn't reserved by any given class.
 *
 * @param group The pin group, as represented in the LPC SCU.
 * @param pin The pin number, as represented in the LPC SCU. 
 *
 * @return The pin number of the owning class, or 0/core if the pin
 * isn't reserved by any given class.
 */
uint32_t pin_get_owning_class(uint8_t group, uint8_t pin);


/**
 * Reserves the provided pin for use by a class (and its affiliates).
 *
 * @param group The pin group, as represented in the LPC SCU.
 * @param pin The pin number, as represented in the LPC SCU. 
 * @param owning_class The class that should own the given pin.
 *
 * @return 0 on success, or an error code on failure.
 */
int pin_reserve_for_class(uint8_t group, uint8_t pin, uint32_t owning_class);


/**
 * Returns true iff the given pin is reserved for the given class,
 * attempting reservation if possible.
 */
bool pin_ensure_reservation(uint8_t group, uint8_t pin, uint32_t active_class);


/**
 * Releases a reserved pin, restoring its ability to be reserved/used by
 * other classes.
 *
 * @param group The pin group, as represented in the LPC SCU.
 * @param pin The pin number, as represented in the LPC SCU. 
 *
 * @return 0 on success, or an error code on failure.
 */
int pin_release_reservation(uint8_t group, uint8_t pin);

#endif
