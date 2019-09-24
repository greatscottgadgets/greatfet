#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import errno
import sys

import greatfet
from greatfet import GreatFET
from greatfet.utils import log_silent, log_verbose


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for experimenting with GreatFET's DAC", verbose_by_default=True)
    parser.add_argument('-f', '--format', dest='format', type=str, default='voltage',
                        choices=['voltage', 'raw'],
                        help="Format for the input.\nVoltage string, or binary value to be loaded into the DAC.")
    parser.add_argument('value', metavar='[value]', type=float,
                        help="The desired voltage (default) or raw value to load into DAC (with -f raw).")

    args         = parser.parse_args()
    log_function = parser.get_log_function()
    device       = parser.find_specified_device()

    device.apis.dac.initialize()

    if args.format == "voltage":

        # Voltage must be passed to the device in millivolts, so * 1000.
        device.apis.dac.set_voltage(int(args.value * 1000))
        log_function("DAC set to {} volts".format(args.value))

    else:

        device.apis.dac.set_value(int(args.value))
        log_function("DAC set to {}".format(int(args.value)))


if __name__ == '__main__':
    main()
