#
# This file is part of GreatFET
#

from .board import GreatFETBoard

# Ensure that we have access to all GreatFET boards. Normally, we'd avoid
# importing an entire namespace, but in this case, this allows us to ensure
# that all board modules are loaded for autoidentification.
from .boards import *


def GreatFET(**board_identifiers):
    """
            Attempts to create a new instance of GreatFET board (sub)class
            most applicable to the given device. For example, if the attached
            board is a GreatFET One, this will automatically create a
            GreatFETOne object.

            Accepts the same arguments as pyusb's usb.find() method, allowing narrowing
            to a more specific GreatFET by e.g. serial number. Like usb.find(), providing
            find_all will return a list of all found devices.

            Throws a DeviceNotFoundError if no device is avaiable and find_all is not set.
    """

    if 'find_all' in board_identifiers and board_identifiers['find_all']:
        del board_identifiers['find_all']
        return GreatFETBoard.autodetect_all(**board_identifiers)
    else:
        return GreatFETBoard.autodetect(**board_identifiers)
