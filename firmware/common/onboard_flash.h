/*
 * This file is part of GreatFET
 */

#ifndef __ONBOARD_FLASH_H__
#define __ONBOARD_FLASH_H__

#include "spiflash.h"
#include "spiflash_target.h"
#include "gpio_lpc.h"
#include "greatfet_core.h"
#include "pins.h"

struct gpio_t gpio_spiflash_hold   = GPIO(1, 14);
struct gpio_t gpio_spiflash_wp     = GPIO(1, 15);
struct gpio_t gpio_spiflash_select = GPIO(5, 11);
spi_target_t spi_target = {
	.bus = &spi_bus_ssp0,
	.gpio_hold = &gpio_spiflash_hold,
	.gpio_wp = &gpio_spiflash_wp,
	.gpio_select = &gpio_spiflash_select,
};

spiflash_driver_t spi_flash_drv = {
	.target = &spi_target,
	.target_init = spiflash_target_init,
	.page_len = ONBOARD_FLASH_PAGE_LEN,
	.num_pages = ONBOARD_FLASH_NUM_PAGES,
	.num_bytes = ONBOARD_FLASH_NUM_BYTES,
	.device_id = ONBOARD_FLASH_DEVICE_ID,
};


#endif//__ONBOARD_FLASH_H__
