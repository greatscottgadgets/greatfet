/*
 * This file is part of GreatFET
 */

#ifndef __GREATFET_CORE_H
#define __GREATFET_CORE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#ifndef __RUNNING_ON_HOST__

#include "spi_ssp.h"
#include "spiflash.h"

#include "i2c_bus.h"
#include "i2c_lpc.h"

#endif

#include "utils.h"

/* hardware identification number */
#define BOARD_ID_ONE 0
#define BOARD_ID_XPLORER  1
#define BOARD_ID_RAD1O	2

#ifdef GREATFET_ONE
#define BOARD_ID BOARD_ID_ONE
#endif

#ifdef NXP_XPLORER
#define BOARD_ID BOARD_ID_XPLORER
#endif

#ifdef RAD1O_BADGE
#define BOARD_ID BOARD_ID_RAD1O
#endif

// TODO: move into libgreat

/**
 * This special register is not cleared on reset-- it thus can
 * be used to pass reset reasons to future software stages.
 */
extern volatile uint32_t reset_reason;

/* Delay in clock cycles */
void delay(uint32_t duration);
/* Delay in us - VERY rough */
void delay_us(uint32_t duration);

/* TODO: Hide these configurations */
#ifndef __RUNNING_ON_HOST__

extern const ssp_config_t ssp_config_spi;
extern spi_bus_t spi_bus_ssp0;
extern const ssp_config_t ssp1_config_spi;
extern spi_bus_t spi_bus_ssp1;

extern i2c_bus_t i2c0;
extern i2c_bus_t i2c1;

#endif


void cpu_clock_init(void);
void cpu_clock_pll1_low_speed(void);
void cpu_clock_pll1_max_speed(void);

void rtc_init(void);

void pin_setup(void);

typedef enum {
	LED1 = 0,
	LED2 = 1,
	LED3 = 2,
	LED4 = 3,
} led_t;

void led_on(const led_t led);
void led_off(const led_t led);
void led_toggle(const led_t led);

void debug_led(uint8_t val);

#ifdef __cplusplus
}
#endif

#endif /* __GREATFET_CORE_H */
