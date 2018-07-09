#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 2.8.9)

if(NOT DEFINED ENV{GREATFET_PATH})
  message(FATAL_ERROR "GREATFET_PATH must be defined to point to your GreatFET tree.")
endif()

set(PATH_GREATFET_FIRMWARE $ENV{GREATFET_PATH}/firmware/)
set(CMAKE_TOOLCHAIN_FILE ${PATH_GREATFET_FIRMWARE}/cmake/toolchain-arm-cortex-m.cmake)
project(pll_fsk)

include(${PATH_GREATFET_FIRMWARE}/cmake/greatfet-common.cmake)

set(SRC_M4
	main.c
	"${PATH_GREATFET_FIRMWARE}/common/fault_handler.c"
)

DeclareTargets()
