#
# This file is part of GreatFET
#

"""
Module containing the core definitions for a GreatFET board.
"""

from .peripherals.led import LED
from .peripherals.gpio import GPIO

from pygreat.board import GreatBoard


# Default device identifiers.
GREATFET_VENDOR_ID = 0x1d50
GREATFET_PRODUCT_ID = 0x60e6

# Quirk constant that helps us identify libusb's pipe errors, which bubble
# up as generic USBErrors with errno 32 on affected platforms.
LIBUSB_PIPE_ERROR = 32

# Total seconds we should wait after a reset before reconnecting.
RECONNECT_DELAY = 3


class GreatFETBoard(GreatBoard):
    """
    Class describing GreatFET devices.
    """

    # Default device identifiers.
    BOARD_VENDOR_ID = 0x1d50
    BOARD_PRODUCT_ID = 0x60e6

    """
    The mappings from GPIO names to port numbers. Paths in names can be delineated
    with underscores to group gpios. For example, if Jumper 7, Pin 3 is Port 5, Pin 11,
    you could add an entry that reads "J7_P3": (5, 11).
    """
    GPIO_MAPPINGS = {}

    # FIXME: should these peripherals be in libgreat?

    def _populate_leds(self, led_count):
        """Adds the standard set of LEDs to the board object.

        Args:
            led_count -- The number of LEDS present on the board.
        """
        self.leds = {}
        for i in range(1, led_count + 1):
            self.leds[i] = LED(self, i)


    def _populate_gpio(self):
        """Adds GPIO pin definitions to the board's main GPIO object."""

        self.gpio = GPIO(self)

        # Handle each GPIO mapping.
        for name, pin in self.GPIO_MAPPINGS.items():
            self.gpio.register_gpio(name, pin)


