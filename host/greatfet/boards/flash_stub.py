#
# This file is part of GreatFET
#

from ..board import GreatFETBoard
from ..errors import DeviceNotFoundError

from ..peripherals.gpio import GPIO
from ..peripherals.led import LED
from ..peripherals.i2c_bus import I2CBus
from ..peripherals.spi_bus import SPIBus
from ..peripherals.spi_flash import SPIFlash

from ..glitchkit import *



class GreatFETOne(GreatFETBoard):
    """ Class representing GreatFET One base-boards. """

    # Currently, all GreatFET One boards have an ID of zero.
    HANDLED_BOARD_IDS = [0]
    HANDLED_SERIAL_NUMBERS = ['dfu_flash_stub']
    BOARD_NAME = "GreatFET One (in DFU flash stub mode)"

    def __init__(self, **device_identifiers):
        """ Initialize a new GreatFET One connection. """

        # Set up the core connection.
        super(GreatFETOne, self).__init__(**device_identifiers)

        # The flash stub only supports an onboard flash.
        self.onboard_flash = SPIFlash(self)


    @classmethod
    def accepts_connected_device(cls, **device_identifiers):
        """
        Returns true iff the provided class is appropriate for handling a connected
        GreatFET.

        Accepts the same arguments as pyusb's usb.find() method, allowing narrowing
        to a more specific GreatFET by e.g. serial number.
        """

        try:
            potential_device = cls(**device_identifiers)
        except DeviceNotFoundError:
            return False

        try:
            board_id = potential_device.board_id()
            serial_number = potential_device.usb_serial_number()
        finally:
            potential_device.close()

        # Accept only GreatFET devices whose board IDs are handled by this
        # class. This is mostly used by subclasses, which should override
        # HANDLED_BOARD_IDS.
        board_id_matches = board_id in cls.HANDLED_BOARD_IDS
        serial_no_matches = serial_number in cls.HANDLED_SERIAL_NUMBERS
        return board_id_matches and serial_no_matches
