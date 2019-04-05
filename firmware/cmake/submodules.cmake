#
# CMake module for automatically fetching submodules.
# Should help to smoothe over some of our common support issues.
#

if(COMMAND include_guard)
    include_guard()
endif()

find_package(Git QUIET)

# If this was checked out as a git repository, ensure we also checked out the submodules.
if(GIT_FOUND AND EXISTS "${PATH_GREATFET}/.git")

    # Allow the user to disable submodule checkout.
    option(AUTOPULL_GIT_SUBMODULES "Automatically update submodules prior to build, when necessary." ON)
    option(FETCH_GIT_SUBMODULES "Fetch submdoules if they're not currently pressent." ON)

    # Try to pull down the submodules to build this.
    if(AUTOPULL_GIT_SUBMODULES OR (NOT EXISTS "${PATH_GREATFET}/libgreat/.git" AND FETCH_GIT_SUBMODULES))
        message(STATUS "Fetching git submodules.")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --rebase
                        WORKING_DIRECTORY ${PATH_GREATFET} RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "Pulling down submodules failed! (${GIT_SUBMOD_RESULT})")
        endif()
    endif()
endif()

# Check for libgreat.
if(NOT EXISTS "${PATH_GREATFET}/libgreat/README.md")
    message(FATAL_ERROR "The `libgreat` submodule is missing, and we can't automatically download it.")
endif()

# For now, also check out libopencm3.
if(NOT EXISTS "${PATH_GREATFET}/firmware/libopencm3/README")
    message(FATAL_ERROR "The `libopencm3` submodule is missing, and we can't automatically download it.")
endif()
