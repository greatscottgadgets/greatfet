#
# This file is part of GreatFET
#

from ..board import GreatFETBoard
from ..peripherals.spi_flash import SPIFlash


class NXPXplorer(GreatFETBoard):
    """ Class representing the GreatFET varian of the NXP Xplorer base-boards. """

    # Currently, all GreatFET One boards have an ID of zero.
    HANDLED_BOARD_IDS = [1]
    BOARD_NAME = "NXP Xplorer"

    # We only have two LEDs.
    SUPPORTED_LEDS = 2

    def __init__(self, **device_identifiers):
        """ Initialize a new GreatFET connection. """

        # Set up the core connection.
        super(NXPXplorer, self).__init__(**device_identifiers)

        self.onboard_flash = SPIFlash(self, device_id=0x15, pages=16384, maximum_address=0x3FFFFF)

        # Add objects for each of our LEDs.
        self._populate_leds(self.SUPPORTED_LEDS)
