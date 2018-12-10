#!/usr/bin/env python
# coding=utf-8
#
# This file is part of GreatFET
#

from __future__ import print_function

import argparse
import errno
import struct
import sys

from greatfet import GreatFET
from greatfet.errors import DeviceNotFoundError
from greatfet.utils import log_silent, log_verbose
from greatfet.protocol import vendor_requests

def spi_write(device, data):
    device.comms._vendor_request_out(request=vendor_requests.SPI_WRITE, data=data)

def spi_read(device, command, length):
    data = device.comms._vendor_request_in(request=vendor_requests.SPI_READ, length=length, value=command)
    return data

def adf7242_command(device, command):
    spi_write(device, struct.pack('B', command))

def adf7242_status(device):
    adf7242_command(device, 0xFF)
    return spi_read(device, 0, 1)[0]

def adf7242_read_reg(device, reg_num):
    data = struct.pack('BBBB',
            0x38 | reg_num >> 8,
            reg_num & 0xff,
            0xff, 0xff)
    spi_write(device, data)
    resp_data = spi_read(device, 0, 4)
    return resp_data[3]

def adf7242_write_reg(device, reg_num, data):
    data = struct.pack('BBB',
            0x18 | reg_num >> 8,
            reg_num & 0xff,
            data)
    spi_write(device, data)

def adf7242_temperature(device):
    adf7242_command(device, 0xB6) # RC_MEAS
    adc_val = adf7242_read_reg(device, 0x3AE) & 0x3F
    temp = 4.72 * adc_val + 65.58 - 322.6 # correction value
    adf7242_command(device, 0xB2) # RC_IDLE
    return temp

def auto_int(x):
    return int(x, 0)

def main():
    parser = argparse.ArgumentParser(description="Drive ADF7242 via SPI")
    parser.add_argument('-r', '--read-register', dest='read_reg', metavar='<addr>', default=None,
                        type=auto_int, help="Read register")
    parser.add_argument('-w', '--write-register', dest='write_reg', default=None,
                        nargs=2, metavar=('<addr>', '<value>'),
                        type=auto_int, help="Write register")
    parser.add_argument('-c', '--command', dest='command', default=None,
                        type=auto_int, metavar='<command>', help="SPI command")
    parser.add_argument('-S', '--status', action='store_true', help='Get status byte')
    parser.add_argument('-t', '--temperature', action='store_true', help='Read temperature')
    parser.add_argument('--reset', action='store_true', help='Reset ADF7242')
    parser.add_argument('-s', dest='serial', metavar='<serialnumber>', type=str,
                        help="Serial number of device, if multiple devices", default=None)
    parser.add_argument('-v', dest='verbose', action='store_true', help="Write data from file")
    args = parser.parse_args()

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

    if args.reset:
        adf7242_command(device, 0xC7)

    if args.read_reg:
        print('Register value: {:02x}'.format(adf7242_read_reg(device, args.read_reg)))

    if args.write_reg:
        adf7242_write_reg(device, args.write_reg[0], args.write_reg[1])

    if args.command:
        adf7242_command(device, args.command)

    if args.status:
        print('Status: {:02x}'.format(adf7242_status(device)))

    if args.temperature:
        temp = adf7242_temperature(device)
        temp_f = temp * 9 / 5 + 32
        print('Temperature: {:0.1f} °F / {:.01f} °C'.format(temp_f, temp))


if __name__ == '__main__':
    main()
