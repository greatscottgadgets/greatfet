#
# This file is part of GreatFET
#

from .. import errors
from ..sensor import GreatFETSensor
from ..peripherals.i2c_device import I2CDevice

class TSL256X(I2CDevice, GreatFETSensor):
    """
        Represents a TSL256X Ambient Light Sensor.
    """

    SHORTHAND = "tsl256x"

    # Default I2C address.
    DEFAULT_ADDRESS = 0x39

    # I2C commands.
    COMMAND_ENABLE_ADC    = 0x80
    COMMAND_GET_DEVICE_ID = 0x8A
    COMMAND_GET_READING   = 0xAC

    # Command arguments.
    ADC_ENABLED = 0x03

    # The maximum brightness reading.
    MAX_ADC_READING = 65535.0

    # Maps device ID numbers to their names.
    DEVICE_NAMES = {
        0: "TSL2560CS",
        1: "TSL2561CS",
        4: "TSL2560T/FN/CL",
        5: "TSL2561T/FN/CL"
    }


    def __init__(self, bus, address=0x39, suffix=''):
        """
        Initialize a new generic TSL256x light sensor.

        Args:
            bus -- An object representing the I2C bus to talk on. For a
                GreatFET One, this would typically be (device.i2c).
            address - Optional; used to specify a non-default address.
            suffix -- Optional suffix for the device name; used to identify
                the device among multiple. No space is added for you.
        """

        # Set up the I2C communications channel used to communicate.
        name = "TSL256x" + suffix
        super(TSL256X, self).__init__(bus, address, name)

        # Set up the sensor for receiving readings.
        self._enable_adc()


    @classmethod
    def create_sensor(cls, board, options=None):
        """
        Factory method that creates a TSL256x using the sensor API.

        This will eventually provide compatibility with e.g. datalogger
        applications.

        Args:
            board -- The GreatFET being configured using the sensor API.
            options -- A dictionary of any options to be provided to the device.
                To be used e.g. on application command lines.
        """

        if not options:
            options = {}

        address = options['address'] if ('address' in options) else cls.DEFAULT_ADDRESS

        default_suffix = "@{}".format(address)
        suffix  = options['suffix'] if ('suffix' in options) else default_suffix

        # Create and return our raw sensor.
        return cls(board.i2c, address, suffix)


    def _enable_adc(self):
        """
        Enables the TSL2561's onboard ADC prior to sampling.
        """

        # Send a full ADC enable...
        response = self.transmit([self.COMMAND_ENABLE_ADC, self.ADC_ENABLED], 1)

        # ... and check to make sure that the device accepted our enable-value.
        result = response[0] & 0x3

        # If we couldn't enable the ADC, throw an IOError.
        if result != self.ADC_ENABLED:
            raise errors.ExternalDeviceError("Could not enable the {} ADC!".format(self.name))


    def _raw_device_id(self):
        """
        Returns the raw contents of the TSL2561's device-and-revision register.
        """
        response = self.transmit([self.COMMAND_GET_DEVICE_ID], 1)
        return response[0]


    def revision_id(self):
        """
        Returns the TSL256x's revision ID.
        """

        id = self._raw_device_id()
        return id & 0xF


    def part_number(self):
        """
        Returns the device's part number.
        """

        id = self._raw_device_id() >> 4

        # If we recognize the device name, return it.
        if id in self.DEVICE_NAMES:
            return self.DEVICE_NAMES[id]
        else:
            return "Unknown"


    def raw_intensity_reading(self):
        """
        Returns an unscaled intensity reading.
        """

        # Get the raw intensity reading from the ADC...
        response = self.transmit([self.COMMAND_GET_READING], 2)

        # ... and convert it to an unscaled number.
        return response[1] | (response[0] << 8)


    def intensity_reading(self):
        """
        Returns a reading of light intensity from the LPC, scaled to a floating-
        point number with a maximum of 1.
        """
        reading = self.raw_intensity_reading()
        return reading / self.MAX_ADC_READING


    def get_reading(self, name_prefix=True):
        """
        Returns an dictionary of all readings provided by the given device.
        Used by the Sensor API.
        """

        # Determine the name of the reading to be captured.
        prefix = (self.name + ': ') if name_prefix else ''
        reading_name = prefix + "Light Intensity"

        # Return our captured reading!
        reading = {}
        reading[reading_name] = self.intensity_reading()
        return reading
