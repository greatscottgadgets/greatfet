#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 3.1.3)

# Point the target to the GreatFET project.
set(PATH_GREATFET ${CMAKE_CURRENT_LIST_DIR}/../..)

include(../cmake/greatfet_prelude.cmake)
project(greatfet_usb C CXX ASM)
include(../cmake/greatfet.cmake)

# Automatically include all GreatFET classes.
file(GLOB_RECURSE CLASSES_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/classes/*.c")

# Add the core of our GreatFET code.
add_greatfet_targets(${PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/greatfet_usb.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_request_handlers.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_bulk_buffer.c
	${CMAKE_CURRENT_SOURCE_DIR}/pin_manager.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_descriptor.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_device.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_endpoint.c
	${CMAKE_CURRENT_SOURCE_DIR}/sgpio_isr.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_spi.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_adc.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_sdir.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_usbhost.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_glitchkit_simple.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_DS18B20.c
	${CLASSES_SOURCES}
)
use_libgreat_modules(${PROJECT_NAME} allocator comms usb usb_comms gpio ad970x)

# TODO: include libgreat's m0_coprocessor include, to allow m0 code to be added
