/*
 * This file is part of GreatFET
 */

#include "usb_api_spiflash.h"
#include <drivers/usb/lpc43xx/usb_queue.h>

#include <stddef.h>
#include <greatfet_core.h>
#include <spiflash.h>
#include <spiflash_target.h>
#include <gpio_lpc.h>

/* Buffer size == spi_flash.page_len */
uint8_t spiflash_buffer[256U];

#define W25Q80BV_DEVICE_ID_RES  0x14 /* Expected device_id for W25Q16DV */

#define W25Q80BV_PAGE_LEN 256U
#define W25Q80BV_NUM_PAGES 8192U
#define W25Q80BV_NUM_BYTES (W25Q80BV_PAGE_LEN * W25Q80BV_NUM_PAGES)

static struct gpio_t gpio_spiflash_hold   = GPIO(1, 14);
static struct gpio_t gpio_spiflash_wp     = GPIO(1, 15);
static struct gpio_t gpio_spiflash_select;
static spi_target_t spi_target = {
	.bus = &spi_bus_ssp0,
	.gpio_hold = &gpio_spiflash_hold,
	.gpio_wp = &gpio_spiflash_wp,
};

static spiflash_driver_t spi_flash_drv = {
	.target = &spi_target,
    .target_init = spiflash_target_init,
};

struct flash_params {
	uint16_t page_len;
	uint16_t num_pages;
	uint32_t num_bytes;
	uint16_t gpio_select;
	uint8_t device_id;

};
struct flash_params params;

usb_request_status_t usb_vendor_request_spiflash_init(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
	if ((stage == USB_TRANSFER_STAGE_SETUP) &&
		(endpoint->setup.length == 11)) {
		usb_transfer_schedule_block(endpoint->out, &params,
									sizeof(struct flash_params),
									NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		spi_flash_drv.page_len = params.page_len;
		spi_flash_drv.num_pages = params.num_pages;
		spi_flash_drv.num_bytes = params.num_bytes;
		spi_flash_drv.device_id = params.device_id;
		// Can't use the GPIO() define as the struct alreay exists
		GPIO_SET(gpio_spiflash_select, (params.gpio_select>>8)&0xFF, params.gpio_select&0xFF);

		spi_target.gpio_select = &gpio_spiflash_select;
		spi_bus_start(spi_flash_drv.target, &ssp_config_spi);
		spiflash_setup(&spi_flash_drv);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_spiflash_erase(
		usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage) {
		if (stage == USB_TRANSFER_STAGE_SETUP) {
			/* only chip erase is implemented */
			spiflash_chip_erase(&spi_flash_drv);
			usb_transfer_schedule_ack(endpoint->in);
		}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_spiflash_write(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint32_t addr = 0;
	uint16_t len = 0;

	if (stage == USB_TRANSFER_STAGE_SETUP) {
		addr = (endpoint->setup.value << 16) | endpoint->setup.index;
		len = endpoint->setup.length;
		if ((len > spi_flash_drv.page_len) || (addr > spi_flash_drv.num_bytes)
				|| ((addr + len) > spi_flash_drv.num_bytes)) {
			return USB_REQUEST_STATUS_STALL;
		} else {
			usb_transfer_schedule_block(endpoint->out, &spiflash_buffer[0], len,
						    NULL, NULL);
			spi_bus_start(spi_flash_drv.target, &ssp_config_spi);
			spiflash_setup(&spi_flash_drv);
			return USB_REQUEST_STATUS_OK;
		}
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		addr = (endpoint->setup.value << 16) | endpoint->setup.index;
		len = endpoint->setup.length;
		/* This check is redundant but makes me feel better. */
		if ((len > spi_flash_drv.page_len) || (addr > spi_flash_drv.num_bytes)
				|| ((addr + len) > spi_flash_drv.num_bytes)) {
			return USB_REQUEST_STATUS_STALL;
		} else {
			spiflash_program(&spi_flash_drv, addr, len, &spiflash_buffer[0]);
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		}
	} else {
		return USB_REQUEST_STATUS_OK;
	}
}

usb_request_status_t usb_vendor_request_spiflash_read(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint32_t addr;
	uint16_t len;

	if (stage == USB_TRANSFER_STAGE_SETUP)
	{
		addr = (endpoint->setup.value << 16) | endpoint->setup.index;
		len = endpoint->setup.length;
		if ((len > spi_flash_drv.page_len) || (addr > spi_flash_drv.num_bytes)
			    || ((addr + len) > spi_flash_drv.num_bytes)) {
			return USB_REQUEST_STATUS_STALL;
		} else {
			spiflash_read(&spi_flash_drv, addr, len, &spiflash_buffer[0]);
			usb_transfer_schedule_block(endpoint->in, &spiflash_buffer[0], len,
						    NULL, NULL);
			return USB_REQUEST_STATUS_OK;
		}
	} else if (stage == USB_TRANSFER_STAGE_DATA)
	{
			addr = (endpoint->setup.value << 16) | endpoint->setup.index;
			len = endpoint->setup.length;
			/* This check is redundant but makes me feel better. */
			if ((len > spi_flash_drv.page_len) || (addr > spi_flash_drv.num_bytes)
					|| ((addr + len) > spi_flash_drv.num_bytes))
			{
				return USB_REQUEST_STATUS_STALL;
			} else
			{
				usb_transfer_schedule_ack(endpoint->out);
				return USB_REQUEST_STATUS_OK;
			}
	} else
	{
		return USB_REQUEST_STATUS_OK;
	}
}
