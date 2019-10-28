#!/usr/bin/env python3
# This file is part of GreatFET.
#
""" Bus Pirate emulation for GreatFET. """

from __future__ import print_function

import errno
import sys

import greatfet
from greatfet import GreatFET
from greatfet.utils import GreatFETArgumentParser


def main():
    from greatfet.utils import GreatFETArgumentParser

    supported_interfaces = {
        'spi': lambda d : d.spi,
        'i2c': lambda d : d.i2c
    }
    supported_interface_string = ', '.join(supported_interfaces)

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(description="bus pirate emulation utility for GreatFET", verbose_by_default=True)
    parser.add_argument('interface', nargs='?', help="The type of interface to use for our commands. Currently supported: {}".format(supported_interface_string))
    parser.add_argument('commands', nargs='*', default=[], help="Bus pirate command to execute.")
    args = parser.parse_args()

    log_function, log_error = parser.get_log_functions()
    device = parser.find_specified_device()

    # By default, if we have a command on the command line, run non-interactively.
    if args.commands:
        interface = supported_interfaces[args.interface](device)
        commands = ' '.join(args.commands)

        # Print the executed commands, and their results.
        log_function("> {}".format(commands))
        run_batch(interface, commands, log_function)
    else:
        if args.interface:
            interface = supported_interfaces[args.interface](device)
        else:
            interface = None

            # FIXME: remove this when we support proper mode-switching
            log_error("Mode-switching currently not supported from interactive mode. Specify a mode on the command line.")
            sys.exit(-1)

        run_interactive(interface, print, args.commands)


def run_batch(interface, commands, log_function):
    """ Runs a single bus-pirate command on the command line."""

    # Issue any commands we have...
    results = interface.run_pirate_commands(commands)

    # ... and by default, print our results to the console.
    log_function(', '.join([str(result) for result in results]))


def run_interactive(interface, log_function, commands=None):
    """ Runs a prompt loop that repeatedly reads bus pirate commands. """

    from prompt_toolkit import PromptSession
    from prompt_toolkit.auto_suggest import AutoSuggestFromHistory

    session = PromptSession(auto_suggest=AutoSuggestFromHistory())

    # Print a welcome header.
    print("Bus pirate emulation started! Press CTRL+D to quit.")
    print("Ctrl+C cancels the active command.")
    print("")

    while True:
        try:

            # Read the command from the user...
            mode = interface.INTERFACE_SHORT_NAME if interface else "HiZ"
            commands = session.prompt("{}> ".format(mode)).strip()

            # ... and handle the relevant command.
            if interface:
                run_batch(interface, commands, log_function)
                print()

        except KeyboardInterrupt:
            continue
        except EOFError:
            break






if __name__ == '__main__':
    main()
