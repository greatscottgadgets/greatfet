/*
 * This file is part of libgreat
 *
 * Synchronization primitives for ARMv7-M devices
 */

/* TODO: move me to a more general location? */
#include <sync.h>

// Calls to our raw assembly mutex code, from sync.S
void _lock_mutex(uint32_t *mutex);
void _unlock_mutex(uint32_t *mutex);

/**
 *  Initializes a new mutex.
 */
void libgreat_mutex_init(mutex_t *mutex)
{
	*mutex = 0;
}


/**
 * Ensures ownership of the provided mutex.
 * Blocks until the mutex can be locked.
 */
void libgreat_mutex_lock(mutex_t *mutex)
{
	_lock_mutex(mutex);	
}


/**
 * Releases ownership of the provided mutex.
 */
void libgreat_mutex_unlock(mutex_t *mutex)
{
	_unlock_mutex(mutex);	
}
