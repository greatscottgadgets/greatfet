#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import argparse
import errno
import struct
import sys
import time

import greatfet
from greatfet import GreatFET
from greatfet.utils import log_silent, log_verbose
from greatfet.protocol import vendor_requests


def main():
    # Set up a simple argument parser.
    parser = argparse.ArgumentParser(description="Periodically print temperature from DS18B20 sensor")
    parser.add_argument('-S', dest='s20', action='store_true', help='DS18S20')
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

    while True:
        data = device.vendor_request_in(vendor_requests.DS18B20_READ, length=2, timeout=2000)
        # temperature data is 16 bit signed
        temp = struct.unpack('<h', data)[0]
        if args.s20:
            temp /= 2.0
        else:
            temp /= 16.0
        print(time.strftime("%H:%M:%S"), temp, '{:.01f}'.format(temp * 9 / 5 + 32))
        time.sleep(1)

if __name__ == '__main__':
    main()
