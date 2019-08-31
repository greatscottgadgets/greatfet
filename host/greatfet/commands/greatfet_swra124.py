#
# This file is part of GreatFET
#

from __future__ import print_function

import argparse
import errno
import sys
import ast

import greatfet
from greatfet import GreatFET
from greatfet.peripherals import swra124
from greatfet.utils import log_silent, log_verbose


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Debug utility for CC11XX")
    parser.add_argument('-I', '--identify', dest='ident', action='store_true',
                        help="Show target identification")
    parser.add_argument('-e', '--erase', dest='erase', action='store_true',
                        help="Erase target flash")
    parser.add_argument('--status', dest='status', action='store_true',
                        help="Read target status")
    parser.add_argument('-f', '--flash', dest='flash', type=str,
                        metavar='<filename>', help="Write target flash")
    parser.add_argument('--old-flash', dest='oldflash', type=str,
                        metavar='<filename>', help="Write target flash")
    parser.add_argument('-V', '--verify', dest='verify', type=str,
                        metavar='<filename>', help="Verify target flash")
    parser.add_argument('--old-verify', dest='oldverify', type=str,
                        metavar='<filename>', help="Verify target flash")
    parser.add_argument('-R', '--peek', dest='peek', action='store_true',
                        help="Read from memory location")
    parser.add_argument('-W', '--poke', dest='poke', type=ast.literal_eval,
                        metavar='<value>', help="Write to memory location")
    parser.add_argument('-a', '--address', dest='address', default=0,
                        type=ast.literal_eval, metavar='<address>',
                        help="Address for peek/poke/flash/dump/verify actions (default 0x00)")
    parser.add_argument('-l', '--length', dest='length', type=ast.literal_eval,
                        metavar='<length>', help="Length for peek/dump actions in bytes")
    parser.add_argument('--halt', dest='halt', action='store_true',
                        help="Halt chip")
    parser.add_argument('--resume', dest='resume', action='store_true',
                        help="Resume chip")
    parser.add_argument('--get-pc', dest='get_pc', action='store_true',
                        help="Get Program Counter")

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

    chipcon = swra124.SWRA124(device, log_function)
    chipcon.setup()
    chipcon.debug_init()

    if args.erase:
        log_function("Erasing chip")
        chipcon.chip_erase()

    if args.ident:
        log_function("Getting chip identification:")
        print("chip=%0.4x" % chipcon.get_chip_id())

    if args.status:
        status = chipcon.read_status()
        print(status)

    if args.flash:
        log_function("Starting flash")
        chipcon.flash(args.flash)

    if args.oldflash:
        log_function("Starting oldflash")
        chipcon.oldflash(args.oldflash)

    if args.verify:
        log_function("Starting verify")
        chipcon.verify(args.verify)

    if args.oldverify:
        log_function("Starting oldverify")
        chipcon.oldverify(args.oldverify)

    if args.peek:
        print("%0.2x" % chipcon.peek_data_byte(args.address))

    if args.poke:
        chipcon.poke_data_byte(args.address, args.poke)

    if args.halt:
        chipcon.halt()

    if args.resume:
        chipcon.resume()

    if args.get_pc:
        print("%0.4x" % chipcon.get_pc())

    chipcon.debug_stop()


if __name__ == '__main__':
    main()
