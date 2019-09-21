#
# This file is part of GreatFET
#

from ..interface import GreatFETInterface

class SPIDevice(GreatFETInterface):
    """ Abstract base class representing an SPI-attached device. """


    def __init__(self, spi_bus, chip_select, spi_mode=0):
        """ Sets up a new SPI-attached device.

        Parameters:
            spi_bus     -- The SPI bus to which the given device is attached.
            chip_select -- The GPIOPin object that acts as the chip select for the given device.
            spi_mode    -- The SPI mode to use for the given transaction.
        """

        # Store our interface...
        self._bus = spi_bus
        self._chip_select = chip_select
        self._spi_mode = spi_mode

        # ... and register ourselves with the parent SPI bus.
        self._bus.attach_device(self)


    def _transmit(self, data, receive_length=None, deassert_chip_select=True):
        """
        Sends (and typically receives) data over the SPI bus.

        Args:
            data                 -- the data to be sent to the given device.
            receive_length       -- the total amount of data to be read. If longer
                    than the data length, the transmit will automatically be extended
                    with zeroes.
            deassert_chip_select -- if set, the chip-select line will be left low after
                    communicating; this allows this transcation to be continued in the future
        """
        return self._bus.transmit(data, receive_length, spi_mode=self._spi_mode,
                chip_select=self._chip_select, deassert_chip_select=deassert_chip_select)
