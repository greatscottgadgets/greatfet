#
# This file is part of greatfet
#

#
# FIXME: make this version configurable.
#
execute_process(
	COMMAND git log -n 1 --format=%h
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	RESULT_VARIABLE GIT_VERSION_FOUND
	ERROR_QUIET
	OUTPUT_VARIABLE GIT_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_VERSION_FOUND)
	set(VERSION "unknown")
else(GIT_VERSION_FOUND)
	set(VERSION ${GIT_VERSION})
endif()
