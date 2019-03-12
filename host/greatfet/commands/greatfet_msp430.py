#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import argparse
import errno
import sys
import ast

import greatfet
from greatfet import GreatFET
from greatfet.peripherals import msp430_jtag
from greatfet.utils import log_silent, log_verbose

def msp430_test(self):
    """Test MSP430 JTAG functions.  Requires that a chip be attached."""
    
    if jtag.ident()==0xffff:
        print("ERROR Is anything connected?")
    print("Testing %s." % jtag.ident_string())
    print("Testing RAM from 200 to 210.")
    for a in range(0x200,0x210):
        jtag.poke(a,0)
        if(jtag.peek(a)!=0):
            print("Fault at %06x" % a)
        jtag.poke(a,0xffff)
        if(jtag.peek(a)!=0xffff):
            print("Fault at %06x" % a)
            
    print("Testing identity consistency.")
    ident=jtag.ident()
    for a in range(1,20):
        ident2=jtag.ident()
        if ident!=ident2:
            print("Identity %04x!=%04x" % (ident,ident2))
    
    print("Testing flash erase.")
    jtag.erase_flash()
    for a in range(0xffe0, 0xffff):
        if jtag.peek(a)!=0xffff:
            print("%04x unerased, equals %04x" % (
                a, jtag.peek(a)))

    print("Testing flash write.")
    for a in range(0xffe0, 0xffff):
        jtag.poke_flash(a,0xbeef)
        if jtag.peek(a)!=0xbeef:
            print("%04x unset, equals %04x" % (
                a, jtag.peek(a)))
    
    print("Tests complete, erasing.")
    jtag.erase_flash()


def main():
    from greatfet.utils import GreatFETArgumentParser

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="JTAG debug utility for MSP430")
    parser.add_argument('-I', dest='ident', action='store_true',
                        help="Show target identification")
    parser.add_argument('-e', dest='erase', action='store_true',
                        help="Erase target flash")
    parser.add_argument('-E', dest='erase_info', action='store_true',
                        help="Erase target info flash")
    parser.add_argument('-f', dest='flash', type=str,
                        help="Write target flash")
    parser.add_argument('-V', dest='verify', type=str,
                        help="Verify target flash")
    parser.add_argument('-d', dest='dump', type=str,
                        help="Dump target flash")
    parser.add_argument('-r', dest='run', action='store_true',
                        help="Run target device")
    parser.add_argument('-R', dest='peek', action='store_true',
                        help="Read from memory location")
    parser.add_argument('-W', dest='poke', type=ast.literal_eval,
                        help="Write to memory location")
    parser.add_argument('-a', dest='address', type=ast.literal_eval,
                        help="Address for peek/poke/flash/dump/verify actions")
    parser.add_argument('-l', dest='length', type=ast.literal_eval,
                        help="Length for peek/dump actions")
    parser.add_argument('-t', dest='test', action='store_true',
                        help="Test MSP430 JTAG functions (destructive)")
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
    
    jtag = msp430_jtag.JTAG_MSP430(device)
    jtag_id = jtag.start()
    if jtag_id in (0x89, 0x91):
        log_function("Target dentified as 0x%02x." % jtag_id)
    else:
        print("Error, misidentified as %02x." % jtag_id)
        print("Check wiring, as this should be 0x89 or 0x91.")
        sys.exit(errno.ENODEV)
    
    if args.ident:
        print("Identifies as %s (%04x)" % 
                    (jtag.ident_string(), jtag.ident()))
    
    if args.dump:
        end = args.address + args.length
        log_function("Dumping from %04x to %04x to %s." 
                     % (args.address, end, args.dump))
        with open(args.dump, 'w') as f:
            pass
    
    if args.erase:
        log_function("Erasing main flash memory.")
        jtag.erase_flash()
    
    if args.erase_info:
        log_function("Erasing info flash.")
        jtag.erase_info()    
    
    if args.flash:
        with open(args.flash, 'r') as f:
            if args.length:
                buffer = f.read(args.length)
            else:
                buffer = f.read()
            length = len(buffer)
            log_function("Writing %s from %04x to %04x."
                         % (args.flash, args.address, args.address + length))
            
    
    if args.verify:
        with open(args.verify, 'r') as f:
            if args.length:
                buffer = f.read(args.length)
            else:
                buffer = f.read()
            if not args.address:
                address = 0x00
            else:
                address = args.address
            length = len(buffer)
            log_function("Verifying %04x bytes of %s from %04x."
                         % (length, args.flash, address))
    
    if args.peek:
        pass
    
    if args.poke:
        jtag.poke()
    
    if args.run:
        jtag.run()
    
    if args.poke:
        pass

if __name__ == '__main__':
    main()
