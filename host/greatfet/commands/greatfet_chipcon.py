#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import io
import argparse

from intelhex import IntelHex

from greatfet.utils import GreatFETArgumentParser, log_silent, log_verbose


def int_auto_base(s):
    """
    Allows the user to pass an integer argument on the command line e.g. in decimal, or in hex with 0x notation.

    Used with argparse like `type=int_auto_base`, since argparse's `type` argument accepts any function.
    """

    # base=0 means autodetect the base from the prefix (if any).
    return int(s, base=0)


def main():

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="""Utility for chipcon debugging via GreatFET
                                                    (See /firmware/common/swra.c for pin mappings)""",
                                    verbose_by_default=True)
    parser.add_argument('--chip-id', action='store_true', # Short options (one dash) should always be one letter
                        help="Print the chip ID of the connected device.")
    parser.add_argument('-a', '--address', dest='address', metavar='<n>', type=int_auto_base,
                        help="Starting address (default: 0)", default=0)
    parser.add_argument('-l', '--length', dest='length', metavar='<n>', type=int_auto_base,
                        help="Length of data to read")
    parser.add_argument('-r', '--read', metavar='<filename>', type=argparse.FileType('wb'),
                        help="Read data into file")
    parser.add_argument('--no-erase', dest='erase', default=True, action='store_false',
                        help="Do not erase the flash before performing a write operation")
    parser.add_argument('--no-verify', dest='verify', action='store_false', default=True,
                        help="Do not verify the flash after performing a write operation")
    parser.add_argument('-E', '--mass-erase', action='store_true', help="Erase the entire flash memory")
    parser.add_argument('-w', '--write', metavar='<filename>', type=argparse.FileType('rb'),
                        help="Write data from file")
    parser.add_argument("--bin", action='store_true', default=False,
                        help="Disable Intel hex detection and flash as-is.")

    args = parser.parse_args()

    log_function = log_verbose if args.verbose else log_silent

    device = parser.find_specified_device()

    chipcon = device.create_programmer('chipcon')

    chipcon.debug_init()

    if args.chip_id:
        chip_id(chipcon)

    if args.read:
        if not args.length:
            parser.error("argument -s/--length: expected one argument")
        read_flash(chipcon, args.read, args.address, args.length, log_function)

    if args.mass_erase:
        mass_erase_flash(chipcon, log_function)

    if args.write:
        program_flash(chipcon, args.write, args.address, args.erase, args.verify, args.bin, log_function)


def chip_id(programmer):
    print("Chip ID:", programmer.get_chip_id())


def read_flash(programmer, out_file, start_address, length, log_function):
    log_function("Reading {} bytes starting at address {:02x}...".format(length, start_address))
    data = programmer.read_flash(start_address=start_address, length=length)
    out_file.write(data)


def mass_erase_flash(programmer, log_function):
    log_function("Erasing entire flash...")
    programmer.mass_erase_flash()


def program_flash(programmer, in_file, start_address, erase, verify, force_bin, log_function):
    image_array = in_file.read()

    if image_array.startswith(b':') and not force_bin:

        print("File type detected as Intel Hex -- converting to binary before flashing...")
        print("To force flashing as-is, pass --bin.")

        # HACK: The IntelHex class expects a filename or file-like object, but...we've already read the file.
        # So let's wrap it in a file-like object. Normally, we'd use io.BytesIO,
        # except IntelHex wants strings, not bytes objects. So...
        intel_hex = IntelHex(io.StringIO(image_array.decode('ascii')))
        image_array = intel_hex.tobinstr()


    log_function("Writing data to flash...")
    programmer.program_flash(image_array, erase=erase, verify=verify, start=start_address)


if __name__ == '__main__':
    main()
