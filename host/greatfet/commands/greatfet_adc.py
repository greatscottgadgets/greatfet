#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import sys
import time

from greatfet.protocol import vendor_requests


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="utility for reading from the GreatFET's ADC")

    args         = parser.parse_args()
    log_function = parser.get_log_function()
    device       = parser.find_specified_device()

    if not device.supports_api('adc'):
        sys.stderr.write("This device doesn't seem to support an ADC. Perhaps your firmware needs to be upgraded?\n")
        sys.exit(0)


    # TODO: replace me with a proper call to gf.adc
    sample = device.apis.adc.read_sample(0, 0, 10)
    reading = (sample / 1024.0) * 3.3

    print("{}V".format(reading))


if __name__ == '__main__':
    main()

