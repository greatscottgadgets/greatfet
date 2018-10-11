#
# This file is part of GreatFET
#

import array
import struct

from .. import errors
from ..protocol import vendor_requests
from ..peripheral import GreatFETPeripheral

from .firmware import DeviceFirmwareManager

# FIXME: abstract these to their own class!
LIBGREAT_CLASS_SPI_FLASH = 0x11
LIBGREAT_SPI_FLASH_VERB_INIT = 0x00


class SPIFlash(DeviceFirmwareManager, GreatFETPeripheral):
    """ Class representing an SPI flash connected to the GreatFET. """

    def __init__(self, board, page_size=256, pages=8192, maximum_address=0x0FFFFF,
            device_id=0x14, chip_select_port=0x05, chip_select_pin=0x0B):
        """Set up a new SPI flash connection.

        Args:
            board -- The GreatFETBoard that will be programming our flash chip.
        """

        # Store a reference to the parent board, via which we'll program the
        # the actual SPI flash.
        self.board = board
        self.class_number = LIBGREAT_CLASS_SPI_FLASH

        # Store the limitations for this SPI flash.
        self.page_size = page_size
        self.maximum_address = maximum_address

        data = struct.pack("<HHIBBB", page_size, pages, page_size*pages,
                           chip_select_port, chip_select_pin, device_id)
        self.board.execute_command(self.class_number, LIBGREAT_SPI_FLASH_VERB_INIT, data=data)

