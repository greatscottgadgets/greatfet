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
from pygreat.comms import CommandFailureError

# Import all of our known JTAG targets.
from ..targets.jtag import *
from ..interfaces.jtag import JTAGPatternError


def print_chain_info(jtag, log_function, log_error, args):
    """ Command that prints information about devices connected to the scan chain to the console. """

    log_function("Scanning for connected devices...")
    detected_devices = jtag.enumerate()

    # If devices exist on the scan chain, print their information.
    supported_commands = {}
    if detected_devices:
        log_function("{} device{} detected on the scan chain:\n".format(
                    len(detected_devices), 's' if len(detected_devices) > 1 else ''))

        for device in detected_devices:
            log_function("    {:08x} -- {}".format(device.idcode(), device.description()))
            commands = device.supported_console_commands()

            # If the device has any associated console commands, add them to our list.
            if commands:
                supported_commands[device.idcode()] = commands

        log_function('')

        if supported_commands:
            log_function("The following commands are provided for working with these devices:")
            for idcode, commands in supported_commands.items():
                log_function("    {:08x} -- {}".format(idcode, ', '.join(commands)))
        else:
            log_function("No console commands exist for these devices; perhaps 'jtag svf' would be useful?")

        log_function('')

    else:
        log_function("No devices found.\n")


def play_svf_file(jtag, log_function, log_error, args):
    """ Command that prints the relevant flash chip's information to the console. """

    if not args.filename:
        log_error("You must provide an SVF filename to play!\n")
        sys.exit(-1)

    try:
        jtag.play_svf_file(args.filename, log_function=log_function, error_log_function=log_error)
    except JTAGPatternError as e:
        # Our SVF player has already logged the error to stderr.
        log_error("")


def main():

    commands = {
        'scan': print_chain_info,
        'svf': play_svf_file
    }


    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for working with JTAG devices")
    parser.add_argument('command', choices=commands, help='the operation to complete')
    parser.add_argument('filename', metavar="[filename]", nargs='?',
                        help='the filename to read from, for SVF playback')

    args = parser.parse_args()
    device = parser.find_specified_device()

    if args.command == 'scan':
        args.verbose = True
    elif args.filename == "-":
        args.verbose = False

    # Grab our log functions.
    log_function, log_error = parser.get_log_functions()

    # Execute the relevant command.
    command = commands[args.command]
    command(device.jtag, log_function, log_error, args)


if __name__ == '__main__':
    main()
