#
# This file is part of greatfet.
#
#

include_guard()

# Get a shortcut to the location of our per-board defines.

# Provide a user-configurable board option.
set(BOARD "Azalea" CACHE STRING "Determines which GreatFET-compatible board will be targeted.")
string(TOLOWER "${BOARD}" BOARD_NAME)

# Compute the path that should house the configuration for this board.
set(PATH_GREATFET_FIRMWARE_BOARD ${PATH_GREATFET_FIRMWARE}/boards/${BOARD_NAME})

# Ensure we have CMake defintiions for the relevant board.
if(NOT EXISTS ${PATH_GREATFET_FIRMWARE_BOARD}/board.cmake)
	message(FATAL_ERROR "Could not find configruation for the board '${BOARD_NAME}'! Did you define BOARD correctly?")
endif()

# And include the per-board definitions.
include(${PATH_GREATFET_FIRMWARE_BOARD}/board.cmake)

# By default, add the board directory to our include path.
set(BOARD_INCLUDE_DIRECTORIES "${PATH_GREATFET_FIRMWARE_BOARD}" "${BOARD_INCLUDE_DIRECTORIES}")

if (BOARD_NAME)
	message(STATUS "Building for ${BOARD_NAME}.")
endif()
