/*
 * This file is part of GreatFET
 */

#include <stdint.h>
#include <debug.h>

#include "greatfet_core.h"
#include "fault_handler.h"
#include "wwdt.h"

extern volatile uint32_t reset_reason;

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

volatile hard_fault_stack_t* hard_fault_stack_pt;

/**
 * Trigger an emergency reset, which resets with REASON_FAULT.
 */
static void emergency_reset()
{
	pr_emergency("Resetting system following fault!\n");
	pr_emergency("Performing emergency reset...\n");
	reset_reason = RESET_REASON_FAULT;

	// Perform the actual reset.
	wwdt_reset(100000);
	while(1);
}


/**
 * Prints the system's state at a given log level.
 */
void print_system_state(loglevel_t loglevel, hard_fault_stack_t *args)
{
	printk(loglevel, "PC: %08x\n", args->pc);
	printk(loglevel, "LR: %08x\n", args->lr);

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
	printk(loglevel, "Current core: unknown\n"); // FIXME: include m4/m0
	printk(loglevel, "R0: %08x\t\tR1: %08x\n", args->r0, args->r1);
	printk(loglevel, "R2: %08x\t\tR3: %08x\n", args->r2, args->r3);
	printk(loglevel, "R12: %08x\t\tPSR: %08x\n", args->r12, args->psr);

	// TODO: print stack

	// Fin.
	printk(loglevel, "\n");
}

__attribute__((used)) void hard_fault_handler_c(hard_fault_stack_t* state)
{
	pr_emergency("\n\n");
	pr_emergency("FAULT: hard fault detected!\n");
	print_system_state(LOGLEVEL_EMERGENCY, state);
	emergency_reset();
}

void mem_manage_handler_c(hard_fault_stack_t *state)
{
	pr_emergency("\n\n");
	pr_emergency("FAULT: memory management fault detected!\n");
	print_system_state(LOGLEVEL_EMERGENCY, state);
	emergency_reset();
}

void bus_fault_handler() {
	pr_emergency("\n\n");
	pr_emergency("FAULT: bus fault detected!\n");
	emergency_reset();
}

void usage_fault_handler() {
	pr_emergency("\n\n");
	pr_emergency("FAULT: usage fault detected!\n");
	emergency_reset();
}
