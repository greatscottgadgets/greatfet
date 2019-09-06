#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import argparse
import errno
import sys
import time
import os
import array
import tempfile
import threading

# Temporary?
import usb

from zipfile import ZipFile

import greatfet

from greatfet import GreatFET
from greatfet.interfaces.pattern_generator import PatternGenerator
from greatfet.utils import GreatFETArgumentParser, eng_notation, from_eng_notation, log_silent, log_error


def generate_counter(args, pattern_gen):
    """
        Generates a simple binary counter.
    """

    # Ensure our counter naturally "rolls over" after the bus width.
    modulus = 2 ** args.bus_width

    # Generate samples for our counter, and return them as a collection of bytes.
    raw_samples = [i % modulus for i in range(args.samples)]
    return bytes(raw_samples)


def print_debugging(args, pattern_gen):
    print(pattern_gen.dump_sgpio_config())


def halt_generation(args, pattern_gen):
    pattern_gen.stop()


commands = {
    'counter': generate_counter,
    'debug' : print_debugging,
    'stop': halt_generation,
}

def main():

    # Simple type-arguments for parsing.
    int_from_msps = lambda x : from_eng_notation(x, units=['Hz', 'SPS'], to_type=int)
    int_from_eng  = lambda x : from_eng_notation(x, to_type=int)

    # Set up our argument parser.
    parser = GreatFETArgumentParser(description="Logic analyzer implementation for GreatFET", verbose_by_default=True)
    parser.add_argument('command', choices=commands, help='the pattern shape to generate, or command to execute')
    parser.add_argument('-n', '--samples', metavar='samples', type=int_from_eng, default=64,
                         dest='samples', help='the number of samples to generate of the given pattern, up to 32K')
    parser.add_argument('-w', '--width', metavar='bus_width', type=int, default=8,
                         dest='bus_width', help='the width of the bus, in bits; up to 16 [default: 8]')
    parser.add_argument('-f', '--samplerate', metavar='samples_per_second', type=int_from_msps, default=1000000,
                         dest='sample_rate', help='samples to emit per second; up to 204MSPS [default: 1MSPS]')
    parser.add_argument('--oneshot', dest='repeat', action='store_false',
                         help='If provided, the given pattern will be shifted out only once.')
    parser.add_argument('--debug-sgpio', dest='debug_sgpio', action='store_true',
                         help='Developer option for debugging; dumps the SGPIO configuration after starting.')

    args = parser.parse_args()
    log_function, log_error = parser.get_log_functions()

    # Find our GreatFET.
    device = parser.find_specified_device()

    # Ensure the GreatFET supports our API.
    if not device.supports_api('pattern_generator'):
        log_error("The connected GreatFET doesn't seem to support pattern generation. A firmware upgrade may help.")

    # Create our pattern generator object.
    pattern_generator = PatternGenerator(device, sample_rate=args.sample_rate, bus_width=args.bus_width)

    # TODO: truncate things to the pattern generator object's limits

    # Figure out how the user wants their samples generated (or what the want done, in general).
    command_to_execute = commands[args.command]

    # Execute the core command, which usually generates samples.
    samples = command_to_execute(args, pattern_generator)

    # If the generator has generated samples, scan them out.
    if samples:
        pattern_generator.scan_out_pattern(samples, args.repeat)

    # If we've been asked to dump our SGPIO configuration, do so.
    if args.debug_sgpio:
        log_error(pattern_generator.dump_sgpio_config())

    # Log what we've done to the user.
    if samples:
        log_function("Scanning out {} samples at {}.".format(len(samples), eng_notation(args.sample_rate, unit='SPS')))
        log_function("Run '{} stop' to halt generation.".format(sys.argv[0]))
        log_function("")




if __name__ == '__main__':
    main()
