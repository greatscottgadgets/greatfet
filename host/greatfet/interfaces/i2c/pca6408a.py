#
# This file is part of GreatFET
#

from ..gpio import GPIOProvider, Directions
from .register_based import I2CRegisterBasedDevice

class PCA6048A(I2CRegisterBasedDevice, GPIOProvider):
    """ Class representing a PCA6048A I/O expander. """

    # Default to an address of 0x20. This can also be set to 0x21 by pulling its ADDR pin high.
    DEVICE_ADDRESS = 0x20

    # Register mapping for the device.
    REGISTER_MAP = {
        'INPUT':           0,
        'OUTPUT':          1,
        'INPUT_INVERSION': 2,
        'DIRECTION':       3
    }

    # This I/O expander provides a set of 8 GPIO pins, named P0-P7 on the  device package.
    FIXED_GPIO_PINS = {
        'P0': 0,
        'P1': 1,
        'P2': 2,
        'P3': 3,
        'P4': 4,
        'P5': 5,
        'P6': 6,
        'P7': 7,
    }


    def __init__(self, i2c_bus, device_address=0x20, name_mappings=None):
        """ Creates a new IO Expander device.

        Parameters:
            i2c_bus        -- The I2C bus that this device is on.
            device_address -- The I2C address for the given device.
            name_mappings  -- The mappings of I/O expander pin names to desired GPIO names.
                    See GPIOProvider.__init__ for more information.
        """

        # Call each of our superclass constructors.
        GPIOProvider.__init__(self, name_mappings)
        I2CRegisterBasedDevice.__init__(self, i2c_bus, device_address)


    def set_up_pin(self, line, direction, initial_value=False):
        """
        Configure a GPIO line for use as an input or output.  This must be
        called before the line can be used by other functions.

        Args:
            line      -- The pin's number.
            direction -- Directions.IN (input) or Directions.OUT (output)

        TODO: allow pull-up/pull-down resistors to be configured for inputs
        """

        # Set the pin's initial value before setting its direction.
        if direction == Directions.IN:
            self._set_bit_in_register('OUTPUT', line, initial_value)

        # Set the pin's direction.
        # Note that the PCA6408A uses '1' for input and '0' for output.
        self._set_bit_in_register('DIRECTION', line, direction == Directions.IN)


    def set_pin_state(self, line, state):
        """
        Set the state of an output line.  The line must have previously been
        configured as an output using setup().

        Args:
            line -- The GPIO pin number.
            state -- True sets line high, False sets line low
        """
        self._set_bit_in_register('OUTPUT', line, state)


    def read_pin_state(self, line):
        """
        Get the state of an input line.  The line must have previously been
        configured as an input using setup().

        Args:
            line -- The GPIO pin number.

        Return:
            bool -- True if line is high, False if line is low
        """
        self._get_bit_in_register('INPUT', line)


    def get_pin_direction(self, line):
        """
        Gets the direction of a GPIO pin.

        Args:
            line -- (port, pin); typically a tuple from J1, J2, J7 below

        Return:
            bool -- Directions.IN if line is an output, Directions.OUT if line is an input
        """
        is_input = self._get_bit_in_register('DIRECTION', line)
        return Directions.IN if is_input else Directions.OUT


    def get_pin_port(self, line):
        """ Returns the 'port number' for a given GPIO pin."""

        # All of our pins on this expander are on the same register, so we'll
        # just call this 'register 0'.
        return 0


    def get_pin_identifier(self, line):
        """ Returns the 'pin number' for a given GPIO pin. """
        return line


    def __dir__(self):
        """ Returns a list of the interesting attributes of this object."""

        # Add in our get_pin() function, to allow access to our GPIO pins.
        attrs = super(PCA6048A, self).__dir__()
        attrs.extend(['get_pin', 'get_available_pins'])
        return attrs
