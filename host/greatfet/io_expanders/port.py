#
# This file is part of GreatFET
#

from __future__ import print_function
from ..io_expander import DIOExpander

class PortExpander(DIOExpander):
    """Example class representing a port based IO expander"""

    def __init__(self, number_pins=4):
        super(PortExpander, self).__init__()

        self.__number_pins = int(number_pins)
        print("Initialized PortExpander")

    @property
    def number_pins(self):
        return self.__number_pins


    def set_direction(self, directions):
        directions = self._validate_port_value(directions)

        # Once the direction has been successfully changed on the IO expander,
        # `_directions` has to be updated.
        self._directions = directions

        print("Directing PortExpander: {0:#0{width}b}".format(directions, width=self.number_pins + 2))

    def set_pin_direction(self, pin_num, direction):
        super(PortExpander, self).set_pin_direction(pin_num, direction)


    def read(self):
        print("Reading PortExpander: {0:#0{width}b}".format(self._pins, width=self.number_pins + 2))
        return self._pins

    def read_pin(self, pin_num):
        return super(PortExpander, self).read_pin(pin_num)


    def write(self, value):
        value = self._validate_port_value(value)

        # Once the value has been successfully sent to the IO expander, `_pins` has to be
        # updated.
        self._pins = value

        print("Writing PortExpander: {0:#0{width}b}".format(value, width=self.number_pins + 2))

    def write_pin(self, pin_num, value):
        super(PortExpander, self).write_pin(pin_num, value)
