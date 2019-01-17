#!/usr/bin/env python
#
# Root command that delegates to all GreatFET subcommands.

# This file is part of GreatFET.

from __future__ import print_function

import difflib
import errno
import sys
import os

# The prefix which all greatfet-subcommands start with.
GREATFET_PREFIX = 'greatfet_'

def looks_like_valid_greatfet_subcommand(directory, executable):
    """
    Returns true iff the given directory/binary pair seem to represent
    a valid GreatFET subcommand.

    @param directory The directory in the PATH we're currently looking in.
    @param executable The binary name, which should be presnent in the given directory.
    @return True iff the given binary appares to be a valid GreatFET command.
    """

    full_path = os.path.join(directory, executable)

    # Valid GreatFET subcommands start with our prefix.
    if not executable.startswith(GREATFET_PREFIX):
        return False

    # Valid GreatFET subcommands are files.
    if not os.path.isfile(full_path):
        return False

    # Valid GreatFET subcommands are executable.
    if not os.access(full_path, os.X_OK):
        return False

    # If all of the commands above are met, we have what looks like a subcommand.
    return True


def find_all_subcommands():
    """
    Locates all GreatFET subcommands in the user's path -- that is, all
    binaries that start with GREATFET_PREFIX.
    """

    # If we don't have a PATH variable, we can't have subcommands.
    # Bail out.
    if 'PATH' not in os.environ:
        return []

    # Parse the system's PATH variable and get each of the relevant directories.
    sys_path = os.environ['PATH']
    path_entires = sys_path.split(os.pathsep)

    # Search each entry in the path for GreatFET subcommands.
    subcommands = {}
    for directory in path_entires:

        # Skip any path entries that don't seem to be represented on the real system...
        if not os.path.isdir(directory):
            continue

        # ... and search each entry that is a real directory.
        for executable in os.listdir(directory):

            # If the executable seems to be a GreatFET command, use it.
            if looks_like_valid_greatfet_subcommand(directory, executable):

                # Cache the relationships between subcommands and their executables.
                full_path = os.path.join(directory, executable)
                subcommand_name = executable[len(GREATFET_PREFIX):]
                subcommands[subcommand_name] = full_path

    return subcommands


def find_subcommand(name, allow_partial_matches=True, print_errors=True):
    """
    Returns the full path to the current subcommand, if one exists.

    @param name The name of the subcommand to look for.
    @param allow_partial_matches If set, this function will accept abbreviated subcommands.
    @param print_errors If set, errors will be printed.
    @return The full path to the subcommand, or None if none exists.
    """

    subcommands = find_all_subcommands()

    # If we have a direct match, return it.
    if name in subcommands:
        return subcommands[name]

    # If we're accepting partial matches, look for one.
    if allow_partial_matches:
        matches = [subcommand for subcommand in subcommands.keys() if subcommand.startswith(name)]

        # If we've found exactly one, use it.
        if len(matches) == 1:
            return subcommands[matches[0]]

        # Otherwise, print the error.
        elif matches and print_errors:
            matches = "\n\t".join(matches)
            print("Subcommand short-name '{}' is ambiguous; it could refer to:\n\t{}\n".format(name, matches))
            return False

    if print_errors:
        print("ERROR: Unsupported subcommand '{}'.\nCheck to ensure the package providing the " \
                "subcommand is installed.\n".format(name))

    return False


def find_corrections_message(name):
    """
    Generates a message that provides potential corrections to the user if
    their command doesn't match.
    """

    # Find a list of "did you mean" style corrections.
    corrections = difflib.get_close_matches(name, find_all_subcommands())

    # If we didn't find any, don't provide a message.
    if not corrections:
        return ''

    # Otherwise, generate a string that informs the user of their potential error.
    plural_suffix = "s are" if len(corrections) > 1 else " is"
    corrections.insert(0, "The most similar sub-command{}:".format(plural_suffix))
    return "\n\t".join(corrections)


def print_usage(argv):

    # If we don't have argument name information, assume this was called "greatfet"
    name = os.path.basename(argv[0]) if len(argv) else "greatfet"

    print("usage: {} <subcommand>\n".format(name))
    print("Top-level utility for working with GreatFET devices.\n")
    print("supported subcommands:")

    for subcommand in find_all_subcommands():
        print("\t{}".format(subcommand))

    print("\nTo get help for a subcommand, use '{} <subcommand> --help'.".format(name))
    print("For example, for help with the firmware subcommand, use '{} firmware --help'.\n\n".format(name))
    print("You can create and install your own subcommands. Simply create an executable with")
    print("the name greatfet_<subcommand>, and add it to your path. See the (forthcoming)")
    print("documentation for more information.\n")


def main():
    """
    Main routine that delegates all commands to GreatFET subcommands.
    """
    argv = sys.argv[:]

    # If we don't have a subcommand, abort.
    if len(argv) < 2:
        print_usage(argv)
        sys.exit(errno.EINVAL)

    # Get the subcommand name.
    subcommand_name = argv[1]

    # If help is passed as a special-case subcommand, and we have
    # more arguments, help the user out by generating a likely-valid help request.
    if subcommand_name == "help" and len(argv) > 2:
        subcommand_name = argv[2]
        argv[1], argv[2] = argv[2], "--help"

    # Find the binary to execute...
    binary = find_subcommand(subcommand_name)

    # If we couldn't find the relevant binary, print a message.
    if not binary:
        # If there are words that are similar to the relevant word, suggest them as corrections.
        corrections = find_corrections_message(subcommand_name)
        if corrections:
            print(corrections)
            print('')

        sys.exit(errno.EINVAL)


    # Compute the arguments that should be passed to the subcommand,
    # which include the subcommand name and all arguments beyond the subcommand.
    binary_name = os.path.basename(binary)
    arguments = [binary_name] + argv[2:]

    # Pass control entirely to the subcommand.
    sys.exit(os.execv(binary, arguments))


if __name__ == '__main__':
    main()
