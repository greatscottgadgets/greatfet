/*
 * This file is part of GreatFET
 */

#include <toolchain.h>

#include <stdint.h>
#include <debug.h>
#include <drivers/reset.h>
#include <drivers/platform_clock.h>

#include <backtrace.h>

#include "greatfet_core.h"
#include "fault_handler.h"

typedef struct
{
	// System state (stored by our prelude).
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	union {
		uint32_t r7;
		uint32_t sp;
	};
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;

	// Exception stack (already pushed).
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr; /* Link Register. */
	uint32_t pc; /* Program Counter. */
	uint32_t psr;/* Program Status Register. */

} hard_fault_stack_t;


#define SAVE_STATE_AND_CALL(target) \
	__asm__("PUSH {r4-r11}"); \
	__asm__("TST LR, #4"); \
	__asm__("ITE EQ"); \
	__asm__("MRSEQ R0, MSP"); \
	__asm__("MRSNE R0, PSP"); \
	__asm__("B " #target)

#define HANDLER_THUNK_FOR(handler) \
	ATTR_NAKED void handler(void) { SAVE_STATE_AND_CALL(handler##_c); }

HANDLER_THUNK_FOR(hard_fault_handler);
HANDLER_THUNK_FOR(mem_manage_handler);
HANDLER_THUNK_FOR(bus_fault_handler);

volatile hard_fault_stack_t* hard_fault_stack_pt;

/**
 * Configure the system to use ancillary faults.
 */
static void set_up_fault_handlers()
{
	// Enable bus and memory-management faults.
	SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA | SCB_SHCSR_BUSFAULTENA | SCB_SHCSR_USGFAULTENA;
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
	backtrace_frame_t frame;

	uintptr_t frame_pointer = args->sp - sizeof(backtrace_frame_t);
	unsigned int program_counter = args->pc;

	uint32_t *stack = (uint32_t *)args->sp;

	// Other interesting registers to examine:
	//	CFSR: Configurable Fault Status Register
	//	HFSR: Hard Fault Status Register
	//	DFSR: Debug Fault Status Register
	//	AFSR: Auxiliary Fault Status Register
	//	MMAR: MemManage Fault Address Register
	//	BFAR: Bus Fault Address Register

	// TODO insert special registers
	printk(loglevel, "\n");
	printk(loglevel, "Current core: Cortex-M4\n"); // FIXME detect this; right now we're harcoding because we only use the M4
	printk(loglevel, "System clock source: %s @ %" PRIu32 " Hz\n",
		platform_get_cpu_clock_source_name(), platform_get_cpu_clock_source_frequency());
	printk(loglevel, " PC:  %08" PRIx32 "\t\tLR:    %08" PRIx32 "\n", program_counter,  args->lr);
	printk(loglevel, " R0:  %08" PRIx32 "\t\tR1:    %08" PRIx32 "\n", args->r0,  args->r1);
	printk(loglevel, " R2:  %08" PRIx32 "\t\tR3:    %08" PRIx32 "\n", args->r2,  args->r3);
	printk(loglevel, " R4:  %08" PRIx32 "\t\tR5:    %08" PRIx32 "\n", args->r4,  args->r5);
	printk(loglevel, " R6:  %08" PRIx32 "\t\tR7|SP: %08" PRIx32 "\n", args->r6,  args->r7);
	printk(loglevel, " R8:  %08" PRIx32 "\t\tR9:    %08" PRIx32 "\n", args->r8,  args->r9);
	printk(loglevel, " R10: %08" PRIx32 "\t\tR11:   %08" PRIx32 "\n", args->r10, args->r11);
	printk(loglevel, " R12: %08" PRIx32 "\t\tPSR:   %08" PRIx32 "\n", args->r12, args->psr);

	// Print the raw stack.
	printk(loglevel, "Stack:\n");
	for (int i = 0; i < 12; i += 4) {
		printk(loglevel, " %08x %08x %08x %08x\n", stack[i], stack[i + 1], stack[i + 2], stack[i + 3]);
	}

	// Print a stack trace, assuming we have one.
	frame.sp = frame_pointer;
	frame.fp = frame_pointer;
	frame.lr = args->lr;
	//frame.pc = program_counter;
	frame.pc = args->lr;
	print_backtrace_from_frame(loglevel, &frame, 0);

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

	if (SCB->BFSR & SCB_BFSR_BFARVALID) {
		const char *decorator;

		if (SCB->BFSR & SCB_BFSR_IBUSERR) {
			decorator = "(accessed as instruction)";
		}
		else if (SCB->BFSR & SCB_BFSR_PRECISERR) {
			decorator = "(accessed as data)";
		} else {
			decorator = "(access unknown)";
		}

		pr_emergency("Faulting address: 0x%08" PRIx32 " %s\n", SCB->BFAR, decorator);

	} else {
		if (SCB->BFSR & SCB_BFSR_IMPRECISERR) {
			pr_emergency("Faulting on data access; precise address not known (BFAR invalid).\n");
		} else {
			pr_emergency("Faulting address not known (BFAR invalid).\n");
		}
	}

	pr_emergency("    BFSR: %02x\t BFAR: %08" PRIx32 "\n", SCB->BFSR, SCB->BFAR);
	pr_emergency("    is instruction access error: %s\n", (SCB->BFSR & SCB_BFSR_IBUSERR) ? "yes" : "no");
	pr_emergency("    is data bus access error: %s\n",
			(SCB->BFSR & SCB_BFSR_PRECISERR) | (SCB->BFSR & SCB_BFSR_IMPRECISERR) ? "yes" : "no");
	pr_emergency("    stacking fault: %s\n", (SCB->BFSR & SCB_BFSR_STKERR) ? "yes" : "no");
	pr_emergency("    unstacking fault: %s\n", (SCB->BFSR & SCB_BFSR_UNSTKERR) ? "yes" : "no");



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
	pr_emergency("\n");
	pr_emergency("FAULT: hard fault detected!\n");
	pr_emergency("HFSR: %08" PRIx32 "\tSHCSR: %08" PRIx32 "\n", SCB->HFSR, SCB->SHCSR);
	pr_emergency("    on vector table read: %s\n", (SCB->HFSR & SCB_HFSR_VECTTBL) ? "yes" : "no");

	// If this is a forced exception, we likely got here from another fault handler.
	if (SCB->HFSR & SCB_HFSR_FORCED) {
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
	} else {
		pr_emergency("  not a forced exception\n\n");
	}

	print_system_state(LOGLEVEL_EMERGENCY, state);
	emergency_reset();
}
