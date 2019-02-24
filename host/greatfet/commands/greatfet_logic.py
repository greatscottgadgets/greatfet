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


def main():
    logfile = 'log.bin'
    # Set up a simple argument parser.
    parser = argparse.ArgumentParser(description="Logic analyzer implementation for GreatFET")
    parser.add_argument('-s', dest='serial', metavar='<serialnumber>', type=str,
                        help="Serial number of device, if multiple devices", default=None)
    parser.add_argument('-f', dest='filename', metavar='<filename>', type=str, help="Write data to file", default=logfile)
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

    device.apis.logic_analyzer.start()

    time.sleep(1)

    f = open(args.filename, 'wb')
    try:
        while True:
            d = device.comms.device.read(0x81, 16384, 1000)
            f.write(d)
    except KeyboardInterrupt:
        pass

    device.apis.logic_analyzer.stop()


if __name__ == '__main__':
    main()
