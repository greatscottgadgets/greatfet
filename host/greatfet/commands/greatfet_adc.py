#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import sys
import time
import argparse

from greatfet.protocol import vendor_requests


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="utility for reading from the GreatFET's ADC")
    parser.add_argument('-f', '--format', dest='format', type=str, default='voltage',
                        choices=['voltage', 'binary', 'raw'],
                        help="Format to output in.\nVoltage string, binary fraction string, or raw binary.")
    parser.add_argument('-n', '--samples', dest='sample_count', type=int, default=1,
                        help="The number of samples to read. Specify 0 to sample continuously. (default: 1)")
    parser.add_argument('-o', '--output', dest='output', type=argparse.FileType('wb'), default='-',
                        help="File to output to. Specify - to output to stdout (default).")

    args         = parser.parse_args()
    log_function = parser.get_log_function()
    device       = parser.find_specified_device()

    if not device.supports_api('adc'):
        sys.stderr.write("This device doesn't seem to support an ADC. Perhaps your firmware needs to be upgraded?\n")
        sys.exit(0)

    # Get the voltage samples from the ADC as a tuple.
    samples = device.adc.read_samples(args.sample_count)

    for sample in samples:

        if args.format == "binary":

            # Output a string representing the binary fraction returned from the ADC.
            args.output.write("{:b}\n".format(sample))

        elif args.format == "raw":

            # Output the binary fraction itself.
            # Change the Python type from integer to string (but not the value itself) so file.write() will accept it.
            args.output.write(chr(sample))

        else:

            # Interpret the binary fraction and output a string representing the returned voltage.
            args.output.write("{}V\n".format((sample / 1024.0) * 3.3))

if __name__ == '__main__':
    main()

