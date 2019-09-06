#
# This file is part of GreatFET
#

from ..interface import GreatFETInterface

from warnings import warn


class UART(GreatFETInterface):
    """
    TODO: description
    """

    # Constants that specify our parity mode.
    PARITY_NONE          = 0
    PARITY_ODD           = 1
    PARITY_EVEN          = 2
    PARITY_STUCK_AT_ONE  = 3
    PARITY_STUCK_AT_ZERO = 4

    # Estimate the UART buffer sizes on the target board.
    ESTIMATED_BUFFER_SIZE = 256

    def __init__(self, board, baud=115200, data_bits=8, stop_bits=1, parity=None, uart_number=0):
        """
        Args:
            board -- GreatFET board whose UART lines are to be controlled
        """

        self.api = board.apis.uart

        # We'll only initialize the on-board UART on demand; so we can have a UART object
        # around by default, without necessarily having it
        self.initialized = False

        # Store our UART control parameters.
        self.baud        = baud
        self.data_bits   = data_bits
        self.stop_bits   = stop_bits
        self.parity      = parity or self.PARITY_NONE
        self.uart_number = 0
        self.actual_baud = 0


    def update_parameters(self, baud=None, data_bits=None, stop_bits=None, parity=None):
        """ Updates the UART parameters for the provided board.

        This is intended to be used by providing one or more keyword arguments:
            baud      -- The new baud rate, in symbols/second.
            data_bits -- The number of data bits per frame.
            stop_bits -- The number of stop bits per frame.
            parity    -- One of the PARITY_ constants on this object.

        Any parameters provided will be updated; any parameters not provided will be left the same.
        """

        # Update our parameters based on any field varlues passed in.
        self.baud        = baud if (baud is not None) else self.baud
        self.data_bits   = data_bits if (data_bits is not None) else self.data_bits
        self.stop_bits   = stop_bits if (stop_bits is not None) else self.stop_bits
        self.parity      = parity if (parity is not None) else self.parity

        # Set up the relevant UART.
        self.actual_baud = self.api.initialize(self.uart_number, self.baud, self.data_bits, self.parity, self.stop_bits)

        # Mark ourselves as initialized.
        self.initialized = True


    def read(self, max_length=0):
        """ Reads data from the specified UART. """

        if not self.initialized:
            self.update_parameters()

        max_length = min(max_length, 256)
        return self.api.read(max_length)


    def write(self, data):
        """ Sends data over this UART. """

        if not self.initialized:
            self.update_parameters()

        self.api.synchronous_transmit(self.uart_number, data)

