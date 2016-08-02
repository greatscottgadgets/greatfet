# - Try to find the libgreatfet library
# Once done this defines
#
#  LIBGREATFET_FOUND - system has libgreatfet
#  LIBGREATFET_INCLUDE_DIR - the libgreatfet include directory
#  LIBGREATFET_LIBRARIES - Link these to use libgreatfet

# Copyright (c) 2013  Benjamin Vernoux
#


if (LIBGREATFET_INCLUDE_DIR AND LIBGREATFET_LIBRARIES)

  # in cache already
  set(LIBGREATFET_FOUND TRUE)

else (LIBGREATFET_INCLUDE_DIR AND LIBGREATFET_LIBRARIES)
  IF (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_LIBGREATFET QUIET libgreatfet)
  ENDIF(NOT WIN32)

  FIND_PATH(LIBGREATFET_INCLUDE_DIR
    NAMES greatfet.h
    HINTS $ENV{LIBGREATFET_DIR}/include ${PC_LIBGREATFET_INCLUDEDIR}
    PATHS /usr/local/include/libgreatfet /usr/include/libgreatfet /usr/local/include
    /usr/include ${CMAKE_SOURCE_DIR}/../libgreatfet/src
    /opt/local/include/libgreatfet
    ${LIBGREATFET_INCLUDE_DIR}
  )

  set(libgreatfet_library_names greatfet)

  FIND_LIBRARY(LIBGREATFET_LIBRARIES
    NAMES ${libgreatfet_library_names}
    HINTS $ENV{LIBGREATFET_DIR}/lib ${PC_LIBGREATFET_LIBDIR}
    PATHS /usr/local/lib /usr/lib /opt/local/lib ${PC_LIBGREATFET_LIBDIR} ${PC_LIBGREATFET_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/../libgreatfet/src
  )

  if(LIBGREATFET_INCLUDE_DIR)
    set(CMAKE_REQUIRED_INCLUDES ${LIBGREATFET_INCLUDE_DIR})
  endif()

  if(LIBGREATFET_LIBRARIES)
    set(CMAKE_REQUIRED_LIBRARIES ${LIBGREATFET_LIBRARIES})
  endif()

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBGREATFET DEFAULT_MSG LIBGREATFET_LIBRARIES LIBGREATFET_INCLUDE_DIR)

  MARK_AS_ADVANCED(LIBGREATFET_INCLUDE_DIR LIBGREATFET_LIBRARIES)

endif (LIBGREATFET_INCLUDE_DIR AND LIBGREATFET_LIBRARIES)
