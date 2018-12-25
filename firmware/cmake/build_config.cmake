#
# This file is part of greatfet.
#

# Automatically detect the GreatFET version.
include(${PATH_GREATFET_FIRMWARE_CMAKE}/detect_version.cmake)

# Allow for version suffixes to be included, allowing people to tag their customzied builds.
set(VERSION_SUFFIX "" CACHE STRING "A local suffix added to version numbers.")

# If we have a version suffix, include it in our build version-ing.
if (NOT(VERSION_SUFFIX STREQUAL ""))
    set(VERSION "${VERSION}-${VERSION_SUFFIX}")
endif()

# FIXME: handle versioning correctly
set(VERSION_STRING "git-${VERSION}")

# Generate the build configuration file, which informs source of our build configuration.
configure_file(${PATH_GREATFET_FIRMWARE}/config.h.in generated/config.h)
set(BUILD_INCLUDE_DIRECTORIES ${PROJECT_BINARY_DIR}/generated)
