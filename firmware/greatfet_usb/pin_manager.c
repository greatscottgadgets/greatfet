/*
 * This file is part of GreatFET
 */

#include <errno.h>

#include <debug.h>
#include <sync.h>

#include <drivers/comms.h>
#include <drivers/memory/allocator.h>


// Number of pin groups the device has.
// For GreatFET, this is currently 16.
#define NUM_PIN_GROUPS 16

/**
 * Structure representing a pin reservation.
 */
struct pin_reservation {

	/* The pin's group. Mostly stored for reference / debug. */
	uint8_t pin_group;

	/** The pin number in the provided group. */
	uint8_t pin_number;

	/** The class number for the class that owns the given pin. */
	uint32_t owning_class;

	/** Creates a linked list of pin reservations, linking together
	 *  reservations in a given group. */
	struct pin_reservation *next;
};

/**
 * Linked list heads for each of our pin groups.
 */
static struct pin_reservation *pin_group_heads[NUM_PIN_GROUPS];
static mutex_t pin_reservation_lock;


/**
 * Finds the pin reservation for the given group/pin number;
 * or returns NULL if none can be located. Assumes the reservation lock is held.
 *
 * @param group The pin group for the pin to be located.
 * @param pin The pin number for the pin to be located.
 * @param predecessor If non-null, this out argument will recieve the 
 *		predecessor to the located node.
 */
static struct pin_reservation *_find_pin_reservation(uint8_t group, uint8_t pin,
		struct pin_reservation **out_predecessor)
{
	// Search the list for the given pin group.
	struct pin_reservation *reservation, *predecessor = NULL;

	pr_trace("manager: searching for %d:%d!\n", group, pin);

	// Search until we run out of entries to check, or find our reservation.
	for (reservation = pin_group_heads[group]; reservation; reservation = reservation->next) {
		pr_trace("manager: checking %p (%d:%d)!\n", reservation , reservation->pin_group, reservation->pin_number);
		if (reservation->pin_number == pin) {
			pr_trace("manager: found in %p (%d:%d), owner is %d!\n", reservation,
					reservation->pin_group, reservation->pin_number, reservation->owning_class);
			break;	
		}
		predecessor = reservation;
	}

	// If we've been passed an pointer with which to accept a predecessor,
	// populate it with our current knowledge of the predecessor.
	if (out_predecessor) {
		*out_predecessor = predecessor;
	}

	return reservation;
}




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
uint32_t pin_get_owning_class(uint8_t group, uint8_t pin)
{
	struct pin_reservation *reservation;
	uint32_t owning_class;

	libgreat_mutex_lock(&pin_reservation_lock);

	// Attempt to find the owner of the given pin,
	// defaulting to 0 if we can't find one.
	reservation = _find_pin_reservation(group, pin, NULL);
	owning_class = reservation ? reservation->owning_class : 0;
	
	libgreat_mutex_unlock(&pin_reservation_lock);
	return owning_class;
}



/**
 *  Reserves the provided pin for use by a class (and its affiliates).
 *  This version assumes the local pin reservation lock is held.
 */
static int _pin_reserve_for_class(uint8_t group, uint8_t pin, uint32_t owning_class)
{
	// Start off by checking for an existing reservation.
	struct pin_reservation *reservation = _find_pin_reservation(group, pin, NULL);

	// If this pin is already reserved, we don't want to allow re-reservation.
	if (reservation) {
		const char *new_class_name;
		const char *old_class_name;

		pr_trace("manager: reserving pin for an existing class!\n");
		
		// One exception: if the pin is reserved to us, consider this done.
		if (reservation->owning_class == owning_class) {
			pr_trace("manager: ignoring re-reservation for same class!\n");
			return 0;	
		}

		// Get usable names for the classes, for  our debug output.
		new_class_name = comms_get_class_name(owning_class, "-unknown-");
		old_class_name = comms_get_class_name(reservation->owning_class, "-unknown-");

		// Fail, preventing re-assignment.
		pr_warning("WARNING: not assigning pin P%X[%d] to '%s' (%d); already owned by '%s' (%d)",
				group, pin, new_class_name, owning_class, old_class_name, reservation->owning_class);
		return EBUSY;
	}

	pr_trace("manager: creating new reservation for %d:%d!\n", group, pin);

	// Otherwise, we'll need to create a new reservation and 
	// add it to our list.
	reservation = malloc(sizeof(*reservation));
	if (!reservation) {
		pr_error("pin_manager: could not issue allocation; out of mem!\n");
		return ENOMEM;
	}

	// Build our relevant pin information to the reservation.
	reservation->pin_group = group;
	reservation->pin_number = pin;
	reservation->owning_class = owning_class;

	// Insert the reservation into the relevant list,
	// and return, indicating sucess.
	reservation->next = pin_group_heads[group];
	pin_group_heads[group] = reservation;
	return 0;
}


/**
 *  Reserves the provided pin for use by a class (and its affiliates).
 */
int pin_reserve_for_class(uint8_t group, uint8_t pin, uint32_t owning_class)
{
	int rc;

	// Delegate to the unlocked-version of our function.
	libgreat_mutex_lock(&pin_reservation_lock);	
	rc = _pin_reserve_for_class(group, pin, owning_class);
	libgreat_mutex_unlock(&pin_reservation_lock);	

	return rc;
}

/**
 * Returns true iff the given pin is reserved for the given class,
 * attempting reservation if possible.
 */
bool pin_ensure_reservation(uint8_t group, uint8_t pin, uint32_t active_class)
{
    return (pin_reserve_for_class(group, pin, active_class) == 0);
}


/**
 * Releases a reserved pin, restoring its ability to be reserved/used by
 * other classes. Assumes the pin reservation lock is held.
 */
static int _pin_release_reservation(uint8_t group, uint8_t pin)
{
	// Find any existing reservations that exist.
	struct pin_reservation *reservation, *predecessor;
	reservation = _find_pin_reservation(group, pin, &predecessor);

	// If we don't have a reservation, fail out.
	if (!reservation) {
		pr_warning("Tried to release a reservated for an unreserved pin!");
		return EINVAL;	
	}

	// Special case: if we have a reservation, but no predecessor,
	// this reservation is the head of our list.
	if (!predecessor) {
		
		// Sanity check ourselves.
		if (pin_group_heads[group] != reservation) {
			pr_error("pin_manager: internal consistency error!\n");
			pr_error("found a node with no predecessor that's not a head?\n");
			return EINVAL;
		}

		// Remove our head link from the list.
		pin_group_heads[group] = reservation->next;
	} 
	// Otherwise, we have a normal, non-head link.
	// Remove it the simple way.
	else {
		predecessor->next = reservation->next;
	}

	// Finally, free the reservation.
	free(reservation);
	return 0;
}



/**
 * Releases a reserved pin, restoring its ability to be reserved/used by
 * other classes.
 */
int pin_release_reservation(uint8_t group, uint8_t pin)
{
	int rc;

	// Delegate to the unlocked-version of our function.
	libgreat_mutex_lock(&pin_reservation_lock);	
	rc = _pin_release_reservation(group, pin);
	libgreat_mutex_unlock(&pin_reservation_lock);	

	return rc;
}
