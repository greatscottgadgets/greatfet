#!/usr/bin/env python3
#
# This file is part of GreatFET

#
# FIXME: should this be integreted into greatfet_sensor?
#

from __future__ import print_function

import struct
import time

from greatfet.protocol import vendor_requests


def main():
    from greatfet.utils import GreatFETArgumentParser
   
    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Periodically print temperature from DS18B20 sensor")
    parser.add_argument('-S', dest='s20', action='store_true', help='DS18S20')

    args = parser.parse_args()
    log_function = parser.get_log_function()
    device = parser.find_specified_device()

    while True:
        data = device.comms._vendor_request_in(vendor_requests.DS18B20_READ, length=2, timeout=2000)
        # temperature data is 16 bit signed
        temp = struct.unpack('<h', data)[0]
        if args.s20:
            temp /= 2.0
        else:
            temp /= 16.0
        print(time.strftime("%H:%M:%S"), temp, '{:.01f}'.format(temp * 9 / 5 + 32))
        time.sleep(1)

if __name__ == '__main__':
    main()
