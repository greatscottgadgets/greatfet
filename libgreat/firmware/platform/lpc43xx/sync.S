/*
 * This file is part of libgreat
 *
 * Synchronization primitives for ARMv7-M devices
 * Provided by ARM as freely-usable reference code.
 */

.equ locked,1
.equ unlocked,0

.global _lock_mutex
_lock_mutex:
    LDR     r1, =locked
1:  LDREX   r2, [r0]
    CMP     r2, r1        // Test if mutex is locked or unlocked
    BEQ     2f            // If locked - wait for it to be released, from 2
    STREX   r2, r1, [r0]  // Not locked, attempt to lock it
    CMP     r2, #1        // Check if Store-Exclusive failed
    BEQ     1b            // Failed - retry from 1
    // Lock acquired
    DMB                   // Required before accessing protected resource
    BX      lr

2:  // spin
    B       1b            // Retry from 1


.global _unlock_mutex
_unlock_mutex:
    LDR     r1, =unlocked
    DMB                   // Required before releasing protected resource
    STR     r1, [r0]      // Unlock mutex
    BX      lr
