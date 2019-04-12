#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import argparse
import errno
import sys
import time
import os
import tempfile
from zipfile import ZipFile

import greatfet
from greatfet import GreatFET
from greatfet.utils import log_silent, log_verbose

def main():
    # Set up a simple argument parser.
    parser = argparse.ArgumentParser(description="Logic analyzer implementation for GreatFET")
    parser.add_argument('-s', dest='serial', metavar='<serialnumber>', type=str,
                        help="Serial number of device, if multiple devices", default=None)
    parser.add_argument('-o', '-b', '--binary', dest='binary', metavar='<filename>', type=str,
                        help="Generate a binary file contianing the captured data.")
    parser.add_argument('-p', '--pulseview', dest='pulseview', metavar="<filename>", type=str,
                        help="Generate a PulseView session file.")
    parser.add_argument('-v', dest='verbose', action='store_true', help="Write data from file")
    args = parser.parse_args()

    capture_data_name='logic-1'

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

    device.apis.logic_analyzer.start()
    time.sleep(1)

    _, path = tempfile.mkstemp(None, None, os.getcwd())
    if args.binary:
        bin_file_name = arc_name = args.output
        sr_name = args.output + ".sr"
    else:
        bin_file_name = path


    print("Press Ctrl+C to stop reading data from device")
    try:
        with open(bin_file_name, "wb") as bin_file:
            try:
                while True:
                    d = device.comms.device.read(0x81, 16384, 1000)
                    bin_file.write(d)
            except KeyboardInterrupt:
                print()

            if args.binary:
                print("Binary data written to file '%s'" % args.output)

            if args.pulseview:
                metadata_str = "[device 1]\n" \
                    "capturefile={}\n" \
                    "total probes=8\n" \
                    "samplerate=17 MHz\n" \
                    "total analog=0\n" \
                    "probe1=SGPIO0\n" \
                    "probe2=SGPIO1\n" \
                    "probe3=SGPIO2\n" \
                    "probe4=SGPIO3\n" \
                    "probe5=SGPIO4\n" \
                    "probe6=SGPIO5\n" \
                    "probe7=SGPIO6\n" \
                    "probe8=SGPIO7\n" \
                    "unitsize=1\n".format(capture_data_name)
                # pulseview compatible .sr archive
                with ZipFile(args.pulseview, "w") as zip:
                    zip.write(bin_file_name, arcname='logic-1')
                    zip.writestr("metadata", metadata_str)
                    zip.writestr("version", "2")
                print("Pulseview compatible session file created: '%s'" % args.pulseview)
    finally:
        try:
            os.remove(path)
        except OSError:
            pass

    device.apis.logic_analyzer.stop()


if __name__ == '__main__':
    main()
