#
# This file is part of GreatFET
#

from __future__ import print_function

def log_silent(string, end=None):
    """Silently discards all log data, but provides our logging interface."""
    pass

def log_verbose(string, end="\n"):
    """Prints all logging data to the screen."""
    print(string, end=end)
