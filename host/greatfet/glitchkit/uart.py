#
# This file is part of GreatFET
#

from .base import GlitchKitModule

class GlitchKitUART(GlitchKitModule):
    """
    """

    # TODO: Dominic, implement me. :)

    SHORT_NAME = 'uart'


    def __init__(self, board):
        """
        Create a new GlitchKit module allowing triggering on UART events.

        Args:
            board -- A representation of the GreatFET that will perform the actual
                triggering.
        """

        # Store a reference to the parent board.
        self.board = board
