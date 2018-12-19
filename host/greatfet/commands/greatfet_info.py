#!/usr/bin/env python
#
# This file is part of GreatFET

from __future__ import print_function

import textwrap
import argparse
import inspect
import errno
import sys

from greatfet import GreatFET

from pygreat.comms import GeneratedCommsClass

def print_core_info(device):
    """ Prints the core information for a device. """

    print("Found a {}!".format(device.board_name()))
    print("  Board ID: {}".format(device.board_id()))
    print("  Firmware version: {}".format(device.firmware_version()))
    print("  Part ID: {}".format(device.part_id()))
    print("  Serial number: {}".format(device.serial_number()))

    # If this board has any version warnings to display, dipslay them.
    warnings = device.version_warnings()
    if warnings:
        wrapped_warnings = textwrap.wrap(warnings)
        wrapped_warnings = "\n".join(["    {}".format(line) for line in wrapped_warnings])
        print("\n  !!! WARNING !!!\n{}\n".format(wrapped_warnings))


def print_apis(device):
    """ Prints a human-readable summary of the device's provided APIs. """

    print("  APIs supported:")

    # Print each of the supported APIs.
    for api_name in device.comms.apis:
        printed = False

        # Get a shortcut to the provided RPC API.
        api = device.comms.apis[api_name]
        print("    {}:".format(api.CLASS_NAME))

        # Print all methods on the given API.
        methods = inspect.getmembers(api, inspect.ismethod)


        # Otherwise, print all of the methods.
        for method_name, method in methods:

            # Don't print private-API methods.
            if method_name.startswith('_'):
                continue

            # Don't print inherited methods.
            if hasattr(GeneratedCommsClass, method_name):
                continue

            printed = True

            # Extract a summary for our view, and print it.
            # TODO: base the amount printed on the terminal size
            method_docs = inspect.getdoc(method)
            method_first_line = method_docs.split("\n")
            method_summary = method_first_line[0][0:60]
            print("      {} -- {}".format(method_name, method_summary))

        # If we had nothing to print for the class,
        if not printed:
            print("      <no introspectable methods>")






def main():

    # Set up a simple argument parser.
    parser = argparse.ArgumentParser(
        description="Utility for  gathering information about connected GreatFETs")
    parser.add_argument('-A', '--api', dest='print_apis', action='store_true',
                        help="Print information about each device's supported APIs.")
    parser.add_argument('-a', '--all', dest='print_all', action='store_true',
                        help="Print all available information about the device.")
    parser.add_argument('-q', '--quiet', dest='quiet', action='store_true',
                        help="Prints only the serial numbers of detected GreatFETs")
    args = parser.parse_args()

    # Try to find all existing devices
    devices = GreatFET(find_all=True)
    if not devices:
        print('No GreatFET devices found!', file=sys.stderr)
        sys.exit(errno.ENODEV)

    # Print the board's information...
    for device in devices:

        # If we're in quiet mode, print only the serial number and abort.
        if args.quiet:
            print(device.serial_number())
            continue

        # Otherwise, print the core information.
        print_core_info(device)

        # If desired, print all APIs.
        if args.print_apis or args.print_all:
            print_apis(device)

        print(" ")



if __name__ == '__main__':
    main()
