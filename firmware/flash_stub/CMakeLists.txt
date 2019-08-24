#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 3.1.3)

# Point the target to the GreatFET project.
set(PATH_GREATFET ${CMAKE_CURRENT_LIST_DIR}/../..)

include(../cmake/greatfet_prelude.cmake)
project(flash_stub C CXX ASM)
include(../cmake/greatfet.cmake)

# Get a quick reference to the main GreatFET-usb project, which (temporarily) provides some of the functionality we use.
set(PATH_GREATFET_USB ${PATH_GREATFET_FIRMWARE}/greatfet_usb)

add_greatfet_targets(${PROJECT_NAME}
	${PATH_GREATFET_USB}/usb_bulk_buffer.c
	${PATH_GREATFET_USB}/usb_descriptor.c
	${PATH_GREATFET_USB}/usb_device.c
	${PATH_GREATFET_USB}/usb_endpoint.c
	${PATH_GREATFET_USB}/classes/core.c
	${PATH_GREATFET_USB}/classes/debug.c
	${PATH_GREATFET_USB}/classes/firmware.c
	${CMAKE_CURRENT_SOURCE_DIR}/main.c
)
use_libgreat_modules(${PROJECT_NAME} comms usb usb_comms gpio)

# If we're not building DFU targets, don't include this in ALL.
if (NOT DFU_TARGETS_AVAILABLE)
	set(DFU_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
endif()

# Allow the flash stub to be included with the host distribution.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
	DESTINATION ${PATH_GREATFET}/host/greatfet/assets
)
