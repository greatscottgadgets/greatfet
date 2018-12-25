#
# This file is part of GreatFET
# Top directory CMake project for GreatFET firmware
#

cmake_minimum_required(VERSION 3.1.3)

set(PATH_GREATFET ${CMAKE_CURRENT_LIST_DIR}/..)

include(cmake/greatfet_prelude.cmake)
project(greatfet_firmware_all)

# Build each of the relevant firmware images.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/greatfet_usb)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/blinky)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/flash_stub)
