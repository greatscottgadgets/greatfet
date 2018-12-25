#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 3.1.3)

# Point the target to the GreatFET project.
set(PATH_GREATFET ${CMAKE_CURRENT_LIST_DIR}/../..)

include(../cmake/greatfet_prelude.cmake)
project(blinky C CXX ASM)
include(../cmake/greatfet.cmake)

add_greatfet_targets(${PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/blinky.c
)
