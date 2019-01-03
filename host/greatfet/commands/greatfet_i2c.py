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
    parser.add_argument('-z', '--scan', action='store_true', help="Scan all possible i2c addresses")
    parser.add_argument('-a', '--address', nargs=1, type=ast.literal_eval, help="Address to transmit data to over the I2C Bus") 
    parser.add_argument('-r', '--receive_length', default=0, help="Number of bytes expecting to receive from the I2C Bus")
    parser.add_argument('data_to_transmit', nargs='*', type=ast.literal_eval, default=[], help="Data to transmit over the I2C Bus (in bytes)")
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
    if args.address:
        transmit(device, args.address[0], args.data_to_transmit, int(args.receive_length), log_function)


def scan(device, log_function):
    i2c_bus = I2CBus(device)
    valid_addresses = i2c_bus.scan()
    print("Working address(es):")
    for address in valid_addresses:
        print(hex(address))
    

def transmit(device, address, data, receive_length, log_function):
    i2c_device = I2CDevice(device.i2c, address >> 1)
    received_data, i2c_status = i2c_device.transmit(data, receive_length)
    if received_data:
        log_function("received bytes:")
        for byte in received_data:
            log_function(hex(byte))
    log_function("i2c status:")
    log_function(hex(i2c_status))


if __name__ == '__main__':
    main()
    