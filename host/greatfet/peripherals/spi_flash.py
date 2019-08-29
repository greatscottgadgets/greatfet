#
# This file is part of GreatFET
#

from ..peripheral import GreatFETPeripheral
from .firmware import DeviceFirmwareManager

from pygreat.comms import CommandFailureError


class SPIFlash(DeviceFirmwareManager, GreatFETPeripheral):
    """ Class representing an SPI flash connected to the GreatFET. """

    def __init__(self, board, autodetect=True, allow_fallback=False
            page_size=256, pages=8192, maximum_address=0x0FFFFF,
            device_id=0, chip_select_port=0x05, chip_select_pin=0x0B):
        """Set up a new SPI flash connection.

        Args:
            board -- The GreatFETBoard that will be programming our flash chip.
            autodetect -- If True, the API will attempt to automatically detect the flash's parameters.
            allow_fallback -- If False, we'll fail if autodetect is set and we can't autodetect the flash's paramters.
                If true, we'll fall-back to the keyword arguments provided
        """

        # Store a reference to the parent board, via which we'll program the
        # the actual SPI flash.
        self.board = board
        self.api = board.apis.spi_flash


        # If autodetect is set to True, we'll try to automatically detect the
        if autodetect:

            # Override the device ID to accept anything, as we're not tied to a specific device.
            device_id = 0

            self.api.initialize(0, 0, 0, chip_select_port, chip_select_pin, device_id)
            try:
                page_size, pages, size_in_bytes = self.api.query_topology()
                maximum_address = size_in_bytes - 1
            except CommandFailureError:
                if not allow_fallback:
                    raise


        # Store the limitations for this SPI flash.
        self.page_size = page_size
        self.maximum_address = maximum_address
        self.api.initialize(page_size, pages, page_size * pages, chip_select_port, chip_select_pin, device_id)

