#
# This file is part of GreatFET
#

from ..protocol import vendor_requests

class GlitchKitCollection(object):
    """
    An object that has a reference to all supported GlitchKit modules,
    and methods for the highest-level configuration.
    """

    # Setup commands.
    SETUP_COMMAND_SET_SYNCHRONIZATION = 0
    SETUP_COMMAND_SET_TRIGGER         = 1
    SETUP_COMMAND_ADD_TRIGGER         = 2

    def __init__(self, board):
        """
        Creates a new GlitchKit collection.

        Args:
            board -- The board for which the GlitchKit module can be populated.
        """

        # Store a reference to the board itself.
        self.board = board

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
        self._issue_set_event_command(self.SETUP_COMMAND_SET_SYNCHRONIZATION, *events)


    def trigger_on_events(self, *events):
        """
        Specify a set of GlitchKit events that should issue a trigger to the ChipWhisperer.

        Arguments:
            events -- One or more comma-separated events to use. _Any_ of the events
                      will issue a trigger.
        """
        self._issue_set_event_command(self.SETUP_COMMAND_SET_TRIGGER, *events)


    def add_trigger_events(self, *events):
        """
        Specify a set of GlitchKit events that should issue a trigger to the ChipWhisperer,
        which will be added to the existing event triggers.

        Arguments:
            events -- One or more comma-separated events to use. _Any_ of the events
                      will issue a trigger.
        """
        self._issue_set_event_command(self.SETUP_COMMAND_ADD_TRIGGER, *events)



    def _issue_set_event_command(self, index, *events):
        """
        Issues a GlitchKit SETUP command.

        Arguments:
            events -- One or more comma-separated events to use. _All_ of the events
                      must occur before any active modules will activate.
        """
        flags = 0
        data_to_send = []

        # Compute the flags to be sent, and then convert them into a set of bytes.
        for event in events:
            flags |= event

        # This could just be .to_bytes, if it weren't for you, python2!
        for _ in range(4):
            byte = flags & 0xFF
            flags = flags >> 8

            # Little endian: last bytes first. :)
            data_to_send.append(byte)

        self.board.vendor_request_out(vendor_requests.GLITCHKIT_SETUP, index=index, data=data_to_send)




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
