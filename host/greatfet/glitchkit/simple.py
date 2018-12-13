#
# This file is part of GreatFET
#

from .base import GlitchKitModule
from ..protocol import vendor_requests
from ..peripherals.gpio import GPIOPin

class GlitchKitSimple(GlitchKitModule):
    """
    Simple trigger module for GlitchKit. Provides simple trigger conditions
    (e.g. "the SPI clock ticks 25 times while CE is high and WP is low") for quick construction
    of simple glitch conditions. This is suprisingly useful for how simple it is. :)
    """

    SHORT_NAME = 'simple'

    # Event flags.
    COUNT_REACHED = 0x001

    # Store the max number of each type of condition we can have.
    MAX_EDGE_CONDITIONS  = 8
    MAX_LEVEL_CONDITIONS = 8

    # The types of conditions we support.
    # These must match the enumeration in firmware/coimmon/gpio_int.h.
    CONDITION_TYPES = {
        'DISABLED'     : -1, # for convenience, we'll ignore these : )
        'LEVEL_HIGH'   : 0,
        'LEVEL_LOW'    : 1,
        'EDGE_RISING'  : 2,
        'EDGE_FALLING' : 3,
        'EDGE_BOTH'    : 4
    }


    def __init__(self, board):
        """
        Create a new GlitchKit module allowing triggering on simple events.

        Args:
            board -- A representation of the GreatFET that will perform the actual
                triggering.
        """

        # Store a reference to the parent board.
        self.board = board



    def prime_trigger_on_event_count(self, count, conditions):

        # TODO: get rid of this alias
        self.watch_for_event(condition, count)


    def watch_for_event(self, count, conditions):
        """
        Sets up the GreatFET to issue an event when a given number of events have passed.

        Args:
            count -- The count to reach.
            conditions -- A list of 2-tuples, which should each contain a value
                a key from self.CONDITION_TYPES, and a GPIOPin or GPIOPin name.
        """

        # Build a packet that describes each of the conditions to be described.
        # This is the bulk of the communication.
        packet = self._build_condition_packet(conditions)

        # Split the target count into two, so we can wedge it into the setup
        # fields.
        count_high = count >> 16
        count_low  = count & 0xFFFF

        # Finally, issue the raw request that should generate the relevant count.
        self.board.comms._vendor_request_out(vendor_requests.GLITCHKIT_SIMPLE_START_EVENT_COUNT,
                value=count_high, index=count_low, data=packet, timeout=3000)


    def _build_condition_packet(self, conditions, ensure_input=True):
        """
        Builds a packet that can communicate a list of conditions to the GreatFET.

        Args:
            conditions -- A list of 2-tuples, which should each contain
                a _key_ from self.CONDITION_TYPES, and a GPIOPin or GPIOPin name.
            ensure_input -- If set, this will ensure that all relevant GPIOPins are
                set to input mode before using them for glitching.

        Returns:
            A list of bytes that will represent the given command.
        """

        packet = []

        # Keep track of the number of each type of conditions obeserved
        num_edge = 0
        num_level = 0

        # Convert each condition to a set of bytes, and add it to the packet.
        for condition in conditions:

            # Skip any DISABLED conditions.
            if condition[0] == 'DISABLED':
                continue
            elif condition[0] in ['LEVEL_HIGH', 'LEVEL_LOW']:
                num_level += 1
            elif condition[0] in ['EDGE_RISING', 'EDGE_FALLING', 'EDGE_BOTH']:
                num_edge += 1
            else:
                raise ValueError("Invalid condition type {}!".format(condition[0]))

            # Check to ensure we're not going to genete more packets that the device can handle.
            if (num_level > self.MAX_LEVEL_CONDITIONS) or (num_edge > self.MAX_EDGE_CONDITIONS):
                raise ValueError('Too many conditions!')

            command = self._build_condition_triple(condition, ensure_input)
            packet.extend(command)

        return packet


    def _build_condition_triple(self, condition, ensure_input=True):
        """
        Converts a condition into a format that the GreatFET simple glitching module
        will accept.

        Args:
            condition-- A single 2-tuple, which should contain a _key_ from
                self.CONDITION_TYPES, and a GPIOPin or GPIOPin name.
            ensure_input -- If set, this will ensure that the relevant GPIOPins is
                set to input mode before using them for glitching.

        Returns:
            A list of bytes that will represent the given command.
        """

        raw_mode, pin_or_name = condition

        # Resolve the arguments to the types we'll need to communicate to the GreatFET.
        mode        = self.CONDITION_TYPES[raw_mode]
        gpio_pin    = pin_or_name if isinstance(pin_or_name, GPIOPin) else self.board.gpio.get_pin(pin_or_name)
        port_number = gpio_pin.get_port()
        pin_number  = gpio_pin.get_pin()

        # If we're ensuring the relevant pin is an input, configure it accordingly.
        if ensure_input:
            gpio_pin.set_direction(self.board.gpio.DIRECTION_IN)

        # Finally, return the configuration list.
        return [mode, port_number, pin_number]

