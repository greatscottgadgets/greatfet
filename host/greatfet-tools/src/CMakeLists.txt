# Copyright 2012 Jared Boone
# Copyright 2013 Benjamin Vernoux
#
# This file is part of GreatFET.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

# Based heavily upon the libftdi cmake setup.

set(INSTALL_DEFAULT_BINDIR "bin" CACHE STRING "Appended to CMAKE_INSTALL_PREFIX")

if(MSVC)
add_library(libgetopt_static STATIC
    ../getopt/getopt.c
)
endif()

add_executable(greatfet_transfer greatfet_transfer.c)
install(TARGETS greatfet_transfer RUNTIME DESTINATION ${INSTALL_DEFAULT_BINDIR})

#add_executable(greatfet_info greatfet_info.c)
#install(TARGETS greatfet_info RUNTIME DESTINATION ${INSTALL_DEFAULT_BINDIR})

if(NOT libgreatfet_SOURCE_DIR)
include_directories(${LIBGREATFET_INCLUDE_DIR})
LIST(APPEND TOOLS_LINK_LIBS ${LIBGREATFET_LIBRARIES})
else()
LIST(APPEND TOOLS_LINK_LIBS greatfet)
endif()

if(MSVC)
LIST(APPEND TOOLS_LINK_LIBS libgetopt_static)
endif()


target_link_libraries(greatfet_transfer ${TOOLS_LINK_LIBS})
#target_link_libraries(greatfet_info ${TOOLS_LINK_LIBS})
