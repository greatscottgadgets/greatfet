/*
 * This file is part of libgreat
 *
 * Generic synchronization primitives
 */


#ifndef __LIBGREAT_SYNC_H__
#define __LIBGREAT_SYNC_H__

#include <platform_sync.h>

// TODO: create normal mutex_lock / mutex_unlock names
// when we don't have libopencm3 to conflict with
void libgreat_mutex_init(mutex_t *mutex);
void libgreat_mutex_lock(mutex_t *mutex);
void libgreat_mutex_unlock(mutex_t *mutex);



#endif // __LIBGREAT_SYNC_H__
