#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_TOOLCHAIN_FILE ../cmake/toolchain-arm-cortex-m.cmake)
set(PATH_GREATFET_FIRMWARE_GFUSB ../greatfet_usb/)

project(flash_stub)

include(../cmake/greatfet-common.cmake)

set(SRC_M4
	main.c
	"${PATH_GREATFET_FIRMWARE_GFUSB}/usb_bulk_buffer.c"
    "${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb.c"
	"${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb_host.c"
	"${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb_request.c"
	"${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb_standard_request.c"
	"${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb_queue.c"
	"${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb_queue_host.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/gpdma.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/gpio_dma.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/gpio_scu.c"
	"${PATH_GREATFET_FIRMWARE_GFUSB}/usb_descriptor.c"
	"${PATH_GREATFET_FIRMWARE_GFUSB}/usb_device.c"
	"${PATH_GREATFET_FIRMWARE_GFUSB}/usb_endpoint.c"
	"${PATH_GREATFET_FIRMWARE_GFUSB}/usb_api_board.c"
	"${PATH_GREATFET_FIRMWARE_GFUSB}/usb_api_spiflash.c"
	"${PATH_GREATFET_FIRMWARE_GFUSB}/usb_api_heartbeat.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/glitchkit.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/fault_handler.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/rom_iap.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/spi_bus.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/debug.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/sgpio.c"
)

DeclareTargets()
