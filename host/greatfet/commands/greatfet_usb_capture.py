#!/usr/bin/env python3
#
# This file is part of GreatFET

from __future__ import print_function

import argparse
import errno
import sys
import time
import os
import array
import tempfile
import threading

# Temporary?
import usb

from zipfile import ZipFile

import greatfet

from greatfet import GreatFET, find_greatfet_asset
from greatfet.utils import GreatFETArgumentParser, log_silent, log_error

# Default sample-delivery timeout.
SAMPLE_DELIVERY_TIMEOUT_MS   = 3000

# Speed constants.
SPEED_HIGH = 0
SPEED_FULL = 1
SPEED_LOW  = 2

# Speed name constants.
SPEED_NAMES = {
    SPEED_HIGH: 'high',
    SPEED_FULL: 'full',
    SPEED_LOW:  'low',
}

# Rhododendron packet types.
PACKET_TYPE_USB_DATA     = 0
PACKET_TYPE_EVENT_START  = 0x80
PACKET_TYPE_EVENT_END_OK = 0x81

PACKET_SIZES = {
    PACKET_TYPE_USB_DATA:          33,
    PACKET_TYPE_EVENT_START:   6,
    PACKET_TYPE_EVENT_END_OK:  6
}


def allocate_transfer_buffer(buffer_size):
    return array.array('B', bytes(buffer_size))


def read_rhododendron_m0_loadable():
    """ Read the contents of the default Rhododendron loadable from the tools distribution. """

    filename = os.getenv('RHODODENDRON_M0_BIN', find_greatfet_asset('rhododendron_m0.bin'))

    with open(filename, 'rb') as f:
        return f.read()



def main():

    # Start off with a default packet state.
    current_packet_type         = None
    current_packet_data         = array.array('B')
    current_packet_remaining    = 0
    next_data_packet_emit_after = []
    next_data_packet_timestamp  = None

    current_usb_data = array.array('B')


    def emit_usb_packet(packet_data):
        """
        Emits a raw USB packet to the target format.
        """

        print("Packet: [{}]".format(packet_data))


    def is_valid_pid_byte(byte):
        """ Returns true iff the given byte could be a valid PID. """

        pid     = byte & 0xf
        inverse = byte >> 4

        return (pid ^ inverse) == 0xf




    def hack_smoothe_out_jitter(packet_data, emit_point):
        """ XXX Horrorhack intended to "smoothe" over a missing firmware piece until it's implemented. """

        try:
            print("next PID would be {} -- valid: {}".format(packet_data[emit_point], is_valid_pid_byte(packet_data[emit_point])))

            # If the next byte is a valid PID, there's no need to hack anything.
            # Move along..
            if is_valid_pid_byte(packet_data[emit_point]):
                return 0
        except IndexError:
            pass

        print("trying to smoothe out a bit of jitter:")

        try:

            print("trying delta -1 [{}] -- {}".format(
                    packet_data[emit_point - 1], is_valid_pid_byte(packet_data[emit_point - 1])
                ))

            # Otherwise, if the previous byte was a valid PID, move back to it.
            if is_valid_pid_byte(packet_data[emit_point - 1]):
                return -1
        except IndexError:
            pass

        try:

            print("trying delta +1 [{}] -- {}".format(
                    packet_data[emit_point + 1], is_valid_pid_byte(packet_data[emit_point + 1])
                ))

            # Otherwise, if the previous byte was a valid PID, move back to it.
            if is_valid_pid_byte(packet_data[emit_point + 1]):
                return 1
        except IndexError:
            pass

        return 0


    def handle_capture_packet(packet_type, packet_data):
        """
        Handles a received full packet from the analyzer.
        """

        nonlocal current_packet_type, current_packet_data, current_packet_remaining
        nonlocal next_data_packet_emit_after, next_data_packet_timestamp, current_usb_data

        # If this is an "end event" packet, grab the point at which
        # we're supposed to emit the packet.
        if packet_type == PACKET_TYPE_EVENT_END_OK:

            if (not next_data_packet_emit_after) or (next_data_packet_emit_after[-1] != packet_data[0]):
                next_data_packet_emit_after.append(packet_data[0])

        # If this is start packet, grab the timestamp from it.
        elif packet_type == PACKET_TYPE_EVENT_START:

            # FIXME: implement
            pass

        # If this is a USB data packet, handle it.
        elif packet_type == PACKET_TYPE_USB_DATA:
            print("got {} bytes of USB data [{}] <emit at: {}>".format(len(packet_data), packet_data, next_data_packet_emit_after))

            existing_packet_length = len(current_usb_data)

            # Add the data to the current packet.
            current_usb_data.extend(packet_data)

            # Store that we're handled 0 bytes into the current packet.
            position_in_packet = 0

            # If this packet ends a USB packet, emit the completed usb packet.
            while next_data_packet_emit_after:

                emit_after_bytes   = next_data_packet_emit_after.pop(0) - position_in_packet + 1

                print("emit after: {} bytes [data: {}]".format(emit_after_bytes, current_usb_data))

                # Emit the USB packet up until this point...
                emit_point = existing_packet_length + emit_after_bytes


                # XXX: Temporary hack to smoothe out single-byte event offsets until
                # the firmware properly has NXT and DIR tied to an SCT counter.
                delta = hack_smoothe_out_jitter(current_usb_data, emit_point)
                emit_point       += delta
                emit_after_bytes += delta


                emit_usb_packet(current_usb_data[0:emit_point])

                # ... and mark ourselves as already having emitted the relevant bytes, so
                # the future "emit afters" can be scaled properly.
                position_in_packet += emit_after_bytes


                # ... and remove those packets from the buffer.
                del current_usb_data[0:emit_point]


            next_data_packet_emit_after = []



    def parse_capture_packets(samples, args, bin_file):
        """
        Parses a set of packets coming from a USB capture device.
        """

        nonlocal current_packet_type, current_packet_data, current_packet_remaining
        nonlocal next_data_packet_emit_after, next_data_packet_timestamp, current_usb_data

        # Parse all of the samples we have in our buffer.
        while samples:

            sample = samples.pop(0)
            #print("sample: {} / current_packet_remaining: {}".format(sample, current_packet_remaining))

            # If we have data remaining in our packet, parse this sample as data.
            if current_packet_remaining:
                current_packet_data.append(sample)
                current_packet_remaining -= 1

                # If we just completed a given packet, handle it.
                if current_packet_remaining == 0:
                    handle_capture_packet(current_packet_type, current_packet_data)

                    # Clear out our packet state.
                    del current_packet_data[:]

            # Otherwise, handle this as a new packet.
            elif (current_packet_remaining == 0) and (sample in PACKET_SIZES):
                current_packet_type      = sample
                current_packet_remaining = PACKET_SIZES[current_packet_type] - 1
            else:
                raise IOError("unknown packet type {}! stream error?\n".format(sample))


    # Set up our argument parser.
    parser = GreatFETArgumentParser(description="Simple Rhododendron capture utility for GreatFET.", verbose_by_default=True)
    parser.add_argument('-o', '-b', '--binary', dest='binary', metavar='<filename>', type=str,
                        help="Write the raw samples captured to a file with the provided name.")
    parser.add_argument('--m0', dest="m0", type=argparse.FileType('rb'), metavar='<filename>',
                        help="loads the specific m0 coprocessor 'loadable' instead of the default Rhododendron one")
    parser.add_argument('-F', '--full-speed', dest='speed', action='store_const', const=SPEED_FULL, default=SPEED_HIGH,
                        help="Capture full-speed data.")
    parser.add_argument('-L', '--low-speed', dest='speed', action='store_const', const=SPEED_LOW, default=SPEED_HIGH,
                        help="Capture low-speed data.")
    parser.add_argument('-H', '--high-speed', dest='speed', action='store_const', const=SPEED_HIGH,
                        help="Capture high-speed data. The default.")
    parser.add_argument('-O', '--stdout', dest='write_to_stdout', action='store_true',
                         help='Provide this option to log the received data to the stdout.. Implies -q.')


    # And grab our GreatFET.
    args = parser.parse_args()

    # If we're writing binary samples directly to stdout, don't emit logs to stdout; otherwise, honor the
    # --quiet flag.
    if args.write_to_stdout:
        log_function = log_silent
    else:
        log_function = parser.get_log_function()

    # Ensure we have at least one write operation.
    if not (args.binary or args.write_to_stdout):
        parser.print_help()
        sys.exit(-1)

    # Find our GreatFET.
    device = parser.find_specified_device()

    # Bring our Rhododendron board online; and capture communication parameters.
    buffer_size, endpoint = device.apis.usb_analyzer.initialize(args.speed, timeout=10000, comms_timeout=10000)

    # $Load the Rhododendron firmware loadable into memory...
    try:
        if args.m0:
            data = args.m0.read()
        else:
            data = read_rhododendron_m0_loadable()
    except (OSError, TypeError):
        log_error("Can't find a Rhododendron m0 program to load!")
        log_error("We can't run without one.")
        sys.exit(-1)

    # Debug only: setup a pin to track when we're handling SGPIO data.
    debug_pin = device.gpio.get_pin('J1_P3')
    debug_pin.set_direction(debug_pin.DIRECTION_OUT)

    # ... and then run it on our m0 coprocessor.
    device.m0.run_loadable(data)

    # Print what we're doing and our status.
    log_function("Reading raw {}-speed USB data!\n".format(SPEED_NAMES[args.speed]))
    log_function("Press Ctrl+C to stop reading data from device.")

    # If we have a target binary file, open the target filename and use that to store samples.
    bin_file = None
    if args.binary:
        bin_file = open(args.binary, 'wb')
        bin_file_name = args.binary

    # Now that we're done with all of that setup, perform our actual sampling, in a tight loop,
    device.apis.usb_analyzer.start_capture()

    transfer_buffer = allocate_transfer_buffer(buffer_size)

    total_captured = 0

    try:
        while True:

            # Capture data from the device, and unpack it.
            try:
                new_samples = device.comms.device.read(endpoint, transfer_buffer, SAMPLE_DELIVERY_TIMEOUT_MS)
                samples =transfer_buffer[0:new_samples - 1]

                total_captured += new_samples
                log_function("Captured {} bytes.".format(total_captured), end="\r")

                parse_capture_packets(samples, args, bin_file)


            except usb.core.USBError as e:
                if e.errno != errno.ETIMEDOUT:
                    raise

    except KeyboardInterrupt:
        pass
    except usb.core.USBError as e:
        log_error("")
        if e.errno == 32:
            log_error("ERROR: Couldn't pull data from the device fast enough! Aborting.")
        else:
            log_error("ERROR: Communications failure -- check the connection to -- and state of  -- the GreatFET. ")
            log_error("(More debug information may be available if you run 'gf dmesg').")
            log_error(e)
    finally:

        # No matter what, once we're done stop the device from sampling.
        device.apis.usb_analyzer.stop_capture()

        if args.binary:
            log_function("Binary data written to file '{}'.".format(args.binary))


if __name__ == '__main__':
    main()
