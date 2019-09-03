#
# This file is part of GreatFET
#

from .board import GreatFETBoard

# Ensure that we have access to all GreatFET boards. Normally, we'd avoid
# importing an entire namespace, but in this case, this allows us to ensure
# that all board modules are loaded for autoidentification.
from .boards import *

active_connections = {}


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


def GreatFETSingleton(serial=None):
    """ Returns a GreatFET object, re-using an existing object if we already have a connection to the given GreatFET. """

    # If we already have a GreatFET with the given serial,
    if serial in active_connections:
        device = active_connections[serial]
        if device.comms.still_connected():
            return device

    # Otherwise, try to create a new GreatFET instance.
    greatfet = GreatFET(serial_number=serial)
    active_connections[serial] = greatfet


    return greatfet

