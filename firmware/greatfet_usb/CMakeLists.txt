#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_TOOLCHAIN_FILE ../cmake/toolchain-arm-cortex-m.cmake)

project(greatfet_usb)

include(../cmake/greatfet-common.cmake)

set(SRC_M4
	greatfet_usb.c
	usb_request_handlers.c
	usb_bulk_buffer.c
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb_host.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb_request.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb_standard_request.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/gpdma.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/gpio_dma.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/gpio_scu.c"
	usb_descriptor.c
	usb_device.c
	usb_endpoint.c
	usb_api_board.c
	usb_api_spiflash.c
	usb_api_spi.c
	usb_api_adc.c
	usb_api_i2c.c
	usb_api_gpio.c
	usb_api_leds.c
	usb_api_heartbeat.c
	usb_api_rfhax.c
	usb_api_logic_analyzer.c
	usb_api_sdir.c
	usb_api_greatdancer.c
	usb_api_usbhost.c
	usb_api_glitchkit.c
	usb_api_glitchkit_simple.c
	usb_api_glitchkit_usb.c
	usb_api_DS18B20.c
	sgpio_isr.c
	"${PATH_GREATFET_FIRMWARE_COMMON}/glitchkit.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb_queue.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb_queue_host.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/fault_handler.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/rom_iap.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/spi_bus.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/debug.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/sgpio.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/one_wire.c"
)

DeclareTargets()
