#!/usr/bin/env python
#
# This file is part of GreatFET

from __future__ import print_function

import inspect
import errno
import sys
import re

from greatfet import GreatFET
from greatfet.utils import GreatFETArgumentParser

import IPython

def main():


    # Set up a simple argument parser.
    parser = GreatFETArgumentParser(
        description="Convenience shell for working with GreatFET devices.")
    parser.add_argument('-e', '--exec', metavar="code", type=str, help="Executes the provided code as though it were passed" +
            "to a greatfet shell, and then terminates.", dest="code")

    args = parser.parse_args()
    gf = parser.find_specified_device()

    # Handle any inline execution.
    if args.code:

        # Replace any ;'s with newlines, so we can execute more than one statement.
        code = re.sub(";\s*", "\n", args.code)
        lines = code.split("\n")

        # And execute the code.
        for line in lines:
            result = repr(eval(line))

        # Print the last result and return.
        print(result)
        sys.exit(0)


    # Break into IPython for the shell.
    print("Spwaning an IPython shell for easy access to your GreatFET.")
    print("Like normal python, you can use help(object) to get help for that object.")
    print("Try help(gf.apis.example) to see the documentation for the example API.\n")

    print("A GreatFET object has been created for you as 'gf'. Have fun!\n")
    IPython.start_ipython(user_ns={"gf": gf}, display_banner=False, argv=[])


if __name__ == '__main__':
    main()
