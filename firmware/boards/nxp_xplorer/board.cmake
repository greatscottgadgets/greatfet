#
# This file is part of greatfet.
#
include_guard()

# General board information.
set(BOARD_ID    1)
set(BOARD_NAME  "NXP LPC4330 Xplorer")

# Pull in the common code for the LPC4330.
include(${PATH_LIBGREAT_FIRMWARE_CMAKE}/platform/lpc4330.cmake)
