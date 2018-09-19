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

from greatfet.peripherals.i2c_device import I2CDevice
from greatfet.peripherals.i2c_bus import I2CBus
from greatfet_test import *

def main():
    # Set up a simple argument parser.
    parser = argparse.ArgumentParser(description="Utility for flashing the GreatFET's onboard SPI flash")
    parser.add_argument('-s', dest='serial', metavar='<serialnumber>', type=str,
                        help="Serial number of device, if multiple devices", default=None)
    parser.add_argument('-v', dest='verbose', action='store_true', help="Write data from file")
    parser.add_argument('-a', '--address_scan', action='store_true', help="Scan all possible i2c addresses")
    parser.add_argument('-t', '--transmit', dest='address', type=str, help="Transmit data with the given address")
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

    if args.address_scan:
        scan(device)
    if args.address:
        transmit(device, args.address)


def scan(device):
    i2c_bus = I2CBus(device)
    valid_addresses = i2c_bus.scan()                       
    print("Working address(es): ", valid_addresses)

def transmit(device, address, data=[]):
    states = []
    i2c_device = I2CDevice(device.i2c, int(address, 16)>>1)
    # states.append(i2c_device.transmit(data, len(data))) # need to handle len(data) when data=[]
    print("states: ", states)


if __name__ == '__main__':
    main()
