#
# This file is part of GreatFET
#


from enum import IntEnum
from warnings import warn

from ..interface import GreatFETInterface


# TODOs:
#  - XXX: Overhaul the GPIO(Collection) class to be more efficient
#  - More cleanup to use the GPIOPin model.
#  - Support ranges of pins from the same port (GPIOPort objects?)
#  - Implement a release function so if e.g. an I2C device is no longer in use
#    it releases its pins back to the GPIO pool.

class Directions(IntEnum):
    IN = 0
    OUT = 1


# Legacy convenience constants.
DIRECTION_IN  = Directions.IN
DIRECTION_OUT = Directions.OUT



class GPIOProvider(GreatFETInterface):
    """ Base class for an object that provides access to GPIO pins. """

    # For convenience.
    DIRECTION_IN  = Directions.IN
    DIRECTION_OUT = Directions.OUT

    # If the subclass has a fixed set of pins, it can override this mapping to
    # specify the fixed pin names to be automatically registered.
    FIXED_GPIO_PINS = {}

    # If the subclass doesn't want to allow external sources to register GPIO pins
    ALLOW_EXTERNAL_REGISTRATION = True


    def __init__(self, name_mappings=None):
        """ Sets up the basic fields for a GPIOProvider.

        Parameters:
            name_mappings -- Allows callers to rename the local / fixed GPIO pin names.
                Optional; accepts a dictionary mapping their fixed names to their new names, or
                to None to remove the relevant pin from the list of available pins.

                This allows instantiators to give a given GPIO collection more specific names, or
                to hide them from general API display/usage.
        """

        if name_mappings is None:
            name_mappings = {}

        # Set up our basic tracking parameters, which track which GPIO pins
        # are available and in use.
        self.pin_mappings   = {}
        self.active_gpio    = {}
        self.available_pins = []

        # Add all of our fixed pins as acceptable GPIO.
        for name, line in self.FIXED_GPIO_PINS.items():

            # If we've been asked to rename the given pin, register it under
            # the new name, rather than under the provided name,
            if name in name_mappings:
                name = name_mappings[name]

            # If our name field winds up remapping to 'None', the instantiator
            # is trying to hide the relevant pin. Skip registering it.
            if name is None:
                continue

            # Register each fixed GPIO.
            self.__register_gpio(name, line)


    def register_gpio(self, name, line, used=False):
        """
        Registers a GPIO pin for later use. Usually only used in board setup.

        Args:
            name -- The name for the GPIO, usually expressed as a position on
                a GreatFET header.
            line -- An abstract argument passed to subclass methods that serves
                to identify the pin. Subclasses often use this to store e.g. port and pin
                numbers.
        """

        # If this class doesn't allow pin registration, raise an error.
        if not self.ALLOW_EXTERNAL_REGISTRATION:
            raise NotImplementedError("This GPIO collection does not allow registration of new pins.")

        # Otherwise, delegate to our internal registration method.
        self.__register_gpio(name, line, used)


    def __register_gpio(self, name, line, used=False):
        """
        Registers a GPIO pin for later use. Usually only used in board setup.

        Args:
            name -- The name for the GPIO, usually expressed as a position on
                a GreatFET header.
            line -- An abstract argument passed to subclass methods that serves
                to identify the pin. Subclasses often use this to store e.g. port and pin
                numbers.
        """

        # Store the full name in our pin mappings.
        self.pin_mappings[name] = line

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



    def get_port(self, *pin_names):
        """ Creates a GPIOPort object that can set multiple pins to a binary value.

        Arguments are a list of pin names to conglomerate into a port, MSB first. This may result in a GPIOPort
        object, or in a derivative class such as a VirtualGPIOPort, depending on the pin locations.
        """

        pins = []

        # Convert each of the header pin names to a GPIOPin object.
        for name in pin_names:
            pins.append(self.get_pin(name))


        # FIXME: apply an optimization for when each pin is on the same logical port:
        return VirtualGPIOPort(pins)


    def release_pin(self, gpio_pin):
        """
        Releases a GPIO pin back to the system for re-use, potentially
        not as a GPIO.
        """

        if gpio_pin.name not in self.active_gpio:
            raise ValueError("Trying to release a pin we don't own!")

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

        Parameters:
            line      -- A unique identifier for the given pin that has meaning to the subclass.
            direction -- Directions.IN (input) or Directions.OUT (output)
        """
        pass


    def set_pin_state(self, line, state):
        """
        Set the state of an output line.  The line must have previously been
        configured as an output using setup().

        Parameters:
            line  -- A unique identifier for the given pin that has meaning to the subclass.
            state -- True sets line high, False sets line low
        """
        pass


    def read_pin_state(self, line):
        """
        Get the state of an input line.  The line must have previously been
        configured as an input using setup().

        Args:
            line  -- A unique identifier for the given pin that has meaning to the subclass.

        Return:
            bool -- True if line is high, False if line is low
        """
        pass


    def get_pin_direction(self, line):
        """
        Gets the direction of a GPIO pin.

        Args:
            line  -- A unique identifier for the given pin that has meaning to the subclass.

        Return:
            bool -- True if line is an output, False if line is an input
        """
        pass


    def get_pin_port(self, line):
        """ Returns the 'port number' for a given GPIO pin.

        For providers for which 'port' isn't a valid semantic concept, this should return
        the same identifier for every pin that can be logically written in a single operation.
        """
        pass


    def get_pin_identifier(self, line):
        """ Returns the 'pin number' for a given GPIO pin.

        This number is typically the 'bit number' in a larger, organized port. For providers
        in which this isn't a valid semantic concept, any convenient semantic identifier (or None)
        is acceptable.
        """
        pass


class GPIO(GPIOProvider):
    """ Work with the GPIO directly present on the GreatFET board. """


    def __init__(self, board):
        """
        Args:
            board -- GreatFET board whose GPIO lines are to be controlled
        """

        # Set up our basic fields...
        super(GPIO, self).__init__()

        # ... and store information about the our low-level connection.
        self.board = board
        self.api   = self.board.apis.gpio

        # TODO: provide functionality to restore GPIO state on reconnect?


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

    def get_pin_port(self, line):
        """ Returns the 'port number' for a given GPIO pin."""
        return line[0]

    def get_pin_identifier(self, line):
        """ Returns the 'pin number' for a given GPIO pin. """
        return line[1]



    #
    # Deprecated methods.
    #
    def output(self, line, state):
        warn("GPIO.output is deprecated; prefer set_pin_state.", DeprecationWarning)
        self.set_pin_state(line, state)

    def input(self, line):
        warn("GPIO.input is deprecated; prefer read_pin_state.", DeprecationWarning)
        return self.read_pin_state(line)

    def setup(self, line, direction):
        warn("GPIO.setup is deprecated; prefer set_up_pin.", DeprecationWarning)
        self.set_up_pin(line, direction)




class GPIOPin(object):
    """
    Class representing a single GPIO pin.
    """

    def __init__(self, gpio_provider, name, line):
        """
        Creates a new object representing a GPIO Pin. Usually instantiated via
        a GPIO object.

        Args:
            gpio_provider -- The GPIO object to which this pin belongs.
            name -- The name of the given pin. Should match a name registered
                in its GPIO collection.
            line -- The pin's 'line' information, as defined by the object that created
                this GPIO pin. This variable has semantic meaning to the GPIO collection;
                but doesn't have any semantic meaning to this class.

        """

        self.name    = name
        self._parent = gpio_provider
        self._line   = line

        # For convenience:
        self.DIRECTION_IN  = Directions.IN
        self.DIRECTION_OUT = Directions.OUT

        # Set up the pin for use. Idempotent.
        self._parent.set_up_pin(self._line, self.get_direction(), self.read())


    def set_direction(self, direction, initial_value=False):
        """
        Sets the GPIO pin to use a given direction.
        """
        self._parent.set_up_pin(self._line, direction, initial_value)


    def get_direction(self):
        """ Returns the pin's direction; will be either Directions.IN or Directions.OUT """
        return self._parent.get_pin_direction(self._line)


    def is_input(self):
        """ Convenience function that returns True iff the pin is configured as an input. """
        return (self.get_direction() == self.DIRECTION_IN)


    def is_output(self):
        """ Convenience function that returns True iff the pin is configured as an output. """
        return (self.get_direction() == self.DIRECTION_OUT)


    def read(self, high_value=True, low_value=False, check_pin_direction=False, set_pin_direction=False):
        """ Convenience alias for get_state."""
        return self.get_state(high_value, low_value, check_pin_direction, set_pin_direction)


    def input(self, high_value=True, low_value=False):
        """ Convenience function that sets the pin to an input and reads its value. """
        return self.read(set_pin_direction=True, high_value=high_value, low_value=low_value)


    def get_state(self, high_value=True, low_value=False, check_pin_direction=False, set_pin_direction=False):
        """ Returns the value of a GPIO pin. """

        # If we're setting the pin direction while we're getting the state, set it.
        if set_pin_direction:
            self.set_direction(self.DIRECTION_IN)

        # Otherwise, enforce the direction, if desired.
        elif check_pin_direction and not self.is_input():
            raise ValueError("Trying to read from a non-input pin {}! Set up the pin first with set_direction.".format(self.name))

        # Finally, read the pin's state.
        raw = self._parent.read_pin_state(self._line)
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

        self._parent.set_pin_state(self._line, high)


    def high(self):
        """ Convenience function that sets the given GPIO pin to both output mode and high, at once. """

        # Note that we can't rely on initial_direction to set the actual port value; as some
        # GPIOProviders may not support that.
        self.set_direction(self.DIRECTION_OUT, True)
        self.write(True)


    def low(self):
        """ Convenience function that sets the given GPIO pin to both output mode and low, at once. """

        # Note that we can't rely on initial_direction to set the actual port value; as some
        # GPIOProviders may not support that.
        self.set_direction(self.DIRECTION_OUT, False)
        self.write(False)


    def get_port(self):
        """ Returns device's port number, if possible. """
        return self._parent.get_pin_port(self._line)


    def get_pin(self):
        """ Returns pin's pin number within its port, if possible. """
        return self._parent.get_pin_identifier(self._line)


    # TODO: Toggle-- we have the hardware for this :)

    # TODO: handle pulldowns/pull-ups, etc.



class VirtualGPIOPort(object):
    """ An object that represents a "virtually contiguous" group of GPIO pins. """


    def __init__(self, *pin_arguments):
        """ Creates a virtual GPIO Port from GPIOPin-compatible objects.

            pins -- A list of pins to be coalesced into a virtual port;
                with the MSB first. For convenience, pins (or lists) can
                also be provided as variadic arguments. Pins should already
                have their directions / resistors set.
        """

        pins = []

        # Take each of our passed in pins/objects, and add them to our ordered list.
        for pin in pin_arguments:
            if isinstance(pin, list):
                pins.extend(pin)
            else:
                pins.append(pin)

        # Reverse the order of our list, so element 0 corresponds to bit zero.
        self.pins = pins[::-1]


    def set_direction(self, word, initial_value=0):
        """ Sets the direction of each individual pin.

        Parameters:
            word -- A number whose bits contain 1s for each bit that should be an output,
                and zeroes for each bit that should be an input.
        """

        for bit, pin in enumerate(self.pins):
            direction = DIRECTION_OUT if (word & (1 << bit)) else DIRECTION_IN
            initial_value = bool(initial_value & (1 << bit))

            pin.set_direction(direction, initial_value=initial_value)


    def all_output(self, initial_value=False):
        """ Sets all of the pins in this port to output mode.

        Parameters:
            initial_value -- Optional; the start value to apply to each pin.
        """

        for pin in self.pins:
            pin.set_direction(DIRECTION_OUT, initial_value)


    def all_input(self):
        """ Sets all of the pins in this port to output mode. """

        for pin in self.pins:
            pin.set_direction(DIRECTION_IN)



    def read(self):
        """ Returns the integer value of the relevant port. """

        value = 0

        # Iterate over each of the contained pins, and add it to our value.
        for bit, pin in enumerate(self.pins):

            # If this pin reads as true, add it to our composite.
            if pin.read():
                value |= (1 << bit)

        return value


    def write(self, value):
        """ Writes a given integer value to the port. """

        for bit, pin in enumerate(self.pins):
            new_value = bool(value & (1 << bit))
            pin.write(new_value)



