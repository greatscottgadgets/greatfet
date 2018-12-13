#
# This file is part of GreatFET
#

from ..board import GreatFETBoard
from ..peripherals.gpio import GPIO
from ..peripherals.i2c_bus import I2CBus
from ..peripherals.spi_bus import SPIBus


class Rad1oBadge(GreatFETBoard):
    """ Class representing GreatFET One base-boards. """

    # Currently, all GreatFET One boards have an ID of zero.
    HANDLED_BOARD_IDS = [2]
    BOARD_NAME = "rad1o badge"

    SUPPORTED_LEDS = 4

    def __init__(self, **device_identifiers):
        """ Initialize a new rad1o-badge connection. """

        # Set up the core connection.
        super(Rad1oBadge, self).__init__(**device_identifiers)

        # Initialize the fixed peripherals that come on the board.
        # TODO: Use a self.add_peripheral mechanism, so peripherals can
        # be dynamically listed?
        self.i2c_busses = [ I2CBus(self, 'I2C0') ]
        self.spi_busses = [ SPIBus(self, 'SPI1') ]

        # Create an easy-to-use alias for the primary busses, for rapid
        # hacking/experimentation.
        self.i2c = self.i2c_busses[0]
        self.spi = self.spi_busses[0]

        self.gpio = GPIO(self)

        # Add objects for each of our LEDs.
        self._populate_leds(self.SUPPORTED_LEDS)



