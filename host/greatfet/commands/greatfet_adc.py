#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import time

from greatfet.protocol import vendor_requests


def main():
    logfile = 'log.bin'
#    logfile = '/tmp/fifo'
    from greatfet.utils import GreatFETArgumentParser
   
    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="Utility for experimenting with GreatFET's ADC")
    parser.add_argument('-f', dest='filename', metavar='<filename>', type=str, help="Write data to file", default=logfile)
    parser.add_argument('-a', dest='adc', action='store_true', help="Use internal ADC")

    args = parser.parse_args()
    log_function = parser.get_log_function()
    device = parser.find_specified_device()

    if args.adc:
        device.comms._vendor_request_out(vendor_requests.ADC_INIT)
    else:
        device.comms._vendor_request_out(vendor_requests.SDIR_RX_START)

    time.sleep(1)
    print(device.comms.device)

    with open(args.filename, 'wb') as f:
        try:
            while True:
                d = device.comms.device.read(0x81, 0x4000, 1000)
                # print(d)
                f.write(d)
        except KeyboardInterrupt:
            pass

    if not args.adc:
        device.comms._vendor_request_out(vendor_requests.SDIR_RX_STOP)


if __name__ == '__main__':
    main()
    
