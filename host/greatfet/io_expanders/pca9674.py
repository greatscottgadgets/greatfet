# coding: utf-8
#
# This file is part of GreatFET
#

from ..io_expander import DIOExpander
from ..peripherals.i2c_device import I2CDevice

class PCA9674(DIOExpander, I2CDevice):
    """PCA9674 I²C I/O expander"""

    def __init__(self, bus, address=0x20):
        """Initialize a new PCA9674 I²C I/O expander.

        Argsuments:
            bus: I²C bus the IO PCA9674 is connected to. For a
                GreatFET One, this would typically be (board.i2c).
            address: Optional; used to specify a non-default address.
                This is the 7 bit address of the PCA9674.
        """
        super(PCA9674, self).__init__()
        I2CDevice.__init__(self, bus, address, 'PCA9674')

        # Power up value of pins with the PCA9674
        self._pins = 0xFF

    @property
    def number_pins(self):
        return 8


    def set_direction(self, value):
        value = self._validate_port_value(value)

        # Invert value
        pin_states = ~value & 2**self.number_pins - 1

        self.transmit(pin_states, 0)
        self._directions = pin_states

    def set_pin_direction(self, pin_num, direction):
        super(PCA9674, self).set_pin_direction(pin_num, direction)


    def read(self):
        return self.transmit([], 1)[0]

    def read_pin(self, pin_num):
        return super(PCA9674, self).read_pin(pin_num)


    def write(self, value):
        value = self._validate_port_value(value)

        self.transmit([value], 0)
        self._pins = value

    def write_pin(self, pin_num, value):
        super(PCA9674, self).write_pin(pin_num, value)
