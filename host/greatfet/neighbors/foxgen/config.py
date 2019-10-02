#
# This file is part of GreatFET.
#

import re

from nmigen import Module, Elaboratable
from .platform.foxglove_r0_1 import FoxglovePlatformR01


# Simple (heh) regular expression that parses indexed names for ease-of-use.
SIMPLE_IDENTIFIER_PARSER = r'([^[]+)(?:\[(\d+)(?::(\d+))?])?'


class FoxgloveConfig(Elaboratable):
    """ Class representing an interface configuration for Foxglove.

    This class is used to describe the I/O interfaces that "flow through"
    Foxglove; whether the interface exists on the base-board, or whether
    Foxglove takes a more active role in the interface drive.

    Instances of this class dynamically build FPGA gateware using nMigen;
    and accordingly require Python3 and an installed ECP5 toolchain.
    """

    DEFAULT_PLATFORM = FoxglovePlatformR01


    def __init__(self, platform=None, name="generated"):
        """ Creates a new configuration.

        Parameters:
            platform -- The Foxglove platform to target, or None to target the
                latest canonical Foxglove board.
        """

        if platform is None:
            platform = self.DEFAULT_PLATFORM


        # Instantiate the target platform.
        self._platform = platform()

        # ... and store our parameters.
        self._name = name

        # Create a new module for the given configuration.
        self._module = Module()


    def _platform_equivalent_for_name(self, name):
        """ Translates a provided pin name to a platform-given name.

        Parameters:
            name -- The string name to be parsed. For convenience, also
                    accepts a platform-equivalent tuple, which will be encapsulated
                    and returned.

        Returns a list of tuples that can each be passed to platform.request().
        """

        # If this is a tuple, we don't need to do anything.
        if isinstance(name, tuple):
            return [name]

        # Parse the relevant index.
        matches = re.match(SIMPLE_IDENTIFIER_PARSER, name)
        if matches is None:
            raise ValueError("Couldn't parse the name {}!\n".format(name))

        name, low, high = matches.group(1, 2, 3)

        # If we don't have a low bound, return just the name.
        if low is None:
            return [(name, )]

        # If we have a low bound, but no high bound, return just the
        if high is None:
            return [(name, int(low, 0))]


        # If we have both a low and high bound, return tuples for each of the relevan bits.
        return [(name, i) for i in range(int(low, 0), int(high, 0))]




    def _get_platform_pins(self, identifier, direction=None, *args, **kwargs):
        """ Retrieves the platform pins for the given name-string or argument tuple.

        Paramters:
            identifier -- The identifier to be converted to platform pins.

        All other arguments are passed to each invocation of platform.request.

        Returns a list of pins, even if the name only indicates a single pin.
        """

        name_args = self._platform_equivalent_for_name(identifier)
        return [self._get_pin_with_direction(signal_args, direction, *args, **kwargs) for signal_args in name_args]



    def _get_pin_with_direction(self, pin_argument_tuple, direction, *args, attrs=None, **kwargs):
        """ Requests the provided pin to be configured with a given direction. """

        # Fetch the raw pin, so we can examine it.
        raw_pin = self._platform.lookup(*pin_argument_tuple)

        # If our pin has a level/shifter and attachments, we'll need to set those up.
        if self._pin_has_conditioning_attributes(raw_pin):

            # Grab the pin...
            pin = self._platform.request(*pin_argument_tuple, dir={'io': direction})

            # ... and set up its level shifter's direction.
            direction_signal = 1 if direction == 'o' else 0
            self._module.comb += pin.direction.o.eq(direction_signal)


            # Return just the pin's I/O signal.
            return pin.io

        else:
            return self._platform.request(*pin_argument_tuple, dir=direction)



    def _pin_has_conditioning_attributes(self, resource):
        """ Returns true iff the given pin has Foxglove signal-conditioning attribtues. """
        return hasattr(resource, 'connections')




    def route(self, source, destination, attrs=None):
        """ Routes a given pin across the FPGA.

        Parameters:
            source      -- The I/O pin to route from; accepts string names (e.g. A[2] or A[2:0]),
                           or a tuple that accepts the same argument format as platform.request.
            destination -- The I/O pin to route to; in the same format as the source.

        Name strings can be in one of three formats:
            "name"      -- Grabs the I/O named 'name'; including all bits if this is a bus signal. For example,
                           'port_b_bus' would grab all bits of the bus named 'port_b_bus', while a name of a single
                           signal would grab that signal.
            "name[n]"   -- Grabs the n-th bit of the name signal. For example, 'port_a[3]' would grab the third bit
                           of 'port_a'.
            "name[l:h)" -- Grabs the range of bits from [low, high). Note that, like Python, the high bound is
                           exclusive.

        The source and destination _must_ have the same width.
        """

        m = self._module

        if attrs is None:
            attrs = {}

        # Grab pin objects for the source and destination pins.
        in_pins  = self._get_platform_pins(source, dir='i')
        out_pins = self._get_platform_pins(destination, dir='o')

        # Ensure we have the same general argument length.
        assert len(in_pins) == len(out_pins)

        # Connect each of the in-pins to each of the out-pins.
        # TODO: assert each signal is the same size

        # Create our interconnections.
        for in_pin, out_pin in zip(in_pins, out_pins):
            m.d.comb += out_pin.o.eq(in_pin.i)


    def elaborate(self, platform):
        """ Returns a generated module that represents our configuration. """

        # We've already built our module, so we only need to return it.
        return self._module



    def to_bitstream(self):
        """ Creates gateware that implements the relevant configuration, and returns its bitstream. """

        # Build the gateware itself for this configuration...
        products = self._platform.build(self, name=self._name, do_program=False)
        return products.get('{}.bit'.format(self._name))
