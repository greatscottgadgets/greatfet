#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import sys
import argparse

from greatfet.utils import GreatFETArgumentParser

def main():

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for loading runtime extensions on to a GreatFET board.")
    parser.add_argument('--m0', dest="m0", type=argparse.FileType('rb'), metavar='<filename>',
        help="loads the provided loadable file to run on the GreatFET's m0 core")

    args = parser.parse_args()
    log_function = parser.get_log_function()
    device = parser.find_specified_device()

    if not args.m0:
        parser.print_help()
        sys.exit(-1)

    if args.m0:
        data = args.m0.read()

        log_function("Loading {} byte loadable onto the M0 coprocessor.\n".format(len(data)))
        device.m0.run_loadable(data)
