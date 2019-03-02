#
# This file is part of greatfet.
# Board configuration for the GreatFET Azalea.
#
include_guard()

# General board information.
set(BOARD_ID    2)
set(BOARD_NAME  "CCC rad1o-badge")

# Pull in the common code for the LPC4330.
include(${PATH_LIBGREAT_FIRMWARE_CMAKE}/platform/lpc4330.cmake)
