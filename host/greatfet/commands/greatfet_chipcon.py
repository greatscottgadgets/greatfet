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
from greatfet.programmers.chipcon import ChipconProgrammer


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for chipcon debugging via GreatFET")
    parser.add_argument('-id', '--chip_id', action='store_true',
                        help="Print the chip ID of the connected device.")
    parser.add_argument('-a', '--address', nargs=1, type=ast.literal_eval,
                        help="Flash memory address to begin reading from / writing to")
    parser.add_argument('-r', '--read', type=int,
                        help="Number of bytes to read from flash memory")
    parser.add_argument('-w', '--write_page', nargs='*', type=ast.literal_eval,
                        help="Data to write to the specified flash page")
    parser.add_argument('-p', '--program_flash',
                        help="File name from which data will be written to flash from")
    parser.add_argument('-e', '--erase', default=True,
                        help="Specify whether or not the flash needs to be erased before writing")
    parser.add_argument('-m', '--mass_erase', action='store_true', help="Erase the entire flash memory")
    parser.add_argument('-rp', '--read_page', action='store_true', help="Read on page of flash memory")
    parser.add_argument('-V', '--verify', nargs=1, default=True,
                        help="Specify whether or not to check if the data was flashed correctly")
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

    cc = device.create_programmer('chipcon')

    if args.chip_id:
        chip_id(cc)
    if args.write_page:
        write_flash_page(cc, args.address[0], args.write_page, args.erase)
    if args.program_flash:
        program_flash(cc, args.program_flash, args.erase, args.verify, args.address[0])
    if args.read:
        read_flash(cc, args.address[0], args.read)
    if args.read_page:
        read_flash_page(cc, args.address[0])
    if args.mass_erase:
        erase_flash(cc)


def chip_id(programmer):
    programmer.debug_init()
    print("Chip ID:", programmer.get_chip_id())


def write_flash_page(programmer, linear_address, input_data, erase_page):
    programmer.debug_init()
    programmer.write_flash_page(linear_address, input_data, erase_page=erase_page)


def program_flash(programmer, file_name, erase, verify=True, start_address=0):
    programmer.debug_init()
    f = open(file_name, "rb")
    image_array = f.read()
    programmer.program_flash(image_array, erase=erase, verify=verify, start=start_address)


def read_flash(programmer, start_address=0, length=0):
    programmer.debug_init()
    data = programmer.read_flash(start_address=start_address, length=length)
    print("Flash data received:", data)


def read_flash_page(programmer, start_address=0):
    print(start_address)
    programmer.debug_init()
    data = programmer.read_flash_page(start_address)
    print("Flash page received:", data)


def erase_flash(programmer):
    programmer.debug_init()
    programmer.mass_erase_flash()


if __name__ == '__main__':
    main()
