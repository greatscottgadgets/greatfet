#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_TOOLCHAIN_FILE ../cmake/toolchain-arm-cortex-m.cmake)

project(blinky)

include(../cmake/greatfet-common.cmake)

set(SRC_M4
	blinky.c
)

DeclareTargets()
