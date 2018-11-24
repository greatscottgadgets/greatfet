#
# This file is part of GreatFET
#

# Top directory CMake project for GreatFET firmware

cmake_minimum_required(VERSION 3.1.3)
set(CMAKE_TOOLCHAIN_FILE cmake/toolchain-arm-cortex-m.cmake)

project (greatfet_firmware_all)

add_subdirectory(blinky)
add_subdirectory(greatfet_usb)
add_subdirectory(flash_stub)
