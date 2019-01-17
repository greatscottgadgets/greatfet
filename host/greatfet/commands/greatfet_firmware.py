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

import usb

from greatfet import GreatFET
from greatfet.errors import DeviceNotFoundError
from greatfet.utils import log_silent, GreatFETArgumentParser

# The serial number expected from the DFU flash stub.
DFU_STUB_NAME  = 'flash_stub.dfu'
DFU_STUB_PATHS = [ '~/.local/share/greatfet', '~/.local/share/GreatFET' ]

# Vendor VID/PID if the device is in DFU.
NXP_DFU_VID = 0x1fc9
NXP_DFU_PID = 0x000c

# Maximum length to allow, for now.
MAX_FLASH_LENGTH = 0x100000

def spi_flash_read(device, filename, address, length, log_function=log_silent):
    """Reads the data from the device's SPI flash to a file. """

    def print_progress(bytes_read, bytes_total):
        log_function("Read {} bytes of {}.".format(bytes_read, bytes_total), end='\r')

    # Read the data from the board's SPI flash to a file.
    with open(filename, 'wb') as f:
        flash_data = device.onboard_flash.read(address, length,
                                               progress_callback=print_progress)
        flash_data.tofile(f)
    log_function('')


def spi_flash_write(device, filename, address, log_function=log_silent):
    """Writes the data from a given file to the SPI flash."""

    def print_progress(bytes_written, bytes_total):
        log_function("Written {} bytes of {}.".format(bytes_written, bytes_total), end='\r')

    # Read the data from the board's SPI flash to a file.
    with open(filename, 'rb') as f:
        flash_data = f.read()
        device.onboard_flash.write(flash_data, address,
                                   erase_first=True,
                                   progress_callback=print_progress)
    log_function('')


def find_dfu_stub(args):
    """ Finds the DFU stub. """

    # FIXME: This should be cleaned up to search paths that make sense given
    # where and how we might install GreatFET.

    # If we have an explicit DFU stub location, use it.
    if args.dfu_stub:
        path = os.path.expanduser(args.dfu_stub)

        if os.path.isfile(path):
            return path

    # Otherwise, search each of the paths around.
    for path in DFU_STUB_PATHS:
        filename = os.path.expanduser(os.path.join(path, DFU_STUB_NAME))
        print(filename)

        if os.path.isfile(filename):
            return filename

    # If we weren't able to find it, give up, for now.
    # TODO: eventually ship this with the GreatFET distribution and/or
    # download it on demand?
    return None



def load_dfu_stub(args):
    """ Loads a DFU programming stub onto a GreatFET in DFU mode. """

    # First: check to make sure we _have_ a DFU'able device.
    dev = usb.core.find(idVendor=NXP_DFU_VID, idProduct=NXP_DFU_PID)
    if not dev:
        raise DeviceNotFoundError
    del dev

    # If we have a DFU'able device, find the DFU stub and load it.
    stub_path = find_dfu_stub(args)
    if stub_path is None:
        raise ValueError("Could not find the DFU stub!")

    #
    # FIXME: This isn't a good way to do things. It's being stubbed in
    # for now, but it'd be better to talk DFU from python directly.
    #
    rc = subprocess.call(['dfu-util', '--device', format(NXP_DFU_VID, 'x'), format(NXP_DFU_PID, 'x'), '--alt', '0', '--download', stub_path])
    if rc:
        raise IOError("Error using DFU-util!")



def find_greatfet(args):
    """ Finds a GreatFET matching the relevant arguments."""

    # If we're prorgamming via DFU mode, look for a device that sports the DFU stub.
    # Note that we only support a single DFU-mode device for now.
    if args.dfu:
        return GreatFET(serial_number=DFU_STUB_SERIAL)

    # If we have an index argument, grab _all_ greatFETs and select by index.
    elif args.index:
        # Find _all_ GreatFETs...
        devices = GreatFET(find_all=True)

        # ... and then select the one with the provided index.
        if len(devices) <= args.index:
            raise DeviceNotFoundError
        return devices[args.index]

    # If we have a serial number, look only for a single device. Theoretically,
    # we should never have more than one GreatFET with the same serial number.
    # Technically, this is violable, but libusb doesn't properly handle searching
    # by serial number if there are multiple devices with the same one, so we
    # enforce this.
    else:
        return GreatFET(serial_number=args.serial)



def main():
    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(dfu=True,
        description="Utility for flashing firmware on GreatFET boards")
    parser.add_argument('-a', '--address', metavar='<n>', type=int,
                        help="starting address (default: 0)", default=0)
    parser.add_argument('-l', '--length', metavar='<n>', type=int,
                        help="number of bytes to read (default: {})".format(MAX_FLASH_LENGTH),
                        default=MAX_FLASH_LENGTH)
    parser.add_argument('-r', '--read', dest='read', metavar='<filename>', type=str,
                        help="Read data into file", default='')
    parser.add_argument('-w', '--write', dest='write', metavar='<filename>', type=str,
                        help="Write data from file", default='')
    parser.add_argument('-R', '--reset', dest='reset', action='store_true',
                        help="Reset GreatFET after performing other operations.")
    args = parser.parse_args()

    # Validate our options.

    # If we don't have an option, print our usage.
    if not any((args.read, args.write, args.reset,)):
        parser.print_help()
        sys.exit(0)

    # Determine whether we're going to log to the stdout, or not at all.
    log_function = parser.get_log_function()

    # If we're supposed to install firmware via a DFU stub, install it first.
    if args.dfu:
        try:
            load_dfu_stub(args)
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
        log_function("Writing data to SPI flash...")
        spi_flash_write(device, args.write, args.address, log_function)
        log_function("Write complete!")
        if not (args.reset or args.dfu):
            log_function("Reset not specified; new firmware will not start until next reset.")

    # Handle any read commands.
    if args.read:
        log_function("Reading data from SPI flash...")
        spi_flash_read(device, args.read, args.address, args.length, log_function)
        log_function("Read complete!")

    # Finally, reset the target
    if args.reset or args.dfu:
        log_function("Resetting GreatFET...")
        device.reset(reconnect=False, is_post_firmware_flash=bool(args.write))
        log_function("Reset complete!")

if __name__ == '__main__':
    main()
