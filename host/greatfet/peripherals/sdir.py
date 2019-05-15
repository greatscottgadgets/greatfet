
#
# This file is part of GreatFET
#

import array
import usb

from ..peripheral import GreatFETPeripheral
from greatfet.protocol import vendor_requests


class SDIRTransceiver(GreatFETPeripheral):
    """
        Data source for scanning out software-defined IR data.
    """

    USB_TIMEOUT_ERRNO = 110

    def __init__(self, board):
        self.board = board
        self.api = board.apis.sdir
        self.running = False


    def start_receive(self):
        self.api.start_receive()
        self.running = True


    def stop(self):
        self.api.stop()
        self.running = False


    def read(self, timeout=1000, max_data=0x4000, autostart=False, allow_timeout=False):
        """ Reads all available samples from the GreatFET. """

        if not self.running and autostart:
            self.start_receive()

        try:
            return self.board.comms.device.read(0x81, max_data, timeout=timeout)
        except usb.core.USBError as e:
            if (e.errno == self.USB_TIMEOUT_ERRNO) and allow_timeout:
                return None
            else:
                raise

