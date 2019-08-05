#
# This file is part of GreatFET
#

"""
Module containing the core definitions for a GreatFET board.
"""

from .peripherals.led import LED
from .peripherals.gpio import GPIO
from .peripherals.adc import ADC

from .peripherals.m0 import M0Coprocessor
from .peripherals.firmware import DeviceFirmwareManager
from .peripherals.pattern_generator import PatternGenerator
from .peripherals.sdir import SDIRTransceiver


from .glitchkit import *

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

    #
    # Quick way to add simple Python wrappers around comms classes.
    # Create an entry for the relevant comms class, and provide a tuple of
    #   (attr_name, peripheral_class).
    #
    SIMPLE_CLASS_MAPPINGS = {
        'loadables' : ('m0', M0Coprocessor),
        'firmware': ('onboard_flash', DeviceFirmwareManager),
        'pattern_generator': ('pattern_generator', PatternGenerator),
        'sdir': ('sdir', SDIRTransceiver),
        'gpio': ('gpio', GPIO),
        'glitchkit': ('glitchkit', GlitchKitCollection)
    }


    def __init__(self, *args, **kwargs):
        """ Initialize a new GreatFETBoard instance with our additional properties. """

        # Create a new list of peripherals.
        self._peripherals = []

        super(GreatFETBoard, self).__init__(*args, **kwargs)


    def available_peripherals(self):
        """ Returns a list of peripheral properties that exist on this board. """
        return self._peripherals[:]


    def _populate_leds(self, led_count):
        """Adds the standard set of LEDs to the board object.

        Args:
            led_count -- The number of LEDS present on the board.
        """

        self._add_peripheral('leds', {})

        for i in range(1, led_count + 1):
            self.leds[i] = LED(self, i)


    def _populate_gpio(self):
        """Adds GPIO pin definitions to the board's main GPIO object."""

        # Handle each GPIO mapping.
        for name, pin in self.GPIO_MAPPINGS.items():
            self.gpio.register_gpio(name, pin)

    def _populate_adc(self):
        """Adds ADC definitions to the board."""

        # Handle each ADC mapping.
        for name, pin in self.ADC_MAPPINGS.items():
            ADC.register_adc(name, pin)

        self.adc = ADC(self)

    def _add_peripheral(self, name, instance):
        """
        Adds a peripheral to the GreatFET object. Prefer this over adding attributes directly,
        as it adds peripherals to a list that can be queried by the user.

        Arguments:
            name -- The name of the attribute to add to this board. "i2c" would create a
                .i2c property on this board.
            instance -- The object to add as that property.
        """

        self._peripherals.append(name)
        setattr(self, name, instance)


    def _add_simple_peripheral(self, name, cls, *args, **kwargs):
        """ Adds a given peripheral to this board.

        Arguments:
            name -- The attribute name to be added to the board.
            cls -- The class to be instantiated to create the given object.
        """

        # Create an instance of the relevant peripheral class...
        instance = cls(self, *args, **kwargs)

        # ... and add it to this board.
        self._add_peripheral(name, instance)



    def _populate_simple_peripherals(self):
        """ Adds simple peripherals to the board object by parsing the SIMPLE_CLASS_MAPPINGS dictionary. """

        for comms_class, peripheral in self.SIMPLE_CLASS_MAPPINGS.items():

            # If the relevant API is supported, add the relevant peripheral.
            if self.supports_api(comms_class):
                name, python_class = peripheral
                self._add_simple_peripheral(name, python_class)

