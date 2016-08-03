#!/usr/bin/env python
#
# Copyright 2015 Dominic Spill <dominicgs@gmail.com)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.

from __future__ import print_function

import sys

import greatfet
from greatfet import GreatFET
from greatfet.protocol import vendor_requests
from greatfet.peripherals import gpio
from greatfet.peripherals import spi_flash

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
    data = device.vendor_request_in(request=vendor_requests.SPI_READ, length=length, value=command)
    #print(' '.join(["0x%02x" % d for d in data]))
    return data

def spi_info(device):
    print("Reading target device information")
    data = spi_read(device, 0x9F, 4)
    manufacturer = data[1]
    model = data[2]
    capacity = data[3]
    device = (manufacturer << 16) | (model << 8) | capacity
    print("Manufacturer: {:02x} {}".format(manufacturer, JEDECmanufacturers.get(manufacturer, "Unknown")))
    print("Device: {:02x} {}".format(model, JEDECdevices.get(device, "Unknown")))
    print("Capacity: {:02x} {} bytes".format(capacity, JEDECsizes.get(capacity, "Unknown")))
    return JEDECsizes.get(capacity)

def dump_flash(device, size):
    print("Dumping flash")
    address = [0x0b, 0,0,0]
    data = device.vendor_request_in(vendor_requests.SPI_DUMP_FLASH, length=255)
    # print(data)
    print(' '.join(["%02x" % d for d in data]))
    print("Command written")
    # data = spi_read(device, 0x00, 255)
    # print(len(data))
    # print(data[:20])

def i2c_xfer(device):
    pass
    
if __name__ == '__main__':

    device = GreatFET()
    if not device:
        print('No GreatFET devices found!')
        sys.exit()

    # Print the board's information...
    print("Found a {}!".format(device.board_name()))
    print("  Board ID: {}".format(device.board_id()))
    print("  Firmware version: {}".format(device.firmware_version()))
    print("  Part ID: {}".format(device.part_id()))
    print("  Serial number: {}".format(device.serial_number()))
    print()

    device.vendor_request_out(vendor_requests.INIT_SPI)
    size = spi_info(device)
    print()
    dump_flash(device, size)
    print()
    i2c_xfer(device)
    

    # target = spi_flash.SPIFlash(device, chip_select=gpio.J2.P35)
    # with open('flash.bin', 'wb') as file:
    #     flash_data = device.onboard_flash.read(0x0000, 0x100000)
    #     flash_data.tofile(file)
    
    # ... and toggle it's third LED, for fun.
    # device.vendor_request_out(vendor_requests.REGISTER_GPIO, index=1, data=[0x05, 0x0b, 0x02, 0x03])

    # Dev note: you can still easily use this to test low-level interfaces.
    # For example, to send the ENABLE_USB1 request, use:
    #
    #   device.vendor_request_out(vendor_requests.ENABLE_USB1)
    #
    # where ENABLE_USB1 is just an integer constant.
