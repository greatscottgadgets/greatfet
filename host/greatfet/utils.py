#
# This file is part of GreatFET
#
"""
    Utilities that help in writing simple scripts for GreatFET.
"""

from __future__ import print_function

import sys
import ast
import time
import errno
import argparse

from decimal import Decimal

from . import GreatFET, _GreatFETSingletonWrapper
from .boards.flash_stub import GreatFETFlashStub

from pygreat.errors import DeviceNotFoundError


SI_PREFIXES = {
    'E-12': 'p',
    'E-9':  'n',
    'E-6':  'u',
    'E-3':  'm',
    'E+3':  'k',
    'E+6':  'M',
    'E+9':  'G',
    'E+12': 'T',
}


def log_silent(string, end=None):
    """Silently discards all log data, but provides our logging interface."""
    pass


def log_verbose(string, end="\n"):
    """Prints all logging data to the screen."""

    print(string, end=end)
    sys.stdout.flush()


def log_error(string, end="\n"):
    """ Prints errors to stderr. """

    sys.stdout.flush()
    print(string, end=end, file=sys.stderr)
    sys.stderr.flush()


def eng_notation(number, unit=None, separator=' '):
    """ Converts a given number to a nicely-formatted engineering number; so 10e6 would become 10 M."""

    # Grab the raw engineering notation from python's decimal class...
    string = Decimal(number).normalize().to_eng_string()

    # ... and replace the normalized engineering suffix with the relevant SI prefix.
    for normalized, prefix in SI_PREFIXES.items():
        string = string.replace(normalized, separator + prefix)

    if unit is not None:
        string += unit

    return string


def from_eng_notation(string, unit=None, units=None, to_type=None):
    """ Converts a string accepted on the command line (potentially in engineering notation) into a
        python number. """

    # Ensure we have a new list of units accessible to us.
    if units is None:
        units = []
    else:
        units = units[:]

    # If we have a single unit specified, absorb it into our units list.
    if unit is not None:
        units.append(unit)

    # If we have an acceptable unit, strip it off before we process things.
    for unit in units:
        string = string.replace(unit, '')
        string = string.replace(unit.upper(), '')
        string = string.replace(unit.lower(), '')

    # Strip off any unnecessary whitespace.
    string = string.strip()

    # Replace each SI prefix with its normalized value.
    for normalized, prefix in SI_PREFIXES.items():
        if string.endswith(prefix):
            string = string.replace(prefix, '').strip()
            string += normalized
            break

    # Finally, try to parse the string as a python literal.
    result = ast.literal_eval(string)

    # If we have a post-processing function, apply it.
    if callable(to_type):
        result = to_type(result)

    return result


def human_readable_size(byte_count, unit="B", binary_marker='i'):
    """ Converts a number of bytes into a human-readable size string. """

    SUFFIXES = {
        0: "",
        1: "k" + binary_marker,
        2: "M" + binary_marker,
        3: "G" + binary_marker,
        4: "T" + binary_marker,
        5: "P" + binary_marker
    }

    if byte_count is None:
        return 0

    suffix_order =0

    while byte_count >= 1024:
        suffix_order += 1
        byte_count /= 1024

    return "{} {}{}".format(byte_count, SUFFIXES[suffix_order], unit)



class GreatFETArgumentParser(argparse.ArgumentParser):
    """ Convenience-extended argument parser for GreatFET. """

    """ Serial number expected from a device in DFU. """
    DFU_STUB_SERIAL = "dfu_flash_stub"

    def __init__(self, *args, **kwargs):
        """ Sets up a GreatFET-specialized argument parser.

        Additional keyword arguments:
            dfu -- If set to True, DFU-reglated arguments will be provided.
            raise_device_find_failures -- If set to True, this will throw a DeviceNotFoundError
                instead of quitting if no device is present.
        """

        # Determine if we should provide DFU arguments.
        if 'dfu' in kwargs:
            self.supports_dfu = kwargs['dfu']
            del kwargs['dfu']
        else:
            self.supports_dfu = False

        # Determine if we should provide DFU arguments.
        if 'verbose_by_default' in kwargs:
            verbose_by_default = kwargs['verbose_by_default']
            del kwargs['verbose_by_default']
        else:
            verbose_by_default = False

        # If set, this will throw DeviceNotFound errors instead of killing the process.
        if 'raise_device_find_failures' in kwargs:
            self.raise_device_find_failures = kwargs['raise_device_find_failures']
            del kwargs['raise_device_find_failures']
        else:
            self.raise_device_find_failures = False

        # Invoke the core function.
        super(GreatFETArgumentParser, self).__init__(*args, **kwargs)

        # Start off with no memoized arguments.
        self.memoized_args = None

        # By default, log queietly.

        # Add the standard arguments used to find a GreatFET.
        self.add_argument('-s', '--serial', dest='serial', metavar='<serialnumber>', type=str,
                            help="Serial number of device to look for", default=None)
        self.add_argument('-i', '--index', dest='index', metavar='<i>', type=int,
                            help="number of the attached device (default: 0)", default=0)
        self.add_argument('--wait', dest='wait', action='store_true',
                            help="Wait for a GreatFET device to come online if none is found.")

        if verbose_by_default:
            self.add_argument('-q', '--quiet', dest='verbose', action='store_false',
                                help="Don't log details to the console unless an error occurs.")
        else:
            self.add_argument('-v', '--verbose', dest='verbose', action='store_true',
                                help="Log more details to the console.")

        # TODO: specify protocol?
        # TODO: accept comms URI


        # If we're accepting devices from DFU mode, accept the relevant arguments, as well.
        # Note that you must put the device into DFU mode and load the stub from the caller.
        if self.supports_dfu:
            self.add_argument('-d', '--dfu', dest='dfu', action='store_true',
                                help="Access a device from in DFU mode by first loading a stub. Always resets.")
            self.add_argument('--dfu-stub', dest='dfu_stub', metavar='<stub.dfu>', type=str,
                                help="The stub to use for DFU programming. If not provided, the utility will attempt to automtaically find one.")


    def find_specified_device(self):
        """ Connects to the GreatFET specified by the user's command line arguments. """

        device = None
        args = self.parse_args()

        # Loop until we have a device.
        # Conditions where we should abort are presented below.
        while device is None:
            try:
                device = self._find_greatfet(args)
            except DeviceNotFoundError:

                # If we're not in wait mode (or waiting for a DFU flash stub to come up), bail out.
                if not (args.wait or (self.supports_dfu and args.dfu)):

                    # If we're not handling location failures, re-raise the exception.
                    if self.raise_device_find_failures:
                        raise

                    # Otherwise, print a message and bail out.
                    if args.serial:
                        print("No GreatFET board found matching serial '{}'.".format(args.serial), file=sys.stderr)
                    elif args.index:
                        print("No GreatFET board found with index '{}'.".format(args.index), file=sys.stderr)
                    else:
                        print("No GreatFET board found!", file=sys.stderr)
                    sys.exit(errno.ENODEV)
                else:
                    time.sleep(1)

        return device


    def get_singleton_for_specified_device(self):
        """
        Connects to the GreatFET specified by the user's command line arguments, but gets a singleton that persists
        across reconnects.
        """

        # Grab the device itself, and find its serial number.
        device = self.find_specified_device()
        serial = device.serial_number()
        device.close()

        # Create an equivalent singleton wrapper.
        return _GreatFETSingletonWrapper(serial)



    def get_log_function(self):
        """ Returns a function that can be used for logging, but which respects verbosity. """
        return log_verbose if self.parse_args().verbose else log_silent


    def get_log_functions(self):
        """ Returns a 2-tuple of a function that can be used for logging data and errors, attempting to repsect -v/-q."""
        return self.get_log_function(), log_error


    def parse_args(self):
        """ Specialized version of parse_args that memoizes, for GreatFET. """

        # If we haven't called parse_args yet, let the base class handle the parsing,
        # first.
        if self.memoized_args is None:
            self.memoized_args = super(GreatFETArgumentParser, self).parse_args()

        # Always return our memoized version.
        return self.memoized_args


    def _find_greatfet(self, args):
        """ Finds a GreatFET matching the relevant arguments."""

        # If we're programming via DFU mode, look for a device that sports the DFU stub.
        # Note that we only support a single DFU-mode device for now, and thus always
        # grab the first one.
        if self.supports_dfu and args.dfu:
            devices = GreatFET(find_all=True)

            for device in devices:
                if isinstance(device, GreatFETFlashStub):
                    return device

            raise DeviceNotFoundError

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



def greatfet_assets_directory():
    """ Provide a quick function that helps us get at our assets directory. """
    import os

    # Find the path to the module, and then find its assets folder.
    module_path = os.path.dirname(__file__)
    return os.path.join(module_path, 'assets')


def find_greatfet_asset(filename):
    """ Returns the path to a given GreatFET asset, if it exists, or None if the GreatFET asset isn't provided."""
    import os

    asset_path = os.path.join(greatfet_assets_directory(), filename)

    if os.path.isfile(asset_path):
        return asset_path
    else:
        return None
