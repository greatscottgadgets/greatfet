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
    parser.add_argument('-r', '--read', nargs=1, type=int, default=0, help="Number of bytes expecting to receive from the UART device")
    parser.add_argument('-w', '--write', nargs='*', type=ast.literal_eval, help="Byte to send to the UART device")
    parser.add_argument('-p', '--pin', nargs=1, type=str, help="Desired GratFET pin")
    parser.add_argument('-b', '--baud', nargs=1, type=int, default=9600, help="Desired baud rate") 
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
        read(device, args.pin[0], args.read[0], args.baud[0])
    if args.write:
        write(device, args.pin[0], args.write, args.baud[0])


def read(device, pin, rx_length, baud):
    d = UART(device)
    device.gpio.mark_pin_as_used(pin)
    uart_pin = d.get_pin(pin)
    rx_data = uart_pin.read(baud=baud, rx_length=rx_length)

    print("rx data:", rx_data)


def write(device, pin, data, baud):
    d = UART(device)
    device.gpio.mark_pin_as_used(pin)
    uart_pin = d.get_pin(pin)
    uart_pin.write(data, baud=baud)
    

if __name__ == '__main__':
    main()
    