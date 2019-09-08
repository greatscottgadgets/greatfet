#
# This file is part of GreatFET
#

from enum import IntEnum

from ..spi_device import SPIDevice

class DAC084S085(SPIDevice):
    """ DAC driver for the DAC084S085 three-wire DAC. """

    class Opcode(IntEnum):
        """ Opcodes for the DAC's supported operations. """
        WRITE_WITHOUT_UPDATE = 0
        WRITE_AND_UPDATE     = 1
        WRITE_ALL_AND_UPDATE = 2
        POWER_DOWN           = 3


    class Termination(IntEnum):
        """ Termination modes for the DAC outputs while in powerdown. """
        HIGH_Z        = 0
        PULLDOWN_2K5  = 1
        PULLDOWN_100K = 2

    # Mapping that translates DAC channel names to numbers.
    CHANNEL_MAPPING = {
        'A': 0,
        'B': 1,
        'C': 2,
        'D': 3
    }

    def __init__(self, spi_bus, chip_select, reference_voltage=3.3):
        """ Sets up a new SPI-attached device.

        Parameters:
            spi_bus           -- The SPI bus to which the given device is attached.
            chip_select       -- The GPIOPin object that acts as the chip select for the given device.
            reference_voltage -- The voltage connected to the DAC's VREF pin; necessary to figure out
                                 how to convert voltages into DAC values. Defaults to 3.3V.
        """

        # Store our reference voltage...
        self._reference_voltage = reference_voltage

        # ... and let our parent constructor handle everything else.
        super(DAC084S085, self).__init__(spi_bus, chip_select, spi_mode=1)


    def _issue_command(self, operation, argument, value=0):
        """ Issues a raw command to the DAC; controls most of the DAC's functions. """

        # Build the command word out of the operation, argument, and value...
        command_word = \
            argument  << 14 | \
            operation << 12 | \
            value     << 4

        # ... and transmit it.
        self._transmit(command_word.to_bytes(2, byteorder="big"))


    def set_channel_value(self, channel, value, immediately=True):
        """ Sets one of the DAC's channels by raw value.

        Parameters:
            channel     -- The DAC channel to set; can be 'A', 'B', 'C', 'D'; or an index from 0-3/
            value       -- The value to apply; should be a raw sample between 0 and 255.
            immediately -- If true, the new value will be applied immediately; otherwise the
                           write will be applied with the next "immediate" update. This allows
                           multiple channels to be changed simultaneously.
        """

        # If we have a channel name instead of a number, translate it.
        if channel in self.CHANNEL_MAPPING:
            channel = self.CHANNEL_MAPPING[channel]

        opcode = self.Opcode.WRITE_AND_UPDATE if immediately else self.Opcode.WRITE_ALL_AND_UPDATE
        self._issue_command(opcode, channel, value)



    def set_channel_voltage(self, channel, voltage, immediately=True):
        """ Sets one of the DAC's channels by raw value.

        Parameters:
            channel     -- The DAC channel to set; can be 'A', 'B', 'C', 'D'; or an index from 0-3/
            voltage     -- The voltage we'd like to produce -- should be between 0 and VRef.
            immediately -- If true, the new value will be applied immediately; otherwise the
                           write will be applied with the next "immediate" update. This allows
                           multiple channels to be changed simultaneously.
        """

        if voltage > self._reference_voltage:
            raise ValueError("Cannot produce a voltage greater than VRef ({}).".format(self._reference_voltage))
        if voltage < 0:
            raise ValueError("Cannot produce a negative voltage!")

        # Convert the voltage into the nearest sample value...
        fraction = voltage / self._reference_voltage
        sample_value = round(fraction * 255)

        # ... and then apply that.
        print("sample value is: {}".format(sample_value))
        self.set_channel_value(channel, sample_value, immediately)



    def power_down_outputs(self, termination=Termination.HIGH_Z):
        """ Places the DAC into a low-power state.

        Parameters:
            termination -- The termination to apply to each of the outputs.
                           Should be selected from the Terminations enumeration.
        """
        self._issue_command(self.Opcode.POWER_DOWN, termination)

