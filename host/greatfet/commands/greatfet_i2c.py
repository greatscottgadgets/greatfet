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
from greatfet.peripherals.i2c_device import I2CDevice
from greatfet.peripherals.i2c_bus import I2CBus


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for I2C communication via GreatFET")
    parser.add_argument('-a', '--address', nargs=1, type=ast.literal_eval, help="7-bit address for communication over the I2C Bus") 
    parser.add_argument('-r', '--read', default=0, help="Number of bytes expecting to receive from the I2C Bus")
    parser.add_argument('-w', '--write', nargs='*', type=ast.literal_eval, default=[], help="Bytes to send over the I2C Bus")
    parser.add_argument('-z', '--scan', action='store_true', help="Scan all possible i2c addresses")
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

    if args.scan:
        scan(device, log_function)
    if args.write:
        write(device, args.address[0], args.write, log_function)
    if args.read:
        read(device, args.address[0], int(args.read), log_function)


def read(device, address, receive_length, log_function):
    i2c_device = I2CDevice(device.i2c, address)
    received_data, i2c_status = i2c_device.read(receive_length)
    if received_data:
        log_function("Bytes received from address %s:" % hex(address))
        for byte in received_data:
            log_function(hex(byte))
    log_function("I2C read status: %s" % hex(i2c_status))


def write(device, address, data, log_function):
    i2c_device = I2CDevice(device.i2c, address)
    log_function("Writing to address %s" %  hex(address))
    i2c_status = i2c_device.write(data)
    log_function("I2C write status: %s" % hex(i2c_status))


def scan(device, log_function):
    i2c_bus = I2CBus(device)
    addresses = i2c_bus.scan()

    # list output
    print("I2C address(es):")
    for address, response in enumerate(addresses):
        if response[0]:
            print("%s W" % hex(address))
        if response[1]:
            print("%s R" % hex(address))

    # table output
    print("\n******** W/R bit set at each valid address ********")
    print("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f", end='')
    for i, response in enumerate(addresses):
        if i % 16 == 0:
            print("\n%d0:" % (i / 16), end=' ')
        if response[0]:
            print("W", end='')
        else:
            print("-", end='')
        if response[1]:
            print("R", end=' ')
        else:
            print("-", end=' ')
    print()


if __name__ == '__main__':
    main()
    