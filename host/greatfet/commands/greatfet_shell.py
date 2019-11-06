#!/usr/bin/env python
#
# This file is part of GreatFET

from __future__ import print_function

import re
import sys
import errno
import inspect
import argparse

from greatfet import GreatFET
from greatfet.utils import GreatFETArgumentParser
from greatfet.util.interactive import GreatFETShellMagics

import IPython
from IPython.terminal.interactiveshell import TerminalInteractiveShell

def main():

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(
        description="Convenience shell for working with GreatFET devices.")
    parser.add_argument('-e', '--exec', metavar="code", type=str, help="Executes the provided code as though it were passed " +
            "to a greatfet shell, and then terminates.", dest="code")
    parser.add_argument('-E', '--pre-exec', metavar="code", type=str, help="Executes the provided code as though it were passed " +
            "to a greatfet shell, but does not explicitly terminate.", dest="prelude")
    parser.add_argument('-f', '--file', metavar="file", type=str, help="Executes the relevant file before starting the given shell.")
    parser.add_argument('-M', '--automagic', dest="automagic", action='store_true',
            help="Enable automagic, so lazy developers don't have to type %%.")
    parser.add_argument('-P', '--avoid-parens', dest="avoidparens", action='store_true',
            help="Enable full autocall, so bare methods are executed, rather than printed.")
    parser.add_argument('-A', '--autoreload', dest="autoreload", action='store_true',
            help="Attempts to reload python modules automatically as they change; so current objects import new functionality. This may sometimes break your shell.")
    parser.add_argument('-S', '--singleton', dest="singleton", action='store_true',
            help="Connect via a singleton that persists across device reconnects. Note: device state is not preserved.")

    args = parser.parse_args()

    if args.singleton:
        connect_function = parser.get_singleton_for_specified_device
    else:
        connect_function = parser.find_specified_device

    gf = connect_function()

    # Break into IPython for the shell.
    if not args.code:
        print("Spawning an IPython shell for easy access to your GreatFET board.")
        print("Like normal python, you can use help(object) to get help for that object.\n")

        print("Try help(gf.gpio) to see the documentation for the GreatFET GPIO;")
        print("try dir(gf) to see a list of properties on the GreatFET object, and")
        print("try gf.available_interfaces() and gf.available_programmers() to see")
        print("the interfaces you can work with, and the programmers you can create.\n")

        print("This GreatFET shell is *magical*. Try some of our IPython magics:\n")
        print("\t %dmesg      -- prints the GreatFET's debug ring (log)")
        print("\t %reconnect  -- tries to reconnect to the current GreatFET")
        print("\t %makeflash  -- when run from a firmware build dir, builds and flashes your GreatFET")
        print("\t %reload     -- tries to reload your host python code; useful with $PYTHONPATH")
        print("\t %refet      -- (not %reset) resets and reconnects to the current GreatFET")
        print("\t                [hey, %reset was taken!]\n\n")


        singleton_text = "singleton " if args.singleton else ""
        print("A GreatFET {}object has been created for you as 'gf'. Have fun!\n".format(singleton_text))


    # Create a new shell, and give it access to our created GreatFET object.
    shell = TerminalInteractiveShell()
    shell.push('gf')

    # Create nice aliases for our primary interfaces.
    i2c =  gf.i2c
    spi =  gf.spi
    adc =  gf.adc
    uart = gf.uart
    gpio = gf.gpio
    shell.push(('i2c', 'spi', 'adc', 'uart', 'gpio',))

    # Make the autoreload extension available.
    shell.extension_manager.load_extension('autoreload')

    # Add our magic commands, to make execution more 'fun'.
    shell.register_magics(GreatFETShellMagics)

    # If the user has requested automagic, let them have their automagic.
    if args.automagic:
        shell.automagic = True

    # If we're in avoid parenthesis mode
    if args.avoidparens:
        shell.autocall = 2

    # If we're using autoreload, enable that.
    if args.autoreload:
        shell.run_cell('%autoreload 2')
        print("Heads up: you've enabled autoreload. Things make break in unexpected ways as your code changes.")
        print("You can fix this by adjusting your expectations regarding breakage.\n")

    # Handle any inline execution requested.
    if args.code or args.prelude:

        # Replace any ;'s with newlines, so we can execute more than one statement.
        code = args.code or args.prelude
        code = re.sub(r";\s*", "\n", code)
        lines = code.split("\n")

        # If we're in execute-and-quit mode, do so.

        for line in lines:
            shell.run_cell(line, shell_futures=True)

        # If we're to exit after running the relevant code, do so.
        if args.code:
            sys.exit(0)

    # If we have a file to execute, execute it.
    if args.file:
        shell.safe_execfile_ipy(args.file, shell_futures=True, raise_exceptions=True)

    # Run the shell itself.
    shell.connect_function = connect_function
    shell.mainloop()


if __name__ == '__main__':
    main()
