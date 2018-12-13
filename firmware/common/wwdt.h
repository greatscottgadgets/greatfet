/** @defgroup wwdt_defines Windowed Watchdog Timer

@brief <b>Defined Constants and Types for the LPC43xx Windowed Watchdog
Timer</b>

@ingroup LPC43xx_defines

@version 1.0.0

@author @htmlonly &copy; @endhtmlonly 2012 Michael Ossmann <mike@ossmann.com>

@date 10 March 2013

LGPL License Terms @ref lgpl_license
 */
/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2012 Michael Ossmann <mike@ossmann.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LPC43XX_WWDT_H
#define LPC43XX_WWDT_H

/**@{*/

#include <libopencm3/cm3/common.h>
#include <libopencm3/lpc43xx/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Windowed Watchdog Timer (WWDT) registers ---------------------------- */

/* Watchdog mode register */
#define WWDT_MOD                        MMIO32(WWDT_BASE + 0x000)

/* Watchdog timer constant register */
#define WWDT_TC                         MMIO32(WWDT_BASE + 0x004)

/* Watchdog feed sequence register */
#define WWDT_FEED                       MMIO32(WWDT_BASE + 0x008)
#define WWDT_FEED_SEQUENCE WWDT_FEED = 0xAA; WWDT_FEED = 0x55

/* Watchdog timer value register */
#define WWDT_TV                         MMIO32(WWDT_BASE + 0x00C)

/* Watchdog warning interrupt register */
#define WWDT_WARNINT                    MMIO32(WWDT_BASE + 0x014)

/* Watchdog timer window register */
#define WWDT_WINDOW                     MMIO32(WWDT_BASE + 0x018)

/* --- WWDT_MOD values ----------------------------------------------------- */

/* WDEN: Watchdog enable bit */
#define WWDT_MOD_WDEN_SHIFT (0)
#define WWDT_MOD_WDEN (1 << WWDT_MOD_WDEN_SHIFT)

/* WDRESET: Watchdog reset enable bit */
#define WWDT_MOD_WDRESET_SHIFT (1)	
#define WWDT_MOD_WDRESET (1 << WWDT_MOD_WDRESET_SHIFT)

/* WDTOF: Watchdog time-out flag */
#define WWDT_MOD_WDTOF_SHIFT (2)
#define WWDT_MOD_WDTOF (1 << WWDT_MOD_WDTOF_SHIFT)

/* WDINT: Watchdog interrupt flag */
#define WWDT_MOD_WDINT_SHIFT (3)
#define WWDT_MOD_WDINT (1 << WWDT_MOD_WDINT_SHIFT)

/* WDPROTECT: Watchdog update mode */
#define WWDT_MOD_WDPROTECT_SHIFT (4)
#define WWDT_MOD_WDPROTECT (1 << WWDT_MOD_WDPROTECT_SHIFT)

/* --- WWDT_TC values ------------------------------------------------------ */

/* WDTC: Watchdog time-out value */
#define WWDT_TC_WDTC_SHIFT (0)
#define WWDT_TC_WDTC_MASK (0xffffff << WWDT_TC_WDTC_SHIFT)
#define WWDT_TC_WDTC(x) ((x) << WWDT_TC_WDTC_SHIFT)

/* --- WWDT_FEED values ---------------------------------------------------- */

/* Feed: Feed value should be 0xAA followed by 0x55 */
#define WWDT_FEED_FEED_SHIFT (0)
#define WWDT_FEED_FEED_MASK (0xff << WWDT_FEED_FEED_SHIFT)
#define WWDT_FEED_FEED(x) ((x) << WWDT_FEED_FEED_SHIFT)

/* --- WWDT_TV values ------------------------------------------------------ */

/* Count: Counter timer value */
#define WWDT_TV_COUNT_SHIFT (0)
#define WWDT_TV_COUNT_MASK (0xffffff << WWDT_TV_COUNT_SHIFT)
#define WWDT_TV_COUNT(x) ((x) << WWDT_TV_COUNT_SHIFT)

/* --- WWDT_WARNINT values ------------------------------------------------- */

/* WDWARNINT: Watchdog warning interrupt compare value */
#define WWDT_WARNINT_WDWARNINT_SHIFT (0)
#define WWDT_WARNINT_WDWARNINT_MASK (0x3ff << WWDT_WARNINT_WDWARNINT_SHIFT)
#define WWDT_WARNINT_WDWARNINT(x) ((x) << WWDT_WARNINT_WDWARNINT_SHIFT)

/* --- WWDT_WINDOW values -------------------------------------------------- */
#define WWDT_WINDOW_WDWINDOW_SHIFT (0)
#define WWDT_WINDOW_WDWINDOW_MASK (0xffffff << WWDT_WINDOW_WDWINDOW_SHIFT)
#define WWDT_WINDOW_WDWINDOW(x) ((x) << WWDT_WINDOW_WDWINDOW_SHIFT)

/**@}*/

/* Reset LPC4330 in timeout*4 clock cycles (min 256, max 2^24) */
void wwdt_reset(uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif
