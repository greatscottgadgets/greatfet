/*
 * This file is part of libgreat
 *
 * High-level dynamic memory allocator API.
 */


#include <string.h>

#include <debug.h>
#include <toolchain.h>

#include <drivers/memory/allocator.h>

// Initialization import from the UMM library.
void umm_init(void);

// Allocate the heap here.
uint8_t libgreat_heap[CONFIG_LIBGREAT_HEAP_SIZE] ATTR_ALIGNED(32) ATTR_SECTION(".bss.heap");

/**
 * Early initialization function that sets up our use of the heap.
 */
void initialize_heap_allocator()
{
    // Clear out our heap. This is required by umm_malloc.
    memset(libgreat_heap, 0, sizeof(libgreat_heap));

	// Initialize our heap allocator.
	umm_init();
}
CALL_ON_PREINIT(initialize_heap_allocator);
