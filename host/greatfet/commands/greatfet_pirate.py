#!/usr/bin/env python3
# This file is part of GreatFET.
#
""" Bus Pirate emulation for GreatFET. """

from __future__ import print_function

import errno
import sys

import greatfet
from greatfet import GreatFET
from greatfet.utils import GreatFETArgumentParser


def main():
    from greatfet.utils import GreatFETArgumentParser

    supported_interfaces = {
        'spi': lambda d : d.spi
    }
    supported_interface_string = ', '.join(supported_interfaces)

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="bus pirate emulation utility for GreatFET", verbose_by_default=True)
    parser.add_argument('interface', nargs='?', help="The type of interface to use for our commands. Currently supported: {}".format(supported_interface_string))
    parser.add_argument('commands', nargs='*', default=[], help="Bus pirate command to execute.")
    args = parser.parse_args()

    log_function, log_error = parser.get_log_functions()

    # FIXME: properly support interactive mode
    if args.interface is None:
        log_error("Interactive mode not yet supported.")
        sys.exit(-1)

    # Grab our device, and interface.
    device = parser.find_specified_device()
    interface = supported_interfaces[args.interface](device)

    # Issue any commands we have...
    commands = ''.join(args.commands)
    log_function("> {}".format(commands))
    results = interface.run_pirate_commands(commands)

    # ... and by default, print the results to the console.
    log_function(results)


if __name__ == '__main__':
    main()
