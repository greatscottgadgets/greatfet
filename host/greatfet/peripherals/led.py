#
# This file is part of GreatFET
#

from ..peripheral import GreatFETPeripheral


class LED(GreatFETPeripheral):
    """ Simple periheral that allows control of an LED through the GreatFET HAL."""

    def __init__(self, board, led_number):
        """Create a new object representing a GreatFET LED.

        board -- The GreatFET board object that owns the given LED.
        led_number -- The one-indexed LED number. On GreatFET boards, this
                matches the number printed on the silkscreen.
        """

        # Store a reference to the parent board.
        self.board = board

        # Store which of the four(?) LEDs we refer to.
        # TODO: Validate this?
        self.led_number = led_number

    # Function that toggles the relevant LED value. """
    def toggle(self):
        self.board.apis.leds.toggle(self.led_number)

    # Function that turns on the relevant LED value. """
    def on(self):
        self.board.apis.leds.on(self.led_number)

    # Function that turns off the relevant LED value. """
    def off(self):
        self.board.apis.leds.off(self.led_number)

