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

def msp430_test(jtag):
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
    parser.add_argument('-I', '--identify', dest='ident', action='store_true',
                        help="Show target identification")
    parser.add_argument('-e', '--erase', dest='erase', action='store_true',
                        help="Erase target flash")
    parser.add_argument('-E', '--erase_info', dest='erase_info', 
                        action='store_true', help="Erase target info flash")
    parser.add_argument('-f', '--flash', dest='flash', type=str,
                        metavar='<filename>', help="Write target flash")
    parser.add_argument('-V', '--verify', dest='verify', type=str,
                        metavar='<filename>', help="Verify target flash")
    parser.add_argument('-d', '--dump', dest='dump', type=str,
                        metavar='<filename>', help="Dump target flash")
    parser.add_argument('-r', '--run', dest='run', action='store_true',
                        help="Run target device")
    parser.add_argument('-R', '--peek', dest='peek', action='store_true',
                        help="Read from memory location")
    parser.add_argument('-W', '--poke', dest='poke', type=ast.literal_eval,
                        metavar='<value>', help="Write to memory location")
    parser.add_argument('-a', '--address', dest='address', default=0,
                        type=ast.literal_eval, metavar='<address>', 
                        help="Address for peek/poke/flash/dump/verify actions (default 0x00)")
    parser.add_argument('-l', '--length', dest='length', type=ast.literal_eval,
                        metavar='<length>', help="Length for peek/dump actions in bytes")
    parser.add_argument('-t', '--test', dest='test', action='store_true',
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
        print("Error, misidentified as 0x%02x." % jtag_id)
        print("Check wiring, as this should be 0x89 or 0x91.")
        sys.exit(errno.ENODEV)
    
    if args.ident:
        print("Identifies as %s (0x%04x)" % 
                    (jtag.ident_string(), jtag.ident()))
    
    if args.dump:
        if args.length:
            end = args.address + args.length
        else:
            end = 0xffff
        log_function("Dumping from 0x%04x to 0x%04x to %s." 
                     % (args.address, end, args.dump))
        with open(args.dump, 'wb') as f:
            address = args.address
            while address < end:
                data = jtag.peek_block(address)
                f.write(data)
                address += len(data)
    
    if args.erase:
        log_function("Erasing main flash memory.")
        jtag.erase_flash()
    
    if args.erase_info:
        log_function("Erasing info flash.")
        jtag.erase_info()    
    
    if args.flash:
        with open(args.flash, 'rb') as f:
            address = args.address
            if args.length:
                end = address + args.length
            else:
                end = address + f.seek(0, 2)
                f.seek(0)
            log_function("Writing %d bytes of %s to 0x%04x."
                         % (end-address, args.flash, address))
            while address < end:
                if end - address < 0x400:
                    block_size = end - address
                else:
                    block_size = 0x400
                data = f.read(block_size)
                result = jtag.poke_flash_block(address, data)
                address += block_size
            else:
                log_function("Flash contents written.")

    if args.verify:
        with open(args.verify, 'rb') as f:
            address = args.address
            if args.length:
                end = address + args.length
            else:
                end = address + f.seek(0, 2)
                f.seek(0)
            log_function("Verifying %d bytes of %s from 0x%04x."
                         % (end-address, args.verify, address))
            while address < end:
                if end - address < 0x400:
                    block_size = end - address
                else:
                    block_size = 0x400
                data = jtag.peek_block(address, block_size)
                buffer = f.read(len(data))
                if data != buffer:
                    print("File does not match flash.")
                    break
                address += len(data)
            else:
                print("Flash contents verified.")
    
    if args.poke:
        log_function("Writing 0x%04x to 0x%04x." % (args.poke, args.address))
        written = jtag.poke(args.address, args.poke)
        if written != args.poke:
            print("Failed to write 0x%04x to 0x%04x" % (args.poke, args.address))

    if args.peek:
        if args.length:
            length = args.length
            if length % 2:
                length += 1
        else:
            length = 2
        log_function("Reading %d bytes from 0x%04x." % (length, args.address))
        values = jtag.peek(args.address, length)
        for i, v in enumerate(values):
            print("%04x: %04x" % (args.address + i*2, v))
    
    if args.run:
        log_function("Resuming target execution.")
        jtag.run()
    
    if args.test:
        log_function("Running test.")
        msp430_test(jtag)
    
if __name__ == '__main__':
    main()
