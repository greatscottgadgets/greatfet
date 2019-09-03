#
# This file is part of greatfet.
#
# Common cmake routines for building GreatFET firmware.

include_guard()

set(FLAGS_COMPILE_COMMON -std=gnu11 -Os -g3 -Wall -Wextra -Wno-format -fno-common -MD -fno-builtin-printf -Wno-missing-field-initializers)
set(FLAGS_LINK_COMMON -Wl,--gc-sections -Os)

# Variable which is used to set default values for features that should only be on in debug builds.
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(DEFAULT_DEBUG_ONLY ON)
else()
    set(DEFAULT_DEBUG_ONLY OFF)
endif()

# libopencm3 requirements: generator stubd
# TODO: better way to find the libgreat cmake includes?
include(${PATH_LIBGREAT}/firmware/cmake/libopencm3.cmake)

# Include libgreat.
include(${PATH_LIBGREAT}/firmware/cmake/libgreat.cmake)

# Pull in build configuration things.
include(${PATH_GREATFET_FIRMWARE_CMAKE}/build_config.cmake)


#
# Function that creates a new GreatFET library / source collection.
# Arguments: [library_name] [sources...]
#
function(add_greatfet_library LIBRARY_NAME)

	# For now, we're just a thin wrapper around add_libgreat_library.
	# This allows us to make sweeping modifications to GreatFET targets later, if we need to.
	add_libgreat_library(${LIBRARY_NAME} ${ARGN})

endfunction(add_greatfet_library)


#
# Function that creates a new GreatFET library / source collection iff the relevant library does not exist.
# Arguments: [library_name] [sources...]
#
function(add_greatfet_library_if_necessary LIBRARY_NAME)

	# If the target doesn't already exist, create it.
	if (NOT TARGET ${LIBRARY_NAME})
		add_greatfet_library(${LIBRARY_NAME} ${ARGN})
	endif()

endfunction(add_greatfet_library_if_necessary)

#
# All in one function that generates a set of GreatFET executables (and relevant commands).
# Arguments: [binary_name] [sources...]
#
function(add_greatfet_targets EXECUTABLE_NAME)

	# Create an object library that will encapsulate all of our relevant sources. This doesn't really create a physical
	# library, but does nicely group all of our sources into a single virtual target we can work with.
	add_greatfet_library(${EXECUTABLE_NAME} OBJECT ${ARGN} ${CLASSES_SOURCES})
	add_dependencies(${EXECUTABLE_NAME} libopencm3)

	# Create our executables.
	add_flash_executable(${EXECUTABLE_NAME}.bin $<TARGET_OBJECTS:${EXECUTABLE_NAME}> $<TARGET_OBJECTS:libgreatfet> $<TARGET_OBJECTS:libgreat>)

	# Program/flash targets.
	add_custom_target(${EXECUTABLE_NAME}-flash   DEPENDS ${EXECUTABLE_NAME}.bin COMMAND greatfet_firmware -Rw ${EXECUTABLE_NAME}.bin)
	add_custom_target(${EXECUTABLE_NAME}-program DEPENDS ${EXECUTABLE_NAME}.bin COMMAND greatfet_firmware -V ${EXECUTABLE_NAME}.bin)

endfunction(add_greatfet_targets)



#
# All in one function that generates a set of GreatFET executables (and relevant commands).
# Arguments: [binary_name] [sources...]
#
function(add_m0_loadable EXECUTABLE_NAME PRIMARY_EXECUTABLE_NAME)

	# Create our executables.
	# TODO: Allow the m0 to use the relevant libraries.
	create_secondary_cpu_executable(${EXECUTABLE_NAME}.bin ${ARGN})

	target_link_options(${EXECUTABLE_NAME}.bin.elf PRIVATE
		-Wl,--just-symbols=${PRIMARY_EXECUTABLE_NAME}.bin.elf
	)

	# Ensure we have a base file before our executable.
	# TODO: does this work well with out of tree builds?
	add_dependencies(${EXECUTABLE_NAME}.bin.elf ${PRIMARY_EXECUTABLE_NAME}.bin.elf)

	# Add a custom target for loading the relevant loadable.
	add_custom_target(${EXECUTABLE_NAME}-load   DEPENDS ${EXECUTABLE_NAME}.bin COMMAND greatfet_loadable --m0 ${EXECUTABLE_NAME}.bin)

endfunction(add_m0_loadable)


# Ensure we always know how to build the GreatFET common library (libgreatfet).
add_subdirectory(${PATH_GREATFET_FIRMWARE_COMMON} common)

# Ensure we always know how to build libgreat.
add_subdirectory(${PATH_LIBGREAT_FIRMWARE} libgreat)
