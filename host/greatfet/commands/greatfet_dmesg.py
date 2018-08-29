#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function
from __future__ import absolute_import

import argparse
import errno
import sys
import time


# XXX debug only, allows out of tree
import os
sys.path.remove(os.path.dirname(os.path.realpath(__file__)))

import greatfet
from greatfet import GreatFET
from greatfet.protocol import vendor_requests
from greatfet.utils import log_silent, log_verbose


def main():
    parser = argparse.ArgumentParser(description="Utility for reading GreatFET debug information")
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

    #Read and print the logs
    logs = device.read_debug_ring()

    log_function("Ring buffer contained {} bytes of data:\n".format(len(logs)))
    print(logs)

if __name__ == '__main__':
    main()
    
