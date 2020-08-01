#!/usr/bin/env python3
#
# This file is part of GreatFET.
#

from __future__ import print_function, absolute_import

import os
import sys
import time
import queue
import select
import threading

import greatfet

from greatfet import GreatFET
from greatfet.utils import from_eng_notation, GreatFETArgumentParser
from greatfet.util.console import Console

from greatfet.interfaces.uart import UART

console = None
input_thread = None
termination_request = None
last_keycodes = bytearray()


def input_handler(console, input_queue, termination_request):
    """ Thread body that gathers input from the user and enqueues it for processing. """

    def should_check_for_data():
        if os.name == 'posix':
            return select.select([sys.stdin], [], [], 0) != ([], [], [])
        else:
            return True

    while not termination_request.is_set():

        # If we don't have data waiting, skip this iteration.
        # This prevents us from entering a blocking read and sticking there
        # after termination is desired.
        if not should_check_for_data():
            time.sleep(0.01)
            continue

        key = console.getkey()
        input_queue.put(key)


def exit(code):
    termination_request.set()
    input_thread.join()
    console.cleanup()
    sys.exit(code)



def handle_special_functions(keycode):
    """ Handles any special functions associated with the relevant key. """

    global last_keycodes

    # Keep track of the last four keycodes.
    # Add any new keycodes to our list, deleting any existing keys that would push us past 4.
    last_keycodes.extend(keycode)
    while len(last_keycodes) > 2:
        last_keycodes.pop(0)

    # If the user's entered CTRL+A, CTRL+C, exit.
    if last_keycodes.endswith(b"\x01\x03"):
        exit(0)



def main():
    """ Core command. """

    global input_thread, termination_request, console

    parity_modes = {
        'none': UART.PARITY_NONE,
        'odd':  UART.PARITY_ODD,
        'even': UART.PARITY_EVEN,
        'one':  UART.PARITY_STUCK_AT_ONE,
        'zero': UART.PARITY_STUCK_AT_ZERO
    }


    # Set up a simple argument parser.
    # TODO: support configurations such as '8n1'
    parser = GreatFETArgumentParser(description="Simple GreatFET UART monitor.", verbose_by_default=True)
    parser.add_argument('baud', nargs='?', type=from_eng_notation, default=115200, help="Baud rate; in symbols/second. Defaults to 115200.")
    parser.add_argument('-d', '--data',  type=int, default=8, help="The number of data bits per frame.")
    parser.add_argument('-S', '--stop',  type=int, default=1, help="The number of stop bits per frame.")
    parser.add_argument('-P', '--parity', choices=parity_modes, default=0, help="The type of parity to use.")
    parser.add_argument('-E', '--echo', action='store_true', help="If provided, local echo will be enabled.")
    parser.add_argument('-N', '--no-newline-translation', action='store_false', dest='tr_newlines',
        help="Provide this option to disable newline translation.")

    args = parser.parse_args()
    device = parser.find_specified_device()

    # Grab our log functions.
    log_function, log_error = parser.get_log_functions()

    # Configure our UART.
    if not hasattr(device, 'uart'):
        log_error("This device doesn't appear to support the UART API. Perhaps it needs a firmware upgrade?")
        sys.exit(-1)

    # Notify the user that we're entering monitor mode.
    log_function("Entering monitor mode. To terminate, type CTRL+A, then CTRL+C.")

    # Create a console object.
    console = Console()
    console.setup()

    # Create a thread to capture input data into a locally-processed queue.
    input_queue  = queue.Queue()
    termination_request = threading.Event()
    input_thread = threading.Thread(target=input_handler, args=(console, input_queue, termination_request))
    input_thread.start()

    # Configure our UART parameters.
    device.uart.update_parameters(baud=args.baud, data_bits=args.data, stop_bits=args.stop, parity=parity_modes[args.parity])

    # Generate our UART monitor.
    while True:

        # Grab any data from the serial port, and print it to the screen.
        data = device.uart.read()

        # If we're preforming newline translation, prepend a "\r" to any newline.
        if args.tr_newlines and (data == b"\n"):
            console.write_bytes(b"\r")

        # Stick the UART data onscreen.
        console.write_bytes(data)

        # Grab any data from the user, and send it via serial.
        try:
            new_key = input_queue.get_nowait()
            handle_special_functions(new_key)

            # If local echo is on, print the character to our local console.
            if args.echo:
                sys.stdout.buffer.write(new_key)

            if args.tr_newlines and (new_key == b"\n"):
                device.uart.write(b"\r")

            device.uart.write(new_key)
        except queue.Empty:
            pass


if __name__ == '__main__':
    main()
