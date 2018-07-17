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
    logfile = 'log.bin'
#    logfile = '/tmp/fifo'
    # Set up a simple argument parser.
    parser = argparse.ArgumentParser(description="Utility for experimenting with GreatFET's ADC")
    parser.add_argument('-s', dest='serial', metavar='<serialnumber>', type=str,
                        help="Serial number of device, if multiple devices", default=None)
    parser.add_argument('-f', dest='filename', metavar='<filename>', type=str, help="Write data to file", default=logfile)
    parser.add_argument('-v', dest='verbose', action='store_true', help="Write data from file")
    parser.add_argument('-a', dest='adc', action='store_true', help="Use internal ADC")
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

    if args.adc:
        device.vendor_request_out(vendor_requests.ADC_INIT)
    else:
        device.vendor_request_out(vendor_requests.SDIR_RX_START)

    time.sleep(1)
    print(device.device)

    with open(args.filename, 'wb') as f:
        try:
            while True:
                d = device.device.read(0x81, 0x4000, 1000)
                # print(d)
                f.write(d)
        except KeyboardInterrupt:
            pass

    if not args.adc:
        device.vendor_request_out(vendor_requests.SDIR_RX_STOP)


if __name__ == '__main__':
    main()
    
