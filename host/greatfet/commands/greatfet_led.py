#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import errno
import sys
import ast

import greatfet
from greatfet import GreatFET
from greatfet.utils import log_silent, log_verbose
from greatfet.peripherals.led import LED


def main():
    from greatfet.utils import GreatFETArgumentParser
    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for LED configuration on GreatFET")
    parser.add_argument('-t', '--toggle', nargs='*', type=ast.literal_eval, default=[], help="LED numbers to toggle (0-3)")
    parser.add_argument('--on', nargs='*', type=ast.literal_eval, default=[], help="LED numbers to turn on (0-3)")
    parser.add_argument('--off', nargs='*', type=ast.literal_eval, default=[], help="LED numbers to turn off (0-3)")
    args = parser.parse_args()

    log_function = log_verbose if args.verbose else log_silent

    try:
        log_function("Trying to find a GreatFET device...")
        device = GreatFET(serial_number=args.serial)
        log_function("{} found. (Serial number: {})".format(device.board_name(), device.serial_number()))
    except greatfet.errors.DeviceNotFoundError:
        if args.serial:
            print("No GreatFET board found matching serial '{}'.".format(args.serial), file=sys.stderr)
        else:
            print("No GreatFET board found!", file=sys.stderr)
        sys.exit(errno.ENODEV)

    if args.toggle:
        toggle(device, args.toggle, log_function)
    if args.on:
        on(device, args.on, log_function)
    if args.off:
        off(device, args.off, log_function)


def toggle(device, leds, log_function):
    for led_num in leds:
        led = LED(device, led_num)
        led.toggle()


def on(device, leds, log_function):
    for led_num in leds:
        led = LED(device, led_num)
        led.on()


def off(device, leds, log_function):
    for led_num in leds:
        led = LED(device, led_num)
        led.off()


if __name__ == '__main__':
    main()
    