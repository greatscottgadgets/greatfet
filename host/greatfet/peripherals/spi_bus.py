#
# This file is part of GreatFET
#

from ..protocol import vendor_requests
from ..peripheral import GreatFETPeripheral

class SPIBus(GreatFETPeripheral):
    """
        Class representing a GreatFET SPI bus.

        For now, supports only the second SPI bus (SPI1), as the first controller
        is being used to control the onboard flash.
    """

    class FREQ():
        """
            Set of predefined frequencies used to configure the SPI bus. It
            contains tuple of clock_prescale_rate & serial_clock_rate. All of
            these frequencies assume that the PCLK is set to 204 MHz
        """
        C204000Hz       = (100, 9)
        C408000Hz       = (100, 4)
        C680000Hz       = (100, 2)
        C1020000Hz      = (100, 1)
        C2040000Hz      = (50, 1)
        C4250000Hz      = (24, 1)
        C8500000Hz      = (12, 1)
        C12750000Hz     = (8, 1)
        C17000000Hz     = (6, 1)
        C20400000Hz     = (2, 4)
        C25500000Hz     = (4, 1)
        C34000000Hz     = (2, 2)
        C51000000Hz     = (2, 1)
        C102000000Hz    = (2, 0)



    def __init__(self, board, name='spi bus', buffer_size=255, freq_preset=0,
                 serial_clock_rate=2, clock_prescale_rate=100):
        """
        Initialize a new SPI bus.

        SPI freq is set using either the freq_preset parameter or the 
        combination of serial_clock_rate and clock_prescale_rate parameters.

        When using serial_clock_rate & clock_prescale_rate parameters, the
        resulting frequency will be:
            PCLK / (clock_prescale_rate * [serial_clock_rate+1]).

        Args:
            board -- The GreatFET board whose SPI bus we want to control.
            name -- The display name for the given SPI bus.
            buffer_size -- The size of the SPI receive buffer on the GreatFET.
            freq_preset -- Set clock_prescale_rate and serial_clock_rate using
                one of the frequency presets defined by SPIBus.FREQ
            clock_prescale_rate -- This even value between 2 and 254, by which
                PCLK is divided to yield the prescaler output clock.
            serial_clock_rate -- The number of prescaler-output clocks per bit
                 on the bus, minus one.
        """

        # Store a reference to the parent board.
        self.board = board

        # Store our limitations.
        self.buffer_size = buffer_size

        # Create a list that will store all connected devices.
        self.devices = []

        # Adjust frecuency parameters
        if freq_preset != 0:
            clock_prescale_rate, serial_clock_rate = freq_preset
        freq = serial_clock_rate << 8 | clock_prescale_rate

        # Set up the SPI bus for communications.
        board.comms._vendor_request_out(vendor_requests.SPI_INIT, value=freq)



    def attach_device(self, device):
        """
        Attaches a given SPI device to this bus. Typically called
        by the SPI device as it is constructed.

        Arguments:
            device -- The device object to attach to the given bus.
        """

        # TODO: Check for select pin conflicts; and handle chip select pins.

        self.devices.append(device)



    def transmit(self, data, receive_length=None):
        """
        Sends (and typically receives) data over the SPI bus.

        Args:
            data -- The data to be sent to the given device.
            receive_length -- Returns the total amount of data to be read. If longer
                than the data length, the transmit will automatically be extended
                with zeroes.

        TODO: Support more than one chip-select for more than one device on the bus!
        """

        if receive_length is None:
            receive_length = len(data)

        # If we need to receive more than we've transmitted, extend the data out.
        if receive_length > len(data):
            padding = receive_length - len(data)
            data.extend([0] * padding)

        if len(data) > self.buffer_size:
            raise ValueError("Tried to send/receive more than the size of the receive buffer.");

        # Perform the core transfer...
        self.board.comms._vendor_request_out(vendor_requests.SPI_WRITE, data=data)

        # If reciept was requested, return the received data.
        if receive_length > 0:
            result = self.board.comms._vendor_request_in(vendor_requests.SPI_READ,
                length=receive_length)
        else:
            result = []

        return result
