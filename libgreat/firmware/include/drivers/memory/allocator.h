/*
 * This file is part of libgreat
 *
 * High-level dynamic memory allocator API.
 */

#include <stddef.h>
#include <stdint.h>

#ifndef __LIBGREAT_ALLOCATOR_H__
#define __LIBGREAT_ALLOCATOR_H__

// TODO: move this to a config.h?
#define CONFIG_LIBGREAT_HEAP_SIZE (32 * 1024)

// Definitions from umm_malloc.
void *umm_malloc(size_t size);
void *umm_calloc(size_t num, size_t size);
void *umm_realloc(void *ptr, size_t size);
void umm_free(void *ptr);

// If we're providing system alloc, create simple wrappers for the umm_* functions.
#ifndef LIBGREAT_DONT_DEFINE_ALLOC
	static inline void *malloc(size_t size) { return umm_malloc(size); }
	static inline void *calloc(size_t num, size_t size) { return umm_calloc(num, size); }
	static inline void *realloc(void *ptr, size_t size) { return umm_realloc(ptr, size); }
	static inline void free(void *ptr) { umm_free(ptr); }
#endif

#endif /* __LIBGREAT_ALLOCATOR_H __ */ 
