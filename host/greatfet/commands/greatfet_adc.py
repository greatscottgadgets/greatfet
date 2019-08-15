#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import sys
import time
import argparse

from greatfet.protocol import vendor_requests


def output_sample(sample, args):

    # We have to write string strings (and not binary strings) to stdout as argparse ignores the binary mode specifier
    # for stdout and stdin, until https://github.com/python/cpython/pull/13165 is merged.

    if args.format == "raw":

        # Output the binary fraction itself as an integer.
        args.output.write("{}\n".format(sample))

    else:

        # Interpret the binary fraction and output a string representing the returned voltage.
        if args.machine_readable:
            args.output.write("{}\n".format((sample / 1024.0) * 3.3))
        else:
            args.output.write("{}V\n".format((sample / 1024.0) * 3.3))


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="utility for reading from the GreatFET's ADC")
    parser.add_argument('-f', '--format', dest='format', type=str, default='voltage',
                        choices=['voltage', 'raw'],
                        help="Format to output in.\nVoltage string, or raw fraction returned by the ADC.")
    parser.add_argument('-m', '--machine-readable', dest='machine_readable', action='store_true',
                        default=False, help="Don't output unit suffixes.")
    parser.add_argument('-n', '--samples', dest='sample_count', type=int, default=1,
                        help="The number of samples to read. (default: 1)")
    parser.add_argument('-o', '--output', dest='output', type=argparse.FileType('w'), default='-',
                        help="File to output to. Specify - to output to stdout (default).")

    args         = parser.parse_args()
    log_function = parser.get_log_function()
    device       = parser.find_specified_device()

    if not device.supports_api('adc'):
        sys.stderr.write("This device doesn't seem to support an ADC. Perhaps your firmware needs to be upgraded?\n")
        sys.exit(0)

    samples = device.adc.read_samples(args.sample_count)

    for sample in samples:
        output_sample(sample, args)

    args.output.close()

if __name__ == '__main__':
    main()

