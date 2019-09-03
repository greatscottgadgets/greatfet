/*
 * This file is part of GreatFET
 */

#ifndef __FAULT_HANDLER__
#define __FAULT_HANDLER__

#include <stdint.h>

#define SCB_MMFSR_IACCVIOL (1 << 0)
#define SCB_MMFSR_DACCVIOL (1 << 1)
#define SCB_MMFSR_MUNSTKERR (1 << 3)
#define SCB_MMFSR_MSTKERR (1 << 4)
#define SCB_MMFSR_MLSPERR (1 << 5)
#define SCB_MMFSR_MMARVALID (1 << 7)
#define SCB_MMFSR_FAULT_MASK ( \
		SCB_MMFSR_IACCVIOL | \
		SCB_MMFSR_DACCVIOL | \
		SCB_MMFSR_MUNSTKERR | \
		SCB_MMFSR_MSTKERR | \
		SCB_MMFSR_MLSPERR \
	)

#define SCB_BFSR_BFARVALID (1 << 7)
#define SCB_BFSR_LSPERR (1 << 5)
#define SCB_BFSR_STKERR (1 << 4)
#define SCB_BFSR_UNSTKERR (1 << 3)
#define SCB_BFSR_IMPRECISERR (1 << 2)
#define SCB_BFSR_PRECISERR (1 << 1)
#define SCB_BFSR_IBUSERR (1 << 0)

#define SCB_BFSR_FAULT_MASK ( \
		SCB_BFSR_LSPERR | \
		SCB_BFSR_STKERR | \
		SCB_BFSR_UNSTKERR | \
		SCB_BFSR_IBUSERR \
	)


#endif//__FAULT_HANDLER__
