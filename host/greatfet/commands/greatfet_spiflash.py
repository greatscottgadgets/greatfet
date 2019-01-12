#!/usr/bin/env python3
#
# This file is part of GreatFET
#

from __future__ import print_function

import argparse
import errno
import sys

from greatfet import GreatFET
from greatfet.errors import DeviceNotFoundError
from greatfet.utils import log_silent, log_verbose
from greatfet.protocol import vendor_requests


JEDECmanufacturers = {
    0xFF: "MISSING",
    0xEF: "Winbond",
    0xC2: "MXIC",
    0x20: "Numonyx/ST",
    0x1F: "Atmel",
    0x1C: "eON",
    0x01: "AMD/Spansion",
}

JEDECdevices = {
    0xFFFFFF: "MISSING",
    0xEF3015: "W25X16L",
    0xEF3014: "W25X80L",
    0xEF3013: "W25X40L",
    0xEF3012: "W25X20L",
    0xEF3011: "W25X10L",
    0xEF4015: "W25Q16DV",
    0xEF4018: "W25Q16DV",
    0xC22017: "MX25L6405D",
    0xC22016: "MX25L3205D",
    0xC22015: "MX25L1605D",
    0xC22014: "MX25L8005",
    0xC22013: "MX25L4005",
    0xC22010: "MX25L512E",
    0x204011: "M45PE10",
    0x202014: "M25P80",
    0x1f4501: "AT24DF081",
    0x1C3114: "EN25F80",
}

JEDECsizes = {
    0x18: 0x1000000,
    0x17: 0x800000,
    0x16: 0x400000,
    0x15: 0x200000,
    0x14: 0x100000,
    0x13: 0x080000,
    0x12: 0x040000,
    0x11: 0x020000,
    0x10: 0x010000,
}


def spi_read(device, command, length):
    device.comms._vendor_request_out(request=vendor_requests.SPI_WRITE, data=command)
    data = device.comms._vendor_request_in(request=vendor_requests.SPI_READ, length=length, value=command)
    
    #log_function(' '.join(["0x%02x" % d for d in data]))
    return data

def spi_info(device, log_function=log_silent):
    log_function("Reading target device information")

    # data = device.comms._vendor_request_in(request=vendor_requests.SPI_READ, length=length)

    data = spi_read(device, 0x9F, 4)
    manufacturer, model, capacity = data[1:4]
    device = (manufacturer << 16) | (model << 8) | capacity
    log_function("Manufacturer: {:02X} {}".format(manufacturer, JEDECmanufacturers.get(manufacturer, "Unknown")))
    log_function("Device: {:02X} {}".format(model, JEDECdevices.get(device, "Unknown")))
    log_function("Capacity: {:02X} {} bytes".format(capacity, JEDECsizes.get(capacity, "Unknown")), end="\n\n")
    return JEDECsizes.get(capacity)

def dump_flash(device, address=None, length=None, filename="flash.bin", log_function=log_silent):
    flash_size = spi_info(device, log_function=log_silent)
    if address is None:
        address = 0x00
    if length is None:
        length = flash_size - address
    log_function("Dumping flash")
    block_length = 256
    with open(filename, 'wb') as file:
        while(address < length):
            if(address+block_length > length):
                block_length = length - address
            if(address & 0xFF == 0):
                log_function("Reading 0x{:06x}".format(address))
            value = (address>>16) & 0xFFFF
            index = address & 0xFFFF
            data = device.comms._vendor_request_in(vendor_requests.SPI_DUMP_FLASH,
                                            length=block_length, index=index, value=value)
            file.write(data)
            address += 256

def i2c_xfer(device, log_function):
    log_function("Starting I2C")
    device.comms._vendor_request_out(vendor_requests.I2C_START)
    log_function("I2C started, writing byte")
    # value = slave address, index = response length"
    device.comms._vendor_request_out(vendor_requests.I2C_XFER, value=0x41, index=3, data=[0xF8])
    log_function("Fetching response")
    data = device.comms._vendor_request_in(vendor_requests.I2C_RESPONSE, length=3)
    log_function(data)

def main():
    # Set up a simple argument parser.
    parser = argparse.ArgumentParser(description="Utility for talking to external SPI flash chips with GreatFET")
    parser.add_argument('-i', '--info', dest='info', action='store_true',
                        help="Read Jedec information from target")
    parser.add_argument('-d', '--dump', dest='dump', metavar='<filename>',
                        type=str, help="Dump flash into file")
    parser.add_argument('-a', '--address', metavar='<n>', default=0,
                        type=int, help="Flash dump starting address")
    parser.add_argument('-l', '--length', metavar='<n>', type=int,default=None,
                        help="number of bytes to read (default: flash size)")
    parser.add_argument('-s', dest='serial', metavar='<serialnumber>', type=str,
                        help="Serial number of device, if multiple devices", default=None)
    parser.add_argument('-v', dest='verbose', action='store_true', help="Write data from file")
    args = parser.parse_args()

    # If we don't have an option, print our usage.
    if not args.info and not args.dump:
        parser.print_help()
        sys.exit()

    # Determine whether we're going to log to the stdout, or not at all.
    log_function = log_verbose if args.verbose else log_silent

    # Create our GreatFET connection.
    try:
        log_function("Trying to find a GreatFET device...")
        device = GreatFET(serial_number=args.serial)
        log_function("{} found. (Serial number: {})".format(device.board_name(), device.serial_number()))
    except DeviceNotFoundError:
        if args.serial:
            print("No GreatFET board found matching serial '{}'.".format(args.serial), file=sys.stderr)
        else:
            print("No GreatFET board found!", file=sys.stderr)
        sys.exit(errno.ENODEV)

    device.comms._vendor_request_out(vendor_requests.SPI_INIT)

    if args.info:
        spi_info(device, log_function=log_verbose)
    if args.dump:
        dump_flash(device, filename=args.dump, address=args.address,
                   length=args.length, log_function=log_verbose)


if __name__ == '__main__':
    main()
