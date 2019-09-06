#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import errno
import sys
import ast

import greatfet
from greatfet import GreatFET
from greatfet.utils import log_silent, log_verbose
from greatfet.interfaces.spi_bus import SPIBus


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for SPI communication via GreatFET")
    parser.add_argument('-r', '--read', default=0, help="Number of bytes expecting to receive from the SPI Bus")
    parser.add_argument('-w', '--write', nargs='*', type=ast.literal_eval, default=[], help="Bytes to send over the SPI Bus")
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

    if args.write:
        transmit(device, args.write, int(args.read), log_function)


def transmit(device, data, receive_length, log_function):
    spi_bus = SPIBus(device)
    result = spi_bus.transmit(data, receive_length)
    print("received data:", result)


if __name__ == '__main__':
    main()
