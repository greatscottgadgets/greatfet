#
# This file is part of GreatFET
#

from ..board import GreatFETBoard
from ..interfaces.i2c_bus import I2CBus


class Rad1oBadge(GreatFETBoard):
    """ Class representing rad1o base-boards. """

    HANDLED_BOARD_IDS = [2]
    BOARD_NAME = "rad1o badge"

    SUPPORTED_LEDS = 4

    def initialize_apis(self):
        """ Initialize a new rad1o-badge connection. """

        # Set up the core connection.
        super(Rad1oBadge, self).initialize_apis()

        # Create our simple peripherals.
        self._populate_simple_interfaces()

        # Initialize the fixed peripherals that come on the board.
        # Populate the per-board GPIO.
        if self.supports_api("gpio"):
            self._populate_gpio()

        if self.supports_api('i2c'):
            self._add_interface('i2c_busses', [ I2CBus(self, 'I2C0') ])
            self._add_interface('i2c', self.i2c_busses[0])

        # Add objects for each of our LEDs.
        self._populate_leds(self.SUPPORTED_LEDS)



