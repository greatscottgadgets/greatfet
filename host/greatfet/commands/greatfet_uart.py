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

from greatfet.peripherals.uart import UART


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for UART communication via GreatFET")
    parser.add_argument('-z', '--init', action='store_true', help="UART Initializer") 
    parser.add_argument('-r', '--read', action='store_true', help="Read data from the UART device")
    parser.add_argument('-w', '--write', default=0, type=ast.literal_eval, help="Byte to send to the UART device")
    parser.add_argument('-p', '--pin', nargs=1, type=str, help="Desired GratFET pin")
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

    if args.read:
        read(device)
    if args.write:
        write(device, args.pin[0], args.write)


def read(device):
    d = UART(device)
    device.gpio.mark_pin_as_used('J2_P20')
    uart_pin = d.get_pin('J2_P20')
    read_data = uart_pin.read()
    print("read data:", read_data)


def write(device, pin, data):
    d = UART(device)
    print("pin:", pin)
    device.gpio.mark_pin_as_used(pin)
    uart_pin = d.get_pin(pin)
    uart_pin.write(data)
    

if __name__ == '__main__':
    main()
    