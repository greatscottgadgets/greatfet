#
# This file is part of greatfet.
#

# Automatically detect the GreatFET version.
include(${PATH_GREATFET_FIRMWARE_CMAKE}/detect_version.cmake)

# Allow for version suffixes to be included, allowing people to tag their customzied builds.
set(VERSION_LOCAL_SUFFIX "" CACHE STRING "A local suffix added to version numbers.")

# If we have a version suffix, include it in our build version-ing.
if (NOT(VERSION_LOCAL_SUFFIX STREQUAL ""))
    set(VERSION "${VERSION}-${VERSION_LOCAL_SUFFIX}")
    set(VERSION_STRING "${VERSION_STRING}-${VERSION_LOCAL_SUFFIX}")
endif()

# Set the maximum depth of a debug backtrace.
set(DEBUG_MAX_BACKTRACE_DEPTH 25 CACHE STRING "The maximum depth of a backtrace to report, if backtracing is enabled.")
configuration_depends_on_features(DEBUG_MAX_BACKTRACE_DEPTH BACKTRACE)

# General options.
configuration_feature(LOGGING "Enables the core GreatFET logging functionality / dmesg. Turning off saves memory, but makes debugging much harder." ON)
dependent_configuration_feature(VERBOSE_LOGGING LOGGING "Enables the capability to log the very-verbose DEBUG and lower loglevels. Disabling saves memory." "${DEFAULT_DEBUG_ONLY}")
dependent_configuration_feature(VERBOSE_TRACING LOGGING "Enables the capability to log trace events to the debug ring. For very specific debug only." OFF)
dependent_configuration_feature(QUIET_LOGGING   LOGGING "Removes all informational logging; saving memory but reducing the usefulness of the ring buffer." OFF)
dependent_configuration_feature(LOG_TIMESTAMPS  LOGGING "If set, the system will timestamp each log line in the log with the number of microseconds into execution." ON)
dependent_configuration_feature(SEMIHOSTING     LOGGING "Uses ARM semihosting to live-print log information over JTAG/SWD when a debugger is connected." ON)
dependent_configuration_feature(DEBUG_RING      LOGGING "Keeps a local ringbuffer that allows debug logs to be fetched over e.g. USB. Uses a bit of memory; but very useful." ON)

# Set the default log level for GreatFET.
# TODO: bring this down to 5 for non-debug builds?
set(DEBUG_DEFAULT_LOG_LEVEL 6 CACHE STRING "The default log-level for GreatFET; higher = more logs. 5-6 is a normal informational level.")
configuration_depends_on_features(DEBUG_DEFAULT_LOG_LEVEL LOGGING)

# Set the size of the debug ring
set(DEBUG_RING_SIZE 4096 CACHE STRING "The size of the platform's debug ring, in bytes. Should be <= 4096.")
configuration_depends_on_features(DEBUG_RING_SIZE DEBUG_RING LOGGING DEBUG_RING)

# Specify whether we support backtracing, locally.
libgreat_configuration_for_feature(BACKTRACE)
libgreat_configuration_for_feature(FUNCTION_NAMES)

# Generate the build configuration file, which informs source of our build configuration.
configure_file(${PATH_GREATFET_FIRMWARE}/config.h.in generated/config.h)
set(BUILD_INCLUDE_DIRECTORIES ${PROJECT_BINARY_DIR}/generated)

