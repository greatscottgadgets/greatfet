#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import argparse
import errno
import sys
import time

import greatfet
from greatfet import GreatFET
from greatfet.utils import log_silent, log_verbose
from greatfet.protocol import vendor_requests

from greatfet.peripherals.i2c_device import I2CDevice
from greatfet_test import *

def main():
    # Set up a simple argument parser.
    parser = argparse.ArgumentParser(description="Utility for flashing the GreatFET's onboard SPI flash")
    parser.add_argument('-s', dest='serial', metavar='<serialnumber>', type=str,
                        help="Serial number of device, if multiple devices", default=None)
    parser.add_argument('-v', dest='verbose', action='store_true', help="Write data from file")
    args = parser.parse_args()

    log_function = log_verbose if args.verbose else log_silent

    try:
        log_function("Trying to find a GreatFET device...")
        device = GreatFET(serial_number=args.serial)
        log_function("{} found. (Serial number: {})".format(device.board_name(), device.serial_number()))
    except greatfet.errors.DeviceNotFoundError:
        if args.serial:
            print("No GreatFET board found matching serial '{}'.".format(args.serial), file=sys.stderr)
        else:
            print("No GreatFET board found!", file=sys.stderr)
        sys.exit(errno.ENODEV)



    # device.vendor_request_in(vendor_requests.I2C_START, length=1)
    # print("started")
    # device.vendor_request_out(vendor_requests.I2C_XFER, value=0x40>>1, index=2)
    # print("transferred")
    # 0x40 (write) 0x41 (read)
    # device.vendor_request_in(vendor_requests.I2C_XFER, value=0x41, length=2, index=0)

    tester = find_tester()
    u1 = initialize_jig(tester)    

if __name__ == '__main__':
    main()
