#
# This file is part of greatfet
#

# Assume an unknown version by default.
set (VERSION "unknown")

# Try to read the version from our VERSION file.
if (EXISTS "${PATH_GREATFET}/VERSION")
	file(READ "${PATH_GREATFET}/VERSION" VERSION)

	# When we increment our cmake version requirement, update this to string(STRIP).
	string(REGEX REPLACE "\n$" "" VERSION "${VERSION}")
	set(VERSION_STRING "v${VERSION}")

# ... failing that, use git to identify the version.
else()
	execute_process(
		COMMAND git describe --dirty
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		RESULT_VARIABLE GIT_VERSION_FOUND
		ERROR_QUIET
		OUTPUT_VARIABLE GIT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	if(GIT_VERSION_FOUND)
		set(VERSION "${GIT_VERSION}")
		set(VERSION_STRING "git-${GIT_VERSION}")
	endif()
endif()
