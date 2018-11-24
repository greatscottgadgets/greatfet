#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_TOOLCHAIN_FILE ../cmake/toolchain-arm-cortex-m.cmake)
set(PATH_GREATFET_FIRMWARE_GFUSB ../greatfet_usb/)

project(flash_stub)

# Override the active board to always reflect the (fully generic) flash stub.
include(../cmake/greatfet-common.cmake)

set(SRC_M4
    main.c

    # usb content
    ${PATH_GREATFET_FIRMWARE_GFUSB}/usb_bulk_buffer.c
    ${PATH_GREATFET_FIRMWARE_GFUSB}/usb_descriptor.c
    ${PATH_GREATFET_FIRMWARE_GFUSB}/usb_device.c
    ${PATH_GREATFET_FIRMWARE_GFUSB}/usb_endpoint.c

    # usb drivers
    "${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb.c"
    "${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb_request.c"
    "${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb_standard_request.c"
    "${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/${LIBGREAT_PLATFORM}/usb_queue.c"
    "${PATH_LIBGREAT_FIRMWARE_DRIVERS}/usb/comms_backend.c"

    # comms drivers
    "${PATH_LIBGREAT_FIRMWARE_DRIVERS}/comms/comms_class.c"
    "${PATH_LIBGREAT_FIRMWARE_DRIVERS}/comms/utils.c"

    # minimal collection of class drivers
    ${LIBGREAT_API_CLASSES}
    ${PATH_GREATFET_FIRMWARE_GFUSB}/classes/core.c
    ${PATH_GREATFET_FIRMWARE_GFUSB}/classes/debug.c
    ${PATH_GREATFET_FIRMWARE_GFUSB}/classes/firmware.c

    # simple drivers used
    "${PATH_GREATFET_FIRMWARE_COMMON}/fault_handler.c"
    "${PATH_GREATFET_FIRMWARE_COMMON}/debug.c"
    "${PATH_GREATFET_FIRMWARE_COMMON}/printf.c"
    "${PATH_GREATFET_FIRMWARE_COMMON}/spi_bus.c"
    "${PATH_GREATFET_FIRMWARE_COMMON}/rom_iap.c"
)


DeclareTargets()
