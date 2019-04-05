#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function
from __future__ import absolute_import

import argparse
import errno
import sys

import greatfet
from greatfet import GreatFET
from greatfet.utils import log_silent, log_verbose, GreatFETArgumentParser


def main():
    parser = GreatFETArgumentParser(
        description="Convenience shell for working with GreatFET devices.")
    parser.add_argument('-n', '--length', dest='length', metavar='<bytes>', type=int,
                        help="maximum amount of data to read (default: 4096)", default=4096)
    args = parser.parse_args()


    args = parser.parse_args()
    gf = parser.find_specified_device()
    log_function = parser.get_log_function()

    if not gf.supports_api('debug'):
        print("ERROR: This board doesn't appear to support the debug API -- was its firmware built without it?")
        return

    if not gf.apis.debug.supports_verb('read_dmesg'):
        print("ERROR: This board doesn't appear to support debug logging. Was its firmware built with logging or the ringbuffer off?")
        return

    #Read and print the dmesg log.
    logs = gf.read_debug_ring(args.length)
    log_function("Ring buffer contained {} bytes of data:\n".format(len(logs)))
    print(logs)

if __name__ == '__main__':
    main()

