#!/usr/bin/env python
#
# This file is part of GreatFET
#
"""
    Utility for flashing firmware on GreatFET boards.
"""

from __future__ import print_function

import os
import sys
import errno
import subprocess

from tqdm import tqdm

from fwup.lpc43xx import LPC43xxTarget
from fwup.errors import BoardNotFoundError

from greatfet import find_greatfet_asset
from greatfet.errors import DeviceNotFoundError
from greatfet.utils import log_silent, GreatFETArgumentParser

# Vendor VID/PID if the device is in DFU.
NXP_DFU_VID = 0x1fc9
NXP_DFU_PID = 0x000c

# Maximum length to allow, for now.
MAX_FLASH_LENGTH = 0x100000

def spi_flash_read(device, filename, address, length, log_function=log_silent):
    """Reads the data from the device's SPI flash to a file. """

    silent    = (log_function == log_silent)
    page_size = device.onboard_flash.page_size
    total_data = length if length else (device.onboard_flash.maximum_address + 1)

    # Read the data from the board's SPI flash to a file.
    with tqdm(total=total_data, ncols=80, unit='B', leave=False, disable=silent) as progress:
        flash_data = device.onboard_flash.dump(filename, address, length, auto_truncate=(length is None),
                progress_callback=lambda offset, len : progress.update(page_size))

    log_function('')


def spi_flash_write(device, filename, address, log_function=log_silent):
    """Writes the data from a given file to the SPI flash."""

    silent     = (log_function == log_silent)
    page_size  = device.onboard_flash.page_size
    total_data = os.path.getsize(filename)

    # Read the data from the board's SPI flash to a file.
    with tqdm(total=total_data, ncols=80, unit='B', leave=False, disable=silent) as progress:
        device.onboard_flash.upload(filename, address, erase_first=True,
                progress_callback=lambda offset,  len : progress.update(page_size))
    log_function('')


def dfu_upload(dfu_target, filename, log_function=log_silent):
    """ Uploads the given filename to the device via DFU. """

    silent = (log_function == log_silent)
    total_data = os.path.getsize(filename)

    # This container essentially creates a nonlocal context that's py2 compatible. :)
    class progress_context:
        last_update = 0

    def update_progress(offset, total):
        """ Callback to update our progress bar in non-silent mode. """

        # Figure out how much we've advanced, and update the progress bar.
        delta = offset - progress_context.last_update
        progress.update(delta)

        # And store the last update, so we can continue providing deltas.
        progress_context.last_update = offset


    with open(filename, 'rb') as f:
        data = f.read()

    with tqdm(total=total_data, ncols=80, unit='B', leave=False, disable=silent) as progress:
        dfu_target.program(data, update_progress)

    log_function('')
    log_function('Firmware uploaded but not flashed; changes will not persist post-reset!')
    log_function('')



def load_dfu_stub(dfu_stub_path):
    """ Loads the DFU stub onto the board for DFU-based programming. """

    try:
        dfu_target = LPC43xxTarget()
    except BoardNotFoundError:
        raise DeviceNotFoundError

    # If we have a DFU'able device, find the DFU stub and load it.
    stub_path = dfu_stub_path
    if stub_path is None:
        raise ValueError("Could not find the DFU stub!")

    # Read the DFU stub into memory...
    with open(dfu_stub_path, "rb") as f:
        dfu_stub = f.read()

    # ... and program it to the board.
    dfu_target.program(dfu_stub)



def main():

    # Grab any GreatFET assets that should have shipped with the tool.
    dfu_stub_path = find_greatfet_asset('flash_stub.bin')
    auto_firmware_path = find_greatfet_asset("greatfet_usb.bin")

    # Set up a simple argument parser.-
    parser = GreatFETArgumentParser(dfu=True, verbose_by_default=True,
        description="Utility for flashing firmware on GreatFET boards")
    parser.add_argument('-a', '--address', metavar='<n>', type=int,
                        help="starting address (default: 0)", default=0)
    parser.add_argument('-l', '--length', metavar='<n>', type=int, default=None,
                        help="number of bytes to read; if not specified, we try to read the programmed sections")
    parser.add_argument('-r', '--read', dest='read', metavar='<filename>', type=str,
                        help="Read data into file", default='')
    parser.add_argument('-w', '--write', dest='write', metavar='<filename>', type=str,
                        help="Write data from file", default='')
    parser.add_argument('-R', '--reset', dest='reset', action='store_true',
                        help="Reset GreatFET after performing other operations.")
    parser.add_argument('-V', '--volatile-upload', dest='volatile', metavar="<filename>", type=str,
                        help="Uploads a GreatFET firmware image to RAM via DFU mode. Firmware is not flashed.")

    # If we have the ability to automatically install firmware, provide that as an option.
    if auto_firmware_path:
        parser.add_argument('--autoflash', action='store_true', dest='autoflash',
                        help="Automatically flash the attached board with the firmware corresponding to the installed tools.")
        parser.add_argument('-U', '--volatile-upload-auto', dest='volatile_auto', action='store_true',
                            help="Automatically upload the tools' firmware via DFU mode. Firmware is not flashed.")

    args = parser.parse_args()

    # If we're trying to automatically flash the given firmware, set the relevant options accordingly.
    try:
        if not args.write and args.autoflash:
            args.write = auto_firmware_path
            args.reset = True
    except AttributeError:
        pass

    try:
        if not args.volatile and args.volatile_auto:
            args.volatile = auto_firmware_path
    except AttributeError:
        pass

    # Validate our options.

    # If we don't have an option, print our usage.
    if not any((args.read, args.write, args.reset, args.volatile)):
        parser.print_help()
        sys.exit(0)

    # Determine whether we're going to log to the stdout, or not at all.
    log_function = parser.get_log_function()

    if args.dfu_stub:
        dfu_stub_path = args.dfu_stub


    # If we're uploading a file via DFU for a "volatile" flash, do so and abort.
    if args.volatile:

        try:
            device = LPC43xxTarget()
        except BoardNotFoundError:
            print("Couldn't find a GreatFET-compatible board in DFU mode!", file=sys.stderr)
            sys.exit(errno.ENODEV)

        log_function("Uploading data to RAM...\n")
        dfu_upload(device, args.volatile, log_function)
        sys.exit(0)


    # If we're supposed to install firmware via a DFU stub, install it first.
    if args.dfu:
        try:
            load_dfu_stub(dfu_stub_path)
        except DeviceNotFoundError:
            print("Couldn't find a GreatFET-compatible board in DFU mode!", file=sys.stderr)
            sys.exit(errno.ENODEV)


    # Create our GreatFET connection.
    log_function("Trying to find a GreatFET device...")
    device = parser.find_specified_device()
    log_function("{} found. (Serial number: {})".format(device.board_name(), device.serial_number()))


    # Ensure that the device supports an onboard SPI flash.
    try:
        device.onboard_flash
    except AttributeError:
        print("The attached GreatFET ({}) doesn't appear to have an SPI flash to program!".format(device.board_name()), file=sys.stderr)
        sys.exit(errno.ENOSYS)

    # If we have a write command, write first, to match the behavior of hackrf_spiflash.
    if args.write:
        log_function("Writing data to SPI flash...\n")
        spi_flash_write(device, args.write, args.address, log_function)
        log_function("Write complete!")
        if not (args.reset or args.dfu):
            log_function("Reset not specified; new firmware will not start until next reset.")

    # Handle any read commands.
    if args.read:
        log_function("Reading data from SPI flash...\n")
        spi_flash_read(device, args.read, args.address, args.length, log_function)
        log_function("Read complete!")

    # Finally, reset the target
    if args.reset or args.dfu:
        log_function("Resetting GreatFET...")
        device.reset(reconnect=False, is_post_firmware_flash=bool(args.write))
        log_function("Reset complete!")

if __name__ == '__main__':
    main()
