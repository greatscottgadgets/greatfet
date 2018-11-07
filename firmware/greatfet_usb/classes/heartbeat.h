/*
 * This file is part of GreatFET
 */

#ifndef __CLASS_HEARTBEAT_H__
#define __CLASS_HEARTBEAT_H__

/**
 * Prepares the system to use heartbeat mode.
 */
void heartbeat_init(void);

/**
 * Performs a single unit of heartbeat mode's work.
 * This should be called repeatedly from the main loop.
 */
void service_heartbeat(void);

#endif
