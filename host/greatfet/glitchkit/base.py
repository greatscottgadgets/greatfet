#
# This file is part of GreatFET
#

from ..protocol import vendor_requests

class GlitchKitCollection(object):
    """
    An object that has a reference to all supported GlitchKit modules,
    and methods for the highest-level configuration.
    """

    # Clock sources constants.
    CLOCK_SOURCE_32K       = 0x00
    CLOCK_SOURCE_IRC       = 0x01
    CLOCK_SOURCE_ENET_RX   = 0x02
    CLOCK_SOURCE_ENET_TX   = 0x03
    CLOCK_SOURCE_GP_CLKIN  = 0x04
    CLOCK_SOURCE_CLKIN     = 0x04
    CLOCK_SOURCE_XTAL      = 0x06
    CLOCK_SOURCE_PLL0USB   = 0x07
    CLOCK_SOURCE_PLL0AUDIO = 0x08
    CLOCK_SOURCE_PLL1      = 0x09
    CLOCK_SOURCE_IDIVA     = 0x0C
    CLOCK_SOURCE_IDIVB     = 0x0D
    CLOCK_SOURCE_IDIVC     = 0x0E
    CLOCK_SOURCE_IDIVD     = 0x0F
    CLOCK_SOURCE_IDIVE     = 0x10

    def __init__(self, board):
        """
        Creates a new GlitchKit collection.

        Args:
            board -- The board for which the GlitchKit module can be populated.
        """

        # Store a reference to the board itself, and to the local API we use.
        self.board = board
        self.api = board.apis.glitchkit

        for cls in GlitchKitModule.__subclasses__():
            if cls.supports_board(board):

                # If we're able to automatically instantiate this module, do so.
                try:
                    self.__dict__[cls.SHORT_NAME] = cls(board)
                except TypeError:
                    pass



    def use_events_for_synchronization(self, *events):
        """
        Specifies passive events (e.g. simple counts) that will need to be met
        before any active events start. These can be used to wait until all of
        the provided events have occurred before starting an active attack.

        Useful for waiting for e.g. a target device to boot. :)

        Arguments:
            events -- One or more comma-separated events to use. _All_ of the events
                      must occur before any active modules will activate.
        """
        self._issue_set_event_command(self.api.set_synchronization_events, *events)


    def trigger_on_events(self, *events):
        """
        Specify a set of GlitchKit events that should issue a trigger to the ChipWhisperer.

        Arguments:
            events -- One or more comma-separated events to use. _Any_ of the events
                      will issue a trigger.
        """
        self._issue_set_event_command(self.api.set_trigger_events, *events)


    def add_trigger_events(self, *events):
        """
        Specify a set of GlitchKit events that should issue a trigger to the ChipWhisperer,
        which will be added to the existing event triggers.

        Arguments:
            events -- One or more comma-separated events to use. _Any_ of the events
                      will issue a trigger.
        """
        self._issue_set_event_command(self.api.add_trigger_events, *events)



    def _issue_set_event_command(self, api_function, *events):
        """
        Issues a GlitchKit SETUP command.

        Arguments:
            api_function -- The API function to call with the provided events.
            events -- One or more comma-separated events to use. _All_ of the events
                      must occur before any active modules will activate.
        """
        flags = 0

        # Compute the event flags to be sent.
        for event in events:
            flags |= event

        # Call the target API function.
        api_function(flags)



    def provide_target_clock(self, clock_source=CLOCK_SOURCE_GP_CLKIN, *events):
        """
        Configure the GreatFET to provide the target clock.
        Currently only supports output to CLK0 (J1_P11); may support more in the future.

        Arguments
            events -- optional; if provided, the clock will not be enabled until all of
                the provided events occur
        """
        flags = 0

        # Compute the flags to be sent, and then convert them into a set of bytes.
        for event in events:
            flags |= event

        self.api.provide_target_clock(clock_source, flags)



class GlitchKitModule(object):
    """
    Generic base class for GlitchKit modules.
    """

    # The short name for the given module.
    SHORT_NAME = None

    @classmethod
    def supports_board(cls, board):
        """ Returns true iff the given class can be instantiated as a peripheral of the given board. """
        return True
