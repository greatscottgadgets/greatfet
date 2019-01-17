/*
 * This file is part of GreatFET
 */

#ifndef __FAULT_HANDLER__
#define __FAULT_HANDLER__

#include <stdint.h>
#include <libopencm3/cm3/memorymap.h>

// TODO: Move all this to a Cortex-M(?) include file, since these
// structures are supposedly the same between processors (to an
// undetermined extent).
typedef struct armv7m_scb_t armv7m_scb_t;
struct armv7m_scb_t {
	volatile const uint32_t CPUID;
	volatile uint32_t ICSR;
	volatile uint32_t VTOR;
	volatile uint32_t AIRCR;
	volatile uint32_t SCR;
	volatile uint32_t CCR;
	volatile uint32_t SHPR1;
	volatile uint32_t SHPR2;
	volatile uint32_t SHPR3;
	volatile uint32_t SHCSR;
	union {
		volatile uint32_t CFSR;
		struct {
			uint16_t UFSR;
			uint8_t BFSR;
			uint8_t MMFSR;
		} __attribute__((packed));
	};
	volatile uint32_t HFSR;
	volatile uint32_t DFSR;
	volatile uint32_t MMFAR;
	volatile uint32_t BFAR;
	volatile uint32_t AFSR;
	volatile const uint32_t ID_PFR0;
	volatile const uint32_t ID_PFR1;
	volatile const uint32_t ID_DFR0;
	volatile const uint32_t ID_AFR0;
	volatile const uint32_t ID_MMFR0;
	volatile const uint32_t ID_MMFR1;
	volatile const uint32_t ID_MMFR2;
	volatile const uint32_t ID_MMFR3;
	volatile const uint32_t ID_ISAR0;
	volatile const uint32_t ID_ISAR1;
	volatile const uint32_t ID_ISAR2;
	volatile const uint32_t ID_ISAR3;
	volatile const uint32_t ID_ISAR4;
	volatile const uint32_t __reserved_0x74_0x87[5];
	volatile uint32_t CPACR;
} __attribute__((packed));

static armv7m_scb_t* const SCB = (armv7m_scb_t*)SCB_BASE;

#define SCB_HFSR_DEBUGEVT (1 << 31)
#define SCB_HFSR_FORCED (1 << 30)
#define SCB_HFSR_VECTTBL (1 << 1)


#define SCB_SHCSR_MEMFAULTENA (1 << 0)
#define SCB_SHCSR_BUSFAULTENA (1 << 1)
#define SCB_SHCSR_MEMFAULTPENDED (1 << 13)
#define SCB_SHCSR_BUSFAULTPENDED (1 << 14)

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
