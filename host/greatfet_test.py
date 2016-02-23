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
usb_vendor_request_enable_usb1 = 6
usb_vendor_request_led_toggle = 7

boards = {0:"GreatFET Azalea"}

def vendor_request_in(request, length=1):
    return device.ctrl_transfer(
        #0xC0,
        usb.ENDPOINT_IN | usb.TYPE_VENDOR | usb.RECIP_DEVICE,
        request, 0, 0, length)

def vendor_request_out(request, val=0):
    return device.ctrl_transfer(
        usb.ENDPOINT_OUT | usb.TYPE_VENDOR | usb.RECIP_DEVICE,
        request, val, 0)

if __name__ == '__main__':
    device = usb.core.find(idVendor=0x1d50, idProduct=0x60e6)
    if device:
        print 'Found GreatFET'
    else:
        print 'No GreatFET devices found'
        sys.exit()
    
    device.set_configuration()
    
    board_id = vendor_request_in(usb_vendor_request_read_board_id)
    print "Board ID %d - %s" % (board_id[0], boards[board_id[0]])
    
    serial_no = vendor_request_in(usb_vendor_request_read_partid_serialno, length=30)
    print "Serial no: " + ''.join(["%02X " % x for x in serial_no])
    
    #vendor_request_out(usb_vendor_request_led_toggle, 4)
    
    vendor_request_out(usb_vendor_request_enable_usb1)
