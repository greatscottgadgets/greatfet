#!/usr/bin/env python3
#
# This file is part of GreatFET
#

from __future__ import print_function

import os
import sys
import errno
import argparse

from tqdm import tqdm


from greatfet import GreatFET
from greatfet.errors import DeviceNotFoundError
from greatfet.utils import from_eng_notation, human_readable_size, GreatFETArgumentParser
from greatfet.programmers.spi_flash import SPIFlash

from pygreat.comms import CommandFailureError



def print_flash_info(spi_flash, log_function, log_error, args):
    """ Function that prints the relevant flash chip's information to the console. """

    capacity = spi_flash.maximum_address + 1

    if args.bypass_jedec:
        log_function("Taking it on faith that an SPI flash is there.")
    else:
        log_function("SPI flash detected:")

    log_function("\tManufacturer: {} (0x{:02x})".format(spi_flash.manufacturer, spi_flash.manufacturer_id))
    log_function("\tPart number: {} (0x{:02x})".format(spi_flash.part, spi_flash.part_id))
    log_function("\tCapacity: {} ({})".format(human_readable_size(capacity),
        human_readable_size(capacity * 8, unit='b')))
    log_function("\tPage size: {} B".format(spi_flash.page_size))
    log_function('')


def log_summary(log_function, spi_flash):
    """ Print a quick summary of the detected flash; for use in non-info commands. """

    if spi_flash.manufacturer_id in SPIFlash.JEDEC_MANUFACTURERS:
        log_function("Detected a {} {} ({}).".format(spi_flash.manufacturer, spi_flash.part,
            human_readable_size(spi_flash.maximum_address + 1)))
    else:
        log_function("Detected an unknown flash chip.")


def be_cautious(args, log_error, operation="write to"):
    """ Enforces the --cowardly option, which refuses to do dangerous things. """

    if args.cowardly or os.getenv('BE_COWARDLY'):
        log_error("Cowardly refusing to {} flash chip. [You provided -C.]".format(operation))
        log_error("")
        sys.exit(-3)


def erase_chip(spi_flash, log_function, log_error, args):
    """ Erase the provided SPI flash. """

    log_summary(log_function, spi_flash)
    be_cautious(args, log_error, operation="erase")

    spi_flash.erase()
    log_function("Erase complete.")


def run_flash_operation(operation_function, page_size, total_data, args, *pargs, **kwargs):
    """ Runs a read or write operation, while showing nice progress bars when appropriate. """

    with tqdm(total=total_data, ncols=80, unit='B', leave=False, disable=not args.verbose) as progress:
        kwargs['progress_callback'] = lambda written, total : progress.update(page_size)
        operation_function(*pargs, **kwargs)


def dump_chip(spi_flash, log_function, log_error, args):
    """ Dump the contents of the provided chip. """

    log_summary(log_function, spi_flash)

    # Figure out how much we're going to read.
    length_to_read = args.length if args.length else (spi_flash.maximum_address + 1)

    # Truncate the read size to the flash's length, if necessary.
    flash_size = spi_flash.maximum_address + 1
    if (args.address + length_to_read) > flash_size:
        length_to_read = flash_size - args.address

        log_error("This operation would read past the end of flash -- truncating to {}.".format(
            human_readable_size(length_to_read)))

    if args.filename is None:
        log_error("You must provide a filename to write the captured data to!\n")
        sys.exit(-1)


    # Finally, run the flash operation.
    try:
        log_function("Reading {}.\n".format(human_readable_size(length_to_read)))
        run_flash_operation(spi_flash.dump, spi_flash.page_size, length_to_read, args,
            args.filename, args.address, length_to_read, auto_truncate=args.truncate)
        log_function("Read complete.\n")
    except FileNotFoundError:
        log_error("Cannot open {} for writing.\n".format(args.filename))



def program_chip(spi_flash, log_function, log_error, args):
    """ Programs data to the given chip. """

    log_summary(log_function, spi_flash)
    be_cautious(args, log_error, operation="erase")

    if args.filename is None:
        log_error("You must provide a filename that will provided the data to write!\n")
        sys.exit(-1)

    try:
        # Grab the size of the file to read.
        if args.length is None:
            length_to_write = None if args.filename == '-' else os.path.getsize(args.filename)

        # Check that we can write the relevant file into flash.
        flash_size = spi_flash.maximum_address + 1
        if length_to_write and ((length_to_write + args.address) > flash_size):
            log_error("This operation would write past the end of flash. If you'd like to trim your file, try --length.\n")
            sys.exit(-3)

        log_function("Writing {}.\n".format(human_readable_size(length_to_write)))
        run_flash_operation(spi_flash.upload, spi_flash.page_size, length_to_write,
            args, args.filename, args.address, length_to_write, erase_first=args.autoerase)
        log_function("Write complete.\n")

    except FileNotFoundError:
        log_error("Cannot open {} for reading.\n".format(args.filename))


def main():

    commands = {
        'info': print_flash_info,
        'erase': erase_chip,
        'read': dump_chip,
        'write': program_chip
    }


    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for programming and dumping SPI flash chips",
                                    verbose_by_default=True)
    parser.add_argument('command', choices=commands, help='the operation to complete')
    parser.add_argument('filename', metavar="[filename]", nargs='?',
                        help='the filename to read or write to, for read/write operations')

    parser.add_argument('-a', '--address', metavar='<addr>', default=0,
                        type=from_eng_notation, help="Starting offset in the given flash memory.")
    parser.add_argument('-l', '--length', metavar='<length>', type=from_eng_notation,
                        default=None, help="number of bytes to read (default: flash size)")
    parser.add_argument('-E', '--no-autoerase', action='store_false', dest='autoerase',
                        help="If provided, the target flash will not be erased before a write operation.")
    parser.add_argument('-C', '--cautious', '--cowardly', action='store_true', dest='cowardly',
                        help="Refuses to do anything that might overwrite or lose chip data.")
    parser.add_argument('-S', '--no-spdf', dest='autodetect', action='store_false',
                        help="Don't attempt to use SPDF to autodetect flash parameters.")
    parser.add_argument('-R', '--require-spdf', dest='allow_fallback', action='store_false',
                        help="Only use SPDF; ignore any argument provided as fall-back options.")
    parser.add_argument('-T', '--auto-truncate', dest='truncate', action='store_true',
                        help="If provided, any read operations will truncate trailing unprogrammed words (0xFFs).")
    parser.add_argument('--page-size', metavar='<bytes>', type=int, default=256,
                        help="manually specify the page size of the target flash; for use with -S")
    parser.add_argument('--flash-size', metavar='<bytes>', type=int, default=8192,
                        help="manually specify the capacity of the target flash; for use with -S")
    parser.add_argument('-J', '--allow-null-jedec-id', dest='bypass_jedec', action='store_true',
                        help="Allow the device to work even if it doesn't appear to support a JEDEC ID.")


    args = parser.parse_args()
    device = parser.find_specified_device()

    if args.command == 'info':
        args.verbose = True
    elif args.filename == "-":
        args.verbose = False

    # Grab our log functions.
    log_function, log_error = parser.get_log_functions()

    try:
        # TODO: use a GreatFET method to automatically instantiate spi_flash

        # Figure out the "override" page and flash size for any arguments provided.
        # If autodetection is enabled and works, these aren't used.
        maximum_address = args.flash_size - 1
        num_pages = (args.flash_size + (args.page_size - 1)) // args.page_size

        # Create a SPI flash object.
        spi_flash = device.create_programmer('spi_flash', args.autodetect, args.allow_fallback,
            page_size=args.page_size, maximum_address=maximum_address, allow_null_jedec=args.bypass_jedec)

        # If we have a device that's ancient enough to not speak JEDEC, notify the user. Pithily.
        if args.bypass_jedec and spi_flash.manufacturer_id == 0xff and spi_flash.part_id == 0xffff:
            log_function("I... really can't see an SPI flash here. I'll trust you.")


    except CommandFailureError:
        log_error("This device doesn't appear to support identifying itself.")
        log_error(" You'll need to specify the page and flash size manually.")
        sys.exit(-1)
    except IOError as e:
        log_error(str(e))
        log_error("If you believe you have a device properly connected, you")
        log_error(" may want to try again with --allow-null-jedec-id.")
        log_error("")
        sys.exit(-2)

    command = commands[args.command]
    command(spi_flash, log_function, log_error, args)


if __name__ == '__main__':
    main()
