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

from greatfet import GreatFET
from greatfet.utils import GreatFETArgumentParser, eng_notation, from_eng_notation, log_silent, log_error

# Default sample-delivery timeout.
SAMPLE_DELIVERY_TIMEOUT_MS   = 3000

# Default number of pre-allocated buffers.
DEFAULT_PREALLOCATED_BUFFERS = 4096


def unpack_data(data, bus_width):

    # If we happen to have data that's packed nicely into bytes, return it directly.
    if bus_width == 8:

        # Technically, the processor captures data such that the most recent
        # sample is in the MSB -- so each word has its bytes flipped in the
        # slice registers. As a happy coincidence, treating this data as raw,
        # block data also reads it as little endian; which conveniently flips
        # all of the bytes back for us. :)
        return bytes(data)

    # Otherwise, we'll need to assemble the data into words.
    else:
        # We'll have to both unpack and re-order the data we capture, as 1) it shifts
        # through the slice buffer from MSB to LSB; and 2) we read the bulk data as
        # little endian words, which reverse the bytes in every word.
        #
        # As an example, consider GreatFET set to sample a set of two-bit values. If we
        # read the value A as our first sample, and then, B, and etc, we wind up with
        # the following values in each slice "shift-register" when the sampling is complete:
        #
        #  PONM  LKJI  HGFE  DCBA
        #
        # When we read them back, each word looks like it has the following arrangement:
        #
        #
        #  DCBA  HGFE  LKJI  PONM
        #
        # If we then flip each set of samples in the relevant byte, we get:
        #
        #  ABCD  EFGH  IJKL  MNOP
        #
        # Thus, flipping each byte individually gives us what we want. Note that
        # the way the SGPIO slices work means we can't actually wind up samples
        # straddling words -- every word is evenly divisible by a sample size.

        ## Figure out how many samples are packed into a byte,  and construct a string
        ## that describes the layout for the bitstruct module.
        samples_per_byte = int(8 / bus_width)
        sample_mask      = (1 << bus_width) - 1

        all_zeroes = [0] * samples_per_byte
        all_ones   = [sample_mask] * samples_per_byte

        unpacked = array.array('B')

        ## Iterate over each byte in the data...
        for byte in data:

            # Optimization: short circuit out all 0s / all 1s.
            if byte == 0x00:
                unpacked.extend(all_zeroes)
            elif byte == 0xFF:
                unpacked.extend(all_ones)
            else:
                # ... and split it into samples.
                for _ in range(samples_per_byte):
                    sample = byte & sample_mask
                    unpacked.append(sample)

                    byte >>= bus_width

        return unpacked


def emit_sigrok_file(filename, sample_file_source, bus_width, sample_rate, first_probe_number=0, channel_names=None):
    """ Generates a sigrok-compatible Session Archive (SR archive) that wraps the given sample file.

    params:
        filename: The archive file to produce; usually ends in .sr.
    """

    # Start building the metadata that describes the Sigrok Archive.
    metadata = [
        "[device 1]",
        "capturefile=logic-1",
        "total probes={}".format(bus_width),
        "samplerate={} Hz\n".format(sample_rate),
        "total analog=0\n"
    ]

    # Add a description of each of our logic channels.
    for i in range(bus_width):

        # If we have a known channel name, use it.
        if channel_names and (i in channel_names):
            channel_name = channel_names[i]
        else:
            channel_name = "SGPIO{}".format(i + first_probe_number)

        metadata.append("probe{}={}\n".format(i + 1, channel_name))

    # Identify how many bytes exist per sample, given our format.
    unit_size = int((bus_width + 7) / 8)
    metadata.append("unitsize={}\n".format(unit_size))

    # Finally, build our zip archive with all of the relevant components.
    with ZipFile(filename, "w") as zip:
        zip.write(sample_file_source, arcname='logic-1')
        zip.writestr("metadata", "\n".join(metadata))
        zip.writestr("version", "2")


def background_process_data(termination_request, args, bus_width, bin_file, empty_buffers, full_buffers):
    """ Thread that handles processing our samples in the background. """

    # Process in the background until we're explicitly terminated.
    while True:

        # If we have nothing to do, check to see if it's time for us to stop.
        # If it isn't, keep looping.
        if len(full_buffers) == 0:
            if termination_request.is_set():
                break
            else:
                # Sleep a very short while before we return to the beginning of the loop
                # this momentarily yields back to the thread scheduler.
                time.sleep(0.0001)
                continue

        if termination_request.is_set():
            if args.verbose:
                sys.stderr.write("{} buffers remaining...\r".format(len(full_buffers)))
                sys.stderr.flush()

        active_buffer = full_buffers.pop(0)

        # Assuming we got a data buffer, process it.
        samples = unpack_data(active_buffer, bus_width)

        # Output the samples to the appropriate targets.
        if args.pulseview or args.binary:
            bin_file.write(samples)
        if args.write_to_stdout:
            sys.stdout.buffer.write(samples)

        # ... and add the buffer back to our empty list.
        empty_buffers.append(active_buffer)


def allocate_transfer_buffer(buffer_size):
    return array.array('B', b"\0" * buffer_size)


def main():

    # Simple type-arguments for parsing.
    int_from_msps = lambda x : from_eng_notation(x, units=['Hz', 'SPS'], to_type=int)

    # Set up our argument parser.
    parser = GreatFETArgumentParser(description="Logic analyzer implementation for GreatFET", verbose_by_default=True)
    parser.add_argument('-o', '-b', '--binary', dest='binary', metavar='<filename>', type=str,
                        help="Write the raw samples captured to a file with the provided name.")
    parser.add_argument('-p', '--pulseview', '--sigrok', dest='pulseview', metavar="<filename>",
                        type=str, help="Generate a Sigrok/PulseView session file, and write it to the provided filename.")
    parser.add_argument('-f', '--samplerate', metavar='samples_per_second', type=int_from_msps, default=17000000,
                         dest='sample_rate', help='samples to capture per second (default: 17MSPS)')
    parser.add_argument('-n', '--num-channels', metavar='channels', type=int, default=8,
                         dest='bus_width', help='the number of channels to capture (default: 8)')
    parser.add_argument('-B', '--second-bank', action='store_const', const=8, default=0, dest='first_pin',
                         help="Provide this option to capture from SGPIO8 up, rather than from SGPIO0 up.")
    parser.add_argument('-O', '--stdout', dest='write_to_stdout', action='store_true',
                         help='Provide this option to write the raw binary samples to the standard out. Implies -q.')
    parser.add_argument('--rhododendron', dest='rhododendron', action='store_true',
                         help='Capture raw packets for e.g. low or full speed USB using the Rhodadendon neighbor.')
    parser.add_argument('--raw-usb', dest='raw_usb', action='store_true',
                         help='Capture raw packets for e.g. low or full speed USB on SGPIO0 and SGPIO1.')
    parser.add_argument('--debug-sgpio', dest='debug_sgpio', action='store_true',
                         help='Developer option for debugging; dumps the SGPIO configuration before starting.')
    parser.add_argument('--stats', dest='print_stats', action='store_true',
                         help='Print capture statistics after the transfer is complete.')

    # And grab our GreatFET.
    args = parser.parse_args()

    # If we're writing binary samples directly to stdout, don't emit logs to stdout; otherwise, honor the
    # --quiet flag.
    if args.write_to_stdout:
        log_function = log_silent
        bin_file = None
    else:
        log_function = parser.get_log_function()

    # Ensure we have at least one write operation.
    if not (args.pulseview or args.binary or args.write_to_stdout):
        parser.print_help()
        sys.exit(-1)

    # Capture a few of the arguments.
    sample_rate = args.sample_rate
    bus_width   = args.bus_width

    channel_names = None

    # If we have one of the raw-usb options, apply their settings.
    if args.raw_usb or args.rhododendron:
        sample_rate    = int(51e6)
        bus_width      = 2
        channel_names = { 0: 'D-', 1: 'D+'}

    if args.rhododendron:
        args.first_pin = 8

    # Find our GreatFET.
    device = parser.find_specified_device()

    # Configure which locations we're using for SGPIO8/9.
    device.apis.logic_analyzer.configure_alt_mappings(args.rhododendron)

    # Set the first pin in our capture according to our bank setting.
    device.apis.logic_analyzer.change_first_pin(args.first_pin)

    # Replace the sample rate with the actual achieved sample rate.
    sample_rate, buffer_size, endpoint = device.apis.logic_analyzer.configure(sample_rate, bus_width)

    # If we've been asked to dump SGPIO debug info, do so.
    if args.debug_sgpio:
        device.apis.logic_analyzer.dump_sgpio_configuration(False)
        print(device.read_debug_ring(), file=sys.stderr)
        sys.stderr.flush()

    # Print what we're doing and our status.
    log_function("Sampling {} channels at {}Hz.".format(bus_width, eng_notation(sample_rate)))
    log_function("Press Ctrl+C to stop reading data from device.")

    # If we have a target binary file, open the target filename and use that to store samples.
    if args.binary:
        bin_file = open(args.binary, 'wb')
        bin_file_name = args.binary

    # Otherwise, create an temporary file and use that. (It's automatically destroyed on close, which is fancy.)
    elif args.pulseview:
        try:
            holding_dir = os.path.dirname(os.path.abspath(args.pulseview))
        except:
            holding_dir = None

        bin_file = tempfile.NamedTemporaryFile(dir=holding_dir)
        bin_file_name = bin_file.name

    # Create queues of transfer objects that we'll use as a producer/consumer interface for our comm thread.
    empty_buffers = []
    full_buffers  = []

    # Allocate a set of transfer buffers, so we don't have to continuously allocate them.
    for _ in range(DEFAULT_PREALLOCATED_BUFFERS):
        empty_buffers.append(allocate_transfer_buffer(buffer_size))

    # Finally, spawn the thread that will handle our data processing and output.
    termination_request = threading.Event()
    thread_arguments    = (termination_request, args, bus_width, bin_file, empty_buffers, full_buffers)
    data_thread         = threading.Thread(target=background_process_data, args=thread_arguments)

    # Now that we're done with all of that setup, perform our actual sampling, in a tight loop,
    data_thread.start()
    device.apis.logic_analyzer.start()
    start_time = time.time()

    try:
        while True:

            # Grab a transfer buffer from the empty list...
            try:
                transfer_buffer = empty_buffers.pop()
            except IndexError:
                # If we don't have a buffer to fill, allocate a new one. It'll wind up in our buffer pool shortly.
                transfer_buffer = allocate_transfer_buffer(buffer_size)

            # Capture data from the device, and unpack it.
            device.comms.device.read(endpoint, transfer_buffer, 3000)

            # ... and pop it into the to-be-processed queue.
            full_buffers.append(transfer_buffer)

    except KeyboardInterrupt:
        pass
    except usb.core.USBError as e:
        log_error("")
        if e.errno == 32:
            log_error("ERROR: Couldn't pull data from the device fast enough! Aborting.")
            log_error("(Lowering the sample rate may help. Sometimes, switching to another USB bus / port may help.)")
        else:
            log_error("ERROR: Communications failure -- check the connection to -- and state of  -- the GreatFET. ")
            log_error("(More debug information may be available if you run 'gf dmesg').")
            log_error(e)
    finally:
        elapsed_time = time.time() - start_time

        # No matter what, once we're done stop the device from sampling.
        device.apis.logic_analyzer.stop()

        # Signal to our data processing thread that it's time to terminate.
        termination_request.set()

    # Wait for our data processing thread to complete.
    log_function('')
    log_function('Capture terminated -- waiting for data processing to complete.')
    data_thread.join()

    # Flush whatever data we've read to disk, so it can be correctly read by subsequent operations.
    if args.pulseview:
        bin_file.flush()

    # Finally, generate our output.
    if args.binary:
        log_function("Binary data written to file '{}'.".format(args.binary))
    if args.pulseview:
        emit_sigrok_file(args.pulseview, bin_file_name, bus_width, sample_rate, args.first_pin, channel_names)
        log_function("Sigrok/PulseView compatible session file created: '{}'.".format(args.pulseview))

    # Print how long we sampled for, as a nicety.
    log_function("Sampled for {} seconds.".format(round(elapsed_time, 4)))


if __name__ == '__main__':
    main()
