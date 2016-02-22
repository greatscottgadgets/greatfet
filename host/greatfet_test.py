#!/usr/bin/env python
#
# Copyright 2015 Dominic Spill <dominicgs@gmail.com)
#
# This file is part of GreatFET.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

import sys
import usb

usb_vendor_request_erase_spiflash = 0
usb_vendor_request_write_spiflash = 1
usb_vendor_request_read_spiflash = 2
usb_vendor_request_read_board_id = 3
usb_vendor_request_read_version_string = 4
usb_vendor_request_read_partid_serialno = 5
    
device = usb.core.find(idVendor=0x1d50, idProduct=0x60e6)
if device:
    print 'Found GreatFET'
else:
    print 'No GreatFET devices found'
    sys.exit()

print 'Setting configuration'
device.set_configuration()
print 'Configuration set'

def vendor_request(request):
    return device.ctrl_transfer(
        #0xC0,
        usb.ENDPOINT_IN | usb.TYPE_VENDOR | usb.RECIP_DEVICE,
        request, 0, 0, 1)

print 'Attempting vendor request'
print vendor_request(usb_vendor_request_read_board_id)
print 'Vendor request complete'
