#!/usr/bin/env python
#
# Copyright 2015 Dominic Spill <dominicgs@gmail.com)
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

from __future__ import print_function

import sys

import greatfet
from greatfet import GreatFET
from greatfet.protocol import vendor_requests

if __name__ == '__main__':

    device = GreatFET()
    if not device:
        print('No GreatFET devices found!')
        sys.exit()

    # Print the board's information...
    print("Found a {}!".format(device.board_name()))
    print("  Board ID: {}".format(device.board_id()))
    print("  Firmware version: {}".format(device.firmware_version()))
    print("  Serial number: {}".format(device.serial_number()))

    # ... and toggle it's third LED, for fun.
    device._vendor_request_out(vendor_requests.LED_TOGGLE, 3)

    # Dev note: you can still easily use this to test low-level interfaces.
    # For example, to send the ENABLE_USB1 request, use:
    #
    #   device._vendor_request_out(vendor_requests.ENABLE_USB1)
    #
    # where ENABLE_USB1 is just an integer constant.
