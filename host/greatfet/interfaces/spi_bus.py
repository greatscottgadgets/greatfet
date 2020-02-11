#
# This file is part of GreatFET
#

from ..interface import PirateCompatibleInterface


class SPIBus(PirateCompatibleInterface):
    """
        Class representing a GreatFET SPI bus.

        For now, supports only the second SPI bus (SPI1), as the first controller
        is being used to control the onboard flash.
    """


    # Short name for this type of interface.
    INTERFACE_SHORT_NAME = "spi"

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



    def __init__(self, board, chip_select_gpio, name='spi bus', buffer_size=255,
            freq_preset=None, serial_clock_rate=2, clock_prescale_rate=100):
        """
        Initialize a new SPI bus.

        FIXME: There's no reason we shouldn't just take the frequency desired
            and compute it for the user. This API should change soon.

        SPI freq is set using either the freq_preset parameter or the
        combination of serial_clock_rate and clock_prescale_rate parameters.

        When using serial_clock_rate & clock_prescale_rate parameters, the
        resulting frequency will be:
            PCLK / (clock_prescale_rate * [serial_clock_rate+1]).

        Args:
            board               -- The GreatFET board whose SPI bus we want to control.
            name                -- The display name for the given SPI bus.
            chip_select_gpio    -- The GPIOPin object that will represent the bus's default chip select
            buffer_size         -- The size of the SPI receive buffer on the GreatFET.
            freq_preset         -- Set clock_prescale_rate and serial_clock_rate using
                one of the frequency presets defined by SPIBus.FREQ
            clock_prescale_rate -- This even value between 2 and 254, by which
                PCLK is divided to yield the prescaler output clock.
            serial_clock_rate   -- The number of prescaler-output clocks per bit
                 on the bus, minus one.
        """

        # Store a reference to the parent board.
        self.api =board.apis.spi
        self.board = board

        # Store our limitations.
        # TODO: grab these from the board!
        self.buffer_size = buffer_size

        # Create a list that will store all connected devices.
        self.devices = []

        # Store our chip select.
        self._chip_select = chip_select_gpio

        # Apply our frequency information.
        if freq_preset:
            clock_prescale_rate, serial_clock_rate = freq_preset

        # Set up the SPI bus for communications.
        self.api.init(serial_clock_rate, clock_prescale_rate)


    def attach_device(self, device):
        """
        Attaches a given SPI device to this bus. Typically called
        by the SPI device as it is constructed.

        Arguments:
            device -- The device object to attach to the given bus.
        """

        # TODO: Check for select pin conflicts; and handle chip select pins.
        # TODO: replace the device list with a set of weak references

        self.devices.append(device)



    def transmit(self, data, receive_length=None, chip_select=None, deassert_chip_select=True, spi_mode=0):
        """
        Sends (and typically receives) data over the SPI bus.

        Args:
            data                 -- the data to be sent to the given device.
            receive_length       -- the total amount of data to be read. If longer
                    than the data length, the transmit will automatically be extended
                    with zeroes.
            chip_select          -- the GPIOPin object that will serve as the chip select
                    for this transaction, None to use the bus's default, or False to not set CS.
            deassert_chip_select -- if set, the chip-select line will be left low after
                    communicating; this allows this transcation to be continued in the future
            spi_mode             -- The SPI mode number [0-3] to use for the communication. Defaults to 0.
        """

        data_to_transmit = bytearray(data)
        data_received = bytearray()

        # If we weren't provided with a chip-select, use the bus's default.
        if chip_select is None:
            chip_select = self._chip_select

        if receive_length is None:
            receive_length = len(data)

        # If we need to receive more than we've transmitted, extend the data out.
        if receive_length > len(data):
            padding = receive_length - len(data)
            data_to_transmit.extend([0] * padding)

        # Set the polarity and phase (the "SPI mode").
        self.api.set_clock_polarity_and_phase(spi_mode)

        # Bring the relevant chip select low, to start the transaction.
        if chip_select:
            chip_select.low()

        # Transmit our data in chunks of the buffer size.
        while data_to_transmit:

            # Extract a single data chunk from the transmit buffer.
            chunk = data_to_transmit[0:self.buffer_size]
            del data_to_transmit[0:self.buffer_size]

            # Finally, exchange the data.
            response = self.api.clock_data(len(chunk), bytes(chunk))
            data_received.extend(response)


        # Finally, unless the caller has requested we keep chip-select asserted,
        # finish the transaction by releasing chip select.
        if chip_select and deassert_chip_select:
            chip_select.high()

        # Once we're done, return the data received.
        return bytes(data_received)


    def disable_drive(self):
        """ Tristates each of the pins on the given SPI bus. """
        self.api.enable_drive(False)


    def enable_drive(self):
        """ Enables the bus to drive each of its output pins. """
        self.api.enable_drive(True)


    #
    # Support methods to support bus pirate commands.
    #


    def _handle_pirate_read(self, length, ends_transaction=False):
        """ Performs a bus-pirate read of the given length, and returns a list of numeric values. """

        data_bytes = self.transmit(b"", receive_length=length, chip_select=False)
        return list(data_bytes)


    def _handle_pirate_write(self, data, ends_transaction=False):
        """ Performs a bus-pirate transmit of the given length, and returns a list of numeric values. """

        data_bytes = self.transmit(data, chip_select=False)
        return list(data_bytes)


    def _handle_pirate_start(self):
        """ Starts a given communication by performing any start conditions present on the interface. """
        self._chip_select.low()


    def _handle_pirate_stop(self):
        """ Starts a given communication by performing any start conditions present on the interface. """
        self._chip_select.high()
