#
# This file is part of GreatFET
#

from .base import GlitchKitModule
from ..protocol import vendor_requests

class GlitchKitUSB(GlitchKitModule):
    """
    """

    # TODO: Kate, implement me. :)

    SHORT_NAME = 'usb'


    def __init__(self, board):
        """
        Create a new GlitchKit module allowing inducing or waiting for USB
        events, and then glitching.

        Args:
            board -- A representation of the GreatFET that will perform the actual
                triggering.
        """

        # Store a reference to the parent board.
        self.board = board

