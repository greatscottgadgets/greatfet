#
# This file is part of GreatFET
#

from ..peripheral import GreatFETPeripheral

from warnings import warn

# TODOs:
#  - XXX: Overhaul the GPIO(Collection) class to be more efficient
#  - More cleanup to use the GPIOPin model.
#  - Support ranges of pins from the same port (GPIOPort objects?)
#  - Integrate the "reservation" system on the board, so other peripherals
#    (or the board) know what peripherals use which reservations.
#  - Implement a release function so if e.g. an I2C device is no longer in use
#    it releases its pins back to the GPIO pool.

DIRECTION_IN  = 0
DIRECTION_OUT = 1

class GPIO(GreatFETPeripheral):
    """
    Interact with the GPIO lines on a GreatFET board
    """

    def __init__(self, board):
        """
        Args:
            board -- GreatFET board whose GPIO lines are to be controlled
        """
        self.board = board
        self.api = self.board.apis.gpio
        self.pin_mappings = {}

        self.available_pins = []
        self.active_gpio = {}

        # For convenience:
        self.DIRECTION_IN = DIRECTION_IN
        self.DIRECTION_OUT = DIRECTION_OUT

        # TODO: restore GPIO state from board objects


    def register_gpio(self, name, pin, used=False):
        """
        Registers a GPIO pin for later use. Usually only used in board setup.

        Args:
            name -- The name for the GPIO, usually expressed as a position on
                a GreatFET header.
            pin -- A 2-tuple containing the port and pin number on the LPC43xx.
        """

        # Store the full name in our pin mappings.
        self.pin_mappings[name] = pin

        # TODO: Create 'image' objects for pin groupings-- e.g. a J1 object
        # that lets you work with the pins on J1 without qualifying them.

        if not used:
            self.mark_pin_as_unused(name)


    def mark_pin_as_used(self, name):
        """ Marks a pin as used by another peripheral. """

        if name not in self.pin_mappings:
            raise ValueError("Unknown GPIO pin {}".format(name))

        self.available_pins.remove(name)


    def mark_pin_as_unused(self, name):
        """ Mark a pin as no longer used by another peripheral. """

        if name not in self.pin_mappings:
            raise ValueError("Unknown GPIO pin {}".format(name))

        if name not in self.available_pins:
            self.available_pins.append(name)


    def get_available_pins(self, include_active=True):
        """ Returns a list of available GPIO names. """
        available = self.available_pins[:]
        available.extend(self.active_gpio.keys())

        return available


    def get_pin(self, name, unique=False):
        """
        Returns a GPIOPin object by which a given pin can be controlled.

        Args:
            name -- The GPIO name to be used.
            unique -- True if this should fail if a GPIO object for this pin
                already exists.
        """

        # If we already have an active GPIO pin for the relevant name, return it.
        if name in self.active_gpio and not unique:
            return self.active_gpio[name]

        # If the pin's available for GPIO use, grab it.
        if name in self.available_pins:
            port = self.pin_mappings[name]

            self.active_gpio[name] = GPIOPin(self, name, port)
            self.mark_pin_as_used(name)

            return self.active_gpio[name]

        # If we couldn't create the GPIO pin, fail out.
        raise ValueError("No available GPIO pin {}".format(name))


    def release_pin(self, gpio_pin):
        """
        Releases a GPIO pin back to the system for re-use, potentially
        not as a GPIO.
        """

        if gpio_pin.name != self.active_gpio:
            raise ValueError("Trying to release a pin we don't own?")

        # Mark the pin as an input, placing it into High-Z mode.
        # TODO: Disable any pull-ups present on the pin.
        gpio_pin.set_direction(DIRECTION_IN)

        # Remove the GPIO pin from our active array, and add it back to the
        # available pool.
        del self.active_gpio[gpio_pin.name]
        self.mark_pin_as_unused(gpio_pin.name)


    def set_up_pin(self, line, direction, initial_value=False):
        """
        Configure a GPIO line for use as an input or output.  This must be
        called before the line can be used by other functions.

        Args:
            line -- (port, pin); typically a tuple from J1, J2, J7 below
            direction -- Directions.IN (input) or Directions.OUT (output)

        TODO: allow pull-up/pull-down resistors to be configured for inputs
        """
        self.api.set_up_pin(line[0], line[1], direction, initial_value)


    def setup(self, line, direction):
        warn("GPIO.setup is deprecated; prefer set_up_pin.", DeprecationWarning)
        self.set_up_pin(line, direction)

    def set_pin_state(self, line, state):
        """
        Set the state of an output line.  The line must have previously been
        configured as an output using setup().

        Args:
            line -- (port, pin); typically a tuple from J1, J2, J7 below
            state -- True sets line high, False sets line low
        """

        # TODO: validate GPIO direction?

        single_write = (line[0], line[1], state,)
        self.api.write_pins(single_write)


    def output(self, line, state):
        warn("GPIO.output is deprecated; prefer set_pin_state.", DeprecationWarning)
        self.set_pin_state(line, state)


    def read_pin_state(self, line):
        """
        Get the state of an input line.  The line must have previously been
        configured as an input using setup().

        Args:
            line -- (port, pin); typically a tuple from J1, J2, J7 below

        Return:
            bool -- True if line is high, False if line is low
        """

        values = self.api.read_pins(line)
        return values[0]


    def get_pin_direction(self, line):
        """
        Gets the direction of a GPIO pin.

        Args:
            line -- (port, pin); typically a tuple from J1, J2, J7 below

        Return:
            bool -- True if line is an output, False if line is an input
        """

        directions = self.api.get_pin_directions(line)
        return directions[0]


    def input(self, line):
        warn("GPIO.input is deprecated; prefer read_pin_state.", DeprecationWarning)
        return self.read_pin_state(line)


class GPIOPin(object):
    """
    Class representing a single GPIO pin.
    """

    def __init__(self, gpio_collection, name, port_and_pin):
        """
        Creates a new object representing a GPIO Pin. Usually instantiated via
        a GPIO object.

        Args:
            gpio_collection -- The GPIO object to which this pin belongs.
            name -- The name of the given pin. Should match a name registered
                in its GPIO collection.
            port_and_pin -- A 2-tuple containing LPC4330 (port, pin) numbers.

        """

        self.gpio = gpio_collection
        self.name = name
        self.port_and_pin = port_and_pin

        # For convenience:
        self.DIRECTION_IN = DIRECTION_IN
        self.DIRECTION_OUT = DIRECTION_OUT

        # Set up the pin for use. Idempotent.
        self.gpio.set_up_pin(self.port_and_pin, self.get_direction(), self.read())


    def set_direction(self, direction, initial_value=False):
        """
        Sets the GPIO pin to use a given direction.
        """
        self.gpio.set_up_pin(self.port_and_pin, direction, initial_value)


    def get_direction(self):
        """ Returns the pin's direction; will be eitther gpio.DIRECTION_IN or gpio.DIRECTION_OUT """
        return self.gpio.get_pin_direction(self.port_and_pin)


    def is_input(self):
        """ Convenience function that returns True iff the pin is configured as an input. """
        return (self.get_direction() == self.DIRECTION_IN)


    def is_output(self):
        """ Convenience function that returns True iff the pin is configured as an output. """
        return (self.get_direction() == self.DIRECTION_IN)


    def read(self, high_value=True, low_value=False, check_pin_direction=False):
        """ Convenience alias for get_state."""
        return self.get_state(high_value, low_value, check_pin_direction)


    def get_state(self, high_value=True, low_value=False, check_pin_direction=False):
        """ Returns the value of a GPIO pin. """

        if check_pin_direction and not self.is_input():
            raise ValueError("Trying to read from a non-input pin {}! Set up the pin first with set_direction.".format(self.name))

        raw = self.gpio.read_pin_state(self.port_and_pin)
        return high_value if raw else low_value


    def write(self, high, check_direction=False):
        """ Convenience alias for set_state."""
        self.set_state(high, check_direction)


    def set_state(self, high, check_direction=True):
        """ Write a given value to the GPIO port.

        Args:
            high -- True iff the pin should be set to high; the pin will be set
                to low otherwise.
        """

        if check_direction and not self.is_output():
            raise ValueError("Trying to write to a non-output pin {}! Set up the pin first with set_direction.".format(self.name))

        self.gpio.set_pin_state(self.port_and_pin, high)


    def get_port(self):
        """ Returns device's port number. """
        return self.port_and_pin[0]


    def get_pin(self):
        """ Returns pin's pin number within its port. """
        return self.port_and_pin[1]


    # TODO: Toggle-- we have the hardware for this :)

    # TODO: handle pulldowns/pull-ups, etc.
