#
# This file is part of greatfet.
# Common code meant to run before the local project declaration.
#

find_package(Git QUIET)

# If we're an external project, it may make sense to read the GREATFET PATH from an environment variable.
# If we're not, the parent project should have set the PATH_GREATFET variable.
if(DEFINED ENV{GREATFET_PATH})
    set(PATH_GREATFET $ENV{GREATFET_PATH})
endif()

# If we don't know where GreatFET is, but we have git, try to use rev-parse to find our root.
if(NOT DEFINED PATH_GREATFET AND GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
        RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE POTENTIAL_PATH)
    if(GIT_RESULT EQUAL "0")
        string(REPLACE "\n" "" PATH_GREATFET "${POTENTIAL_PATH}")
        message(STATUS "Assuming project root of ${PATH_GREATFET}.")
    endif()
endif()

# Ensure we know where the GreatFET path is located.
if(NOT DEFINED PATH_GREATFET)
    message(FATAL_ERROR "Can't figure out the location of the GreatFET tree. You may need to set the GREATFET_PATH environment variable.")
endif()

# Ensure the GreatFET path is real.
if(NOT EXISTS "${PATH_GREATFET}")
    message(FATAL_ERROR "Path \"${PATH_GREATFET}\" does not exist -- are you setting GREATFET_PATH correctly?")
endif()

# Set some top-level paths we'll want to use later.
set(PATH_GREATFET_FIRMWARE        ${PATH_GREATFET}/firmware)
set(PATH_GREATFET_FIRMWARE_CMAKE  ${PATH_GREATFET_FIRMWARE}/cmake)
set(PATH_GREATFET_FIRMWARE_COMMON ${PATH_GREATFET_FIRMWARE}/common)

# Define the path to libgreat, which allows us to use libgreat.
set(PATH_LIBGREAT                 ${PATH_GREATFET}/libgreat)
set(PATH_LIBGREAT_FIRMWARE_CMAKE  ${PATH_GREATFET}/libgreat/firmware/cmake)

# CMake compatibility for older CMake versions. This is nice to have for submodules(), but not required, so we'll
# first try to include this with it optional.
include(${PATH_LIBGREAT_FIRMWARE_CMAKE}/compatibility.cmake OPTIONAL)

# Ensure that we have any submodules we depend on.
include(${PATH_GREATFET}/firmware/cmake/submodules.cmake)

# CMake compatibility for older CMake versions. We now _definitely_ should have this.
include(${PATH_LIBGREAT_FIRMWARE_CMAKE}/compatibility.cmake)

# Figure out which board we'll be building for.
include(${PATH_GREATFET_FIRMWARE_CMAKE}/board_select.cmake)

# And include libgreat's prelude.
include(${PATH_LIBGREAT}/firmware/cmake/libgreat_prelude.cmake)

# Default to release builds, if not set.
set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Determines the type of build; usually 'MinSizeRel' or 'Debug'. See the CMake documentation for more info.")

# Don't show optionst hat aren't relevant to us in ccmake.
mark_as_advanced(CMAKE_INSTALL_PREFIX)
