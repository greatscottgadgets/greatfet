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

def spi_read(device, command, length):
    data = device.vendor_request_in(request=vendor_requests.SPI_READ, length=length, value=command)
    print(' '.join(["0x%02x" % d for d in data]))

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

    data = device.vendor_request_out(vendor_requests.INIT_SPI)
    print("Attempting Read")
    spi_read(device, 0xAB, 5)
    spi_read(device, 0x90, 6)
    spi_read(device, 0x9F, 4)

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
