#
# This file is part of GreatFET
#
"""
    Utilities that help in wirting simple scripts for GreatFET.
"""

from __future__ import print_function

import sys
import time
import errno
import argparse

from greatfet import GreatFET
from greatfet.boards.flash_stub import GreatFETFlashStub

from pygreat.errors import DeviceNotFoundError

def log_silent(string, end=None):
    """Silently discards all log data, but provides our logging interface."""
    pass

def log_verbose(string, end="\n"):
    """Prints all logging data to the screen."""
    print(string, end=end)


class GreatFETArgumentParser(argparse.ArgumentParser):
    """ Convenience-extended argument parser for GreatFET. """

    """ Serial number expected from a device in DFU. """
    DFU_STUB_SERIAL = "dfu_flash_stub"

    def __init__(self, *args, **kwargs):
        """ Sets up a GreatFET-specialized argument parser.

        Additional keyword arguments:
            dfu -- If set to True, DFU-reglated arguemnts will be provided.
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

                    # If we're not handling locaiton failures, re-raise the exception.
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


    def get_log_function(self):
        """ Returns a function that can be used for logging, but which respects verbosity. """
        return log_verbose if self.parse_args().verbose else log_silent


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

        # If we're prorgamming via DFU mode, look for a device that sports the DFU stub.
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
