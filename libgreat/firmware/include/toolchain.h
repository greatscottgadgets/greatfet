/*
 * This file is part of libgreat
 *
 * Toolchain helper functions.
 */


/**
 * Generic attribute wrappers.
 */
#define ATTR_PACKED __attribute__((packed))
#define ATTR_ALIGNED(x)	__attribute__ ((aligned(x)))
#define ATTR_SECTION(x) __attribute__ ((section(x)))
#define WEAK __attribute__((weak))

/**
 * Macros for populating the preinit_array and init_array
 * headers -- which are automatically executed in by our crt0 during startup.
 */
#define CALL_ON_PREINIT(preinit) \
	__attribute__((section(".preinit_array"), used)) static typeof(preinit) *preinit##_initcall_p = preinit;
#define CALL_ON_INIT(init) \
	__attribute__((section(".init_array"), used)) static typeof(init) *init##_initcall_p = init;
