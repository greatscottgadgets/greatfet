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
from greatfet.protocol import vendor_requests
from greatfet.utils import log_silent, log_verbose


def main():
    # Set up a simple argument parser.
    parser = argparse.ArgumentParser(description="Utility for experimenting with GreatFET's ADC")
    parser.add_argument('-s', dest='serial', metavar='<serialnumber>', type=str,
                        help="Serial number of device, if multiple devices", default=None)
    parser.add_argument('-o', dest='off', action='store_true', help="Disable output")
    parser.add_argument('-c', dest='cw', action='store_true', help="CW output")
    parser.add_argument('-a', dest='ask', action='store_true', help="Basic ASK output")
    parser.add_argument('-f', dest='fsk', action='store_true', help="Basic FSK output")
    parser.add_argument('-p', dest='psk', action='store_true', help="PSK Opera Cake function")
    parser.add_argument('-v', dest='verbose', action='store_true', help="Increase verbosity of logging")
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

    if args.off:
        device.vendor_request_out(vendor_requests.RFHAX, index=0)
    if args.cw:
        device.vendor_request_out(vendor_requests.RFHAX, index=1, value=452)
    if args.ask:
        device.vendor_request_out(vendor_requests.RFHAX, index=2, value=4339)
    if args.fsk:
        device.vendor_request_out(vendor_requests.RFHAX, index=3, value=4339)
    if args.psk:
        device.vendor_request_out(vendor_requests.RFHAX, index=4, value=4339)

if __name__ == '__main__':
    main()
