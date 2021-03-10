#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 3.1.3)
set(PATH_GREATFET ${CMAKE_CURRENT_LIST_DIR}/../..)

include(../cmake/greatfet_prelude.cmake)
project(greatfet_common C CXX ASM)
include (../cmake/greatfet.cmake)

# firmware/common sources files used by greatfet_usb
add_greatfet_library_if_necessary(libgreatfet OBJECT
    ${PATH_GREATFET_FIRMWARE_COMMON}/gpdma.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/gpio_dma.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/gpio_scu.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/glitchkit.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/fault_handler.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/rom_iap.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/spi_bus.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/debug.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/sgpio.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/one_wire.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/greatfet_core.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/spiflash_target.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/spiflash.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/spi_ssp.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/i2c_bus.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/i2c_lpc.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/gpio_lpc.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/gpio_int.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/jtag.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/jtag_msp430.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/printf.c
    ${PATH_GREATFET_FIRMWARE_COMMON}/swra124.c
)

# printf.c is external code; override the compile flags to silence these warnings.
set_source_files_properties(${PATH_GREATFET_FIRMWARE_COMMON}/printf.c PROPERTIES COMPILE_FLAGS
	"-Wno-implicit-fallthrough -Wno-unused-variable -Wno-stringop-overflow")

# FIXME: move debug into its own module so we don't  have to manually code this
target_include_directories(libgreatfet PRIVATE ${PATH_LIBGREAT_FIRMWARE}/third-party/backtrace)
