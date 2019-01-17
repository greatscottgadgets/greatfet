/*
 * This file is part of GreatFET
 */

#include <toolchain.h>

#include <stdint.h>
#include <debug.h>
#include <drivers/reset.h>

#include "greatfet_core.h"
#include "fault_handler.h"


typedef struct
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr; /* Link Register. */
	uint32_t pc; /* Program Counter. */
	uint32_t psr;/* Program Status Register. */
} hard_fault_stack_t;

__attribute__((naked))
void hard_fault_handler(void) {
	__asm__("TST LR, #4");
	__asm__("ITE EQ");
	__asm__("MRSEQ R0, MSP");
	__asm__("MRSNE R0, PSP");
	__asm__("B hard_fault_handler_c");
}

// FIXME: deduplicate
__attribute__((naked))
void mem_manage_handler(void) {
	__asm__("TST LR, #4");
	__asm__("ITE EQ");
	__asm__("MRSEQ R0, MSP");
	__asm__("MRSNE R0, PSP");
	__asm__("B mem_manage_handler_c");
}
__attribute__((naked))
void bus_fault_handler(void) {
	__asm__("TST LR, #4");
	__asm__("ITE EQ");
	__asm__("MRSEQ R0, MSP");
	__asm__("MRSNE R0, PSP");
	__asm__("B bus_fault_handler_c");
}

volatile hard_fault_stack_t* hard_fault_stack_pt;

/**
 * Configure the system to use ancillary faults.
 */
static void set_up_fault_handlers()
{
	// Enable bus and memory-management faults.
	//SCB->SHCSR |= SCH_SHCSR_MEMFAULTENA | SCH_SHCSR_BUSFAULTENA;
}
CALL_ON_PREINIT(set_up_fault_handlers);


/**
 * Trigger an emergency reset, which resets with REASON_FAULT.
 */
static void emergency_reset(void)
{
	pr_emergency("Resetting system following fault!\n");
	pr_emergency("Performing emergency reset...\n");
	system_reset(RESET_REASON_FAULT, true);
}


/**
 * Prints the system's state at a given log level.
 */
void print_system_state(loglevel_t loglevel, hard_fault_stack_t *args)
{
	printk(loglevel, "PC: %08" PRIx32 "\n", args->pc);
	printk(loglevel, "LR: %08" PRIx32 "\n", args->lr);

	// Other interesting registers to examine:
	//	CFSR: Configurable Fault Status Register
	//	HFSR: Hard Fault Status Register
	//	DFSR: Debug Fault Status Register
	//	AFSR: Auxiliary Fault Status Register
	//	MMAR: MemManage Fault Address Register
	//	BFAR: Bus Fault Address Register

	// TODO insert relevant system state analysis here
	// TODO insert special registers
	printk(loglevel, "\n");
	printk(loglevel, "Current core: Cortex-M4\n"); // FIXME detect this; right now we're harcoding because we only use the M4
	printk(loglevel, "R0: %08" PRIx32 "\t\tR1: %08" PRIx32 "\n", args->r0, args->r1);
	printk(loglevel, "R2: %08" PRIx32 "\t\tR3: %08" PRIx32 "\n", args->r2, args->r3);
	printk(loglevel, "R12: %08" PRIx32 "\t\tPSR: %08" PRIx32 "\n", args->r12, args->psr);

	// TODO: print stack

	// Fin.
	printk(loglevel, "\n");
}


void mem_manage_handler_c(hard_fault_stack_t *state)
{
	pr_emergency("\n\n");
	pr_emergency("FAULT: memory management fault detected!\n");
	pr_emergency("    MMFSR: %02x\t MMFAR: %08" PRIx32 "\n", SCB->MMFSR, SCB->MMFAR);
	pr_emergency("    is instruction access violation: %s\n", (SCB->MMFSR & SCB_MMFSR_IACCVIOL) ? "yes" : "no");
	pr_emergency("    is data access violation: %s\n", (SCB->MMFSR & SCB_MMFSR_DACCVIOL) ? "yes" : "no");
	pr_emergency("    stacking fault: %s\n", (SCB->MMFSR & SCB_MMFSR_MSTKERR) ? "yes" : "no");
	pr_emergency("    unstacking fault: %s\n", (SCB->MMFSR & SCB_MMFSR_MUNSTKERR) ? "yes" : "no");

	if (SCB->MMFSR & SCB_MMFSR_MMARVALID) {
		pr_emergency("Faulting address: 0x%08" PRIx32 " (accessed as data)\n", SCB->MMFAR);
	} else {
		if (SCB->MMFSR & SCB_MMFSR_IACCVIOL) {
			pr_emergency("Faulting address: 0x%08" PRIx32 " (accessed as instruction)\n", state->pc);
		} else {
			pr_emergency("Faulting address not known (MMFAR invalid).\n");
		}
	}

	pr_emergency("\n");
	print_system_state(LOGLEVEL_EMERGENCY, state);
	emergency_reset();
}

void bus_fault_handler_c(hard_fault_stack_t *state) {
	pr_emergency("\n\n");
	pr_emergency("FAULT: bus fault detected!\n");
	print_system_state(LOGLEVEL_EMERGENCY, state);
	emergency_reset();
}

void usage_fault_handler() {
	pr_emergency("\n\n");
	pr_emergency("FAULT: usage fault detected!\n");
	emergency_reset();
}


__attribute__((used)) void hard_fault_handler_c(hard_fault_stack_t* state)
{
	// Announce the fault.
	pr_emergency("\n\n");
	pr_emergency("FAULT: hard fault detected!\n");
	pr_emergency("HFSR: %08" PRIx32 "\tSHCSR: %08" PRIx32 "\n", SCB->HFSR, SCB->SHCSR);
	pr_emergency("    on vector table read: %s\n", (SCB->HFSR & SCB_HFSR_VECTTBL) ? "yes" : "no");

	// If this is a forced exception, we likely got here from another fault handler.
	if (SCB->HFSR & SCB_HFSR_FORCED) {
		pr_emergency("\n");
		pr_emergency("FORCED exception! Looking for inner fault...\n");
		pr_emergency("    MMFSR: %02x\tBFSR: %02x\tUFSR: %04x\n", SCB->MMFSR, SCB->BFSR, SCB->UFSR);

		if (SCB->MMFSR & SCB_MMFSR_FAULT_MASK) {
			pr_emergency("Exception has an inner MM fault. Handling accordingly.\n");
			mem_manage_handler_c(state);
		}
		if (SCB->BFSR & SCB_BFSR_FAULT_MASK) {
			pr_emergency("Exception has an inner bus fault. Handling accordingly.\n");
			bus_fault_handler_c(state);
		}
		pr_emergency("    ... couldn't figure out what kind of fault this is. Continuing.\n");
		pr_emergency("\n");
	} else {
		pr_emergency("  not a forced exception\n\n");
	}

	print_system_state(LOGLEVEL_EMERGENCY, state);
	emergency_reset();
}
