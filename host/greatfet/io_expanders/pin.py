#
# This file is part of GreatFET
#

from __future__ import print_function
from ..io_expander import DIOExpander

class PinExpander(DIOExpander):
    """Example class representing a pin based IO expander"""

    def __init__(self, number_pins=4):
        super(PinExpander, self).__init__()

        self.__number_pins = int(number_pins)
        print("Initialized PinExpander")

    @property
    def number_pins(self):
        return self.__number_pins


    def set_direction(self, directions):
        super(PinExpander, self).set_direction(directions)

    def set_pin_direction(self, pin_num, direction):
        pin_num = self._validate_pin_number(pin_num)

        # Once the direction has been successfully changed on the IO expander,
        # `_directions` has to be updated.
        self._directions = self._set_bit(self._directions, pin_num, direction)

        print("Directing PinExpander:  pin: {} direction: {}".format(pin_num, direction))


    def read(self):
        return super(PinExpander, self).read()

    def read_pin(self, pin_num):
        pin_num = self._validate_pin_number(pin_num)

        pin_value = self._git_bit(self._pins, pin_num)
        print("Reading PinExpander:  pin: {} value: {}".format(pin_num, pin_value))

        return pin_value


    def write(self, value):
        super(PinExpander, self).write(value)

    def write_pin(self, pin_num, value):
        pin_num, value = self._validate_pin_args(pin_num, value)

        # Once the value has been successfully sent to the IO expander, `_pins`
        # has to be updated.
        self._pins = self._set_bit(self._pins, pin_num, value)

        print("Writing PinExpander:  pin: {} value: {}".format(pin_num, value))
