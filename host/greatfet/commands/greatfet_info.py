#!/usr/bin/env python
#
# This file is part of GreatFET

from __future__ import print_function

import errno
import sys

from greatfet import GreatFET
from greatfet.errors import DeviceNotFoundError


def main():
    try:
        device = GreatFET()
    except DeviceNotFoundError:
        print('No GreatFET devices found!', file=sys.stderr)
        sys.exit(errno.ENODEV)

    # Print the board's information...
    print("Found a {}!".format(device.board_name()))
    print("  Board ID: {}".format(device.board_id()))
    print("  Firmware version: {}".format(device.firmware_version()))
    print("  Part ID: {}".format(device.part_id()))
    print("  Serial number: {}".format(device.serial_number()))


    # Dev note: you can easily use this to test low-level interfaces.
    #
    # For example, toggle its third LED, use:
    #
    #   device.vendor_request_out(vendor_requests.LED_TOGGLE, 3)
    #
    # Or to send the ENABLE_USB1 request, use:
    #
    #   device.vendor_request_out(vendor_requests.ENABLE_USB1)
    #
    # where ENABLE_USB1 is just an integer constant.


if __name__ == '__main__':
    main()
