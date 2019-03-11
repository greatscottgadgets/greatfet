#
# This file is part of GreatFET
#

include_guard(GLOBAL)

execute_process(
	COMMAND dfu-suffix -V
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	RESULT_VARIABLE DFU_NOT_FOUND
	ERROR_QUIET
	OUTPUT_VARIABLE DFU_VERSION_STRING
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Depending on how well the DFU utility binary ran, either mark us as capable or incapable of building DFU targets.
if(NOT DFU_NOT_FOUND)
	set(DFU_ALL "ALL" CACHE STRING "determines whether DFU variables will be included in the ALL target")
	set(DFU_TARGETS_AVAILABLE "YES" CACHE BOOL "identifies whether DFU targets can be built")
else(NOT DFU_NOT_FOUND)
	message(WARNING "dfu-suffix not found: not building DFU targets")
	set(DFU_ALL "" CACHE STRING "determines whether DFU variables will be included in the ALL target")
	set(DFU_TARGETS_AVAILABLE "NO" CACHE BOOL "identifies whether DFU targets can be built")
endif(NOT DFU_NOT_FOUND)

# DFU state shouldn't normally show up in the CMake configuration.
mark_as_advanced(DFU_ALL)
mark_as_advanced(DFU_TARGETS_AVAILABLE)
