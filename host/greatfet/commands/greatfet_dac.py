#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import errno
import sys

import greatfet
from greatfet import GreatFET
from greatfet.utils import log_silent, log_verbose


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for experimenting with GreatFET's DAC")
    parser.add_argument('-S', '--set', nargs=1, type=int, help="DAC value to set on ADC0_0 (0-1023)") 
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

    if args.set:
        set(device, args.set[0])


def set(device, dac_value):
    device.apis.dac.set(dac_value)
    print("DAC value set to", dac_value)

if __name__ == '__main__':
    main()
