#
# This file is part of GreatFET
#

from .. import errors
from ..sensor import GreatFETSensor
from ..interfaces.i2c_device import I2CDevice
from enum import Enum

class LSM6DS33(I2CDevice, GreatFETSensor):
    """
        Represents a LSM6DS33 IMU.
    """

    SHORTHAND = "lsm6ds33"

    # Default I2C address
    DEFAULT_ADDRESS = 0x6B

    # Who Am I register return value
    WHO_AMI_I = 0x69

    # Device registers
    REG_FUNC_CFG_ACCESS = 0x01
    REG_FIFO_CTRL1 = 0x06
    REG_FIFO_CTRL2 = 0x07
    REG_FIFO_CTRL3 = 0x08
    REG_FIFO_CTRL4 = 0x09
    REG_FIFO_CTRL5 = 0x0A
    REG_ORIENT_CFG_G = 0x0B
    REG_INT1_CTRL = 0x0D
    REG_INT2_CTRL = 0x0E
    REG_WHO_AM_I = 0x0F
    REG_CTRL1_XL = 0x10
    REG_CTRL2_G = 0x11
    REG_CTRL3_C = 0x12
    REG_CTRL4_C = 0x13
    REG_CTRL5_C = 0x14
    REG_CTRL6_C = 0x15
    REG_CTRL7_C = 0x16
    REG_CTRL8_C = 0x17
    REG_CTRL9_C = 0x18
    REG_CTRL10_C = 0x19
    REG_WAKE_UP_SRC = 0x1B
    REG_TAP_SRC = 0x1C
    REG_D6D_SRC = 0x1D
    REG_STATUS_REG = 0x1E
    REG_OUT_TEMP_L = 0x20
    REG_OUT_TEMP = 0x21
    REG_OUTX_L_G = 0x22
    REG_OUTX_H_G = 0x23
    REG_OUTY_L_G = 0x24
    REG_OUTY_H_G = 0x25
    REG_OUTZ_L_G = 0x26
    REG_OUTZ_H_G = 0x27
    REG_OUTX_L_XL = 0x28
    REG_OUTX_H_XL = 0x29
    REG_OUTY_L_XL = 0x2A
    REG_OUTY_H_XL = 0x2B
    REG_OUTZ_L_XL = 0x2C
    REG_OUTZ_H_XL = 0x2D
    REG_FIFO_STATUS1 = 0x3A
    REG_FIFO_STATUS2 = 0x3B
    REG_FIFO_STATUS3 = 0x3C
    REG_FIFO_STATUS4 = 0x3D
    REG_FIFO_DATA_OUT_L = 0x3E
    REG_FIFO_DATA_OUT_H = 0x3F
    REG_TIMESTAMP0_REG = 0x40
    REG_TIMESTAMP1_REG = 0x41
    REG_TIMESTAMP2_REG = 0x42
    REG_STEP_TIMESTAMP_L = 0x49
    REG_STEP_TIMESTAMP_H = 0x4A
    REG_STEP_COUNTER_L = 0x4B
    REG_STEP_COUNTER_H = 0x4C
    REG_FUNC_SRC = 0x53
    REG_TAP_CFG = 0x58
    REG_TAP_THS_6D = 0x59
    REG_INT_DUR2 = 0x5A
    REG_WAKE_UP_THS = 0x5B
    REG_WAKE_UP_DUR = 0x5C
    REG_FREE_FALL = 0x5D
    REG_MD1_CFG = 0x5E
    REG_MD2_CFG = 0x5F
    REG_PEDO_THS_REG = 0x0F
    REG_SM_THS = 0x13
    REG_PEDO_DEB_REG = 0x14
    REG_STEP_COUNT_DELTA = 0x15

    def __init__(self, bus, address=DEFAULT_ADDRESS, suffix=''):
        """
        Initialize a new generic LSM6DS33 sensor.

        Args:
            bus -- An object representing the I2C bus to talk on. For a
                GreatFET One, this would typically be (device.i2c).
            address - Optional; used to specify a non-default address.
            suffix -- Optional suffix for the device name; used to identify
                the device among multiple. No space is added for you.
        """

        # Set up the I2C communications channel used to communicate.
        name = "LSM6DS33" + suffix
        super(LSM6DS33, self).__init__(bus, address, name)

        # Get device ID as sensor presence check
        self._dev_id = self._who_am_i()

    @classmethod
    def create_sensor(cls, board, options=None):
        """
        Factory method that creates a LSM6DS33 using the sensor API.

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


    def _who_am_i(self):
        """
        Returns the raw contents of the LSM6DS33 Who Am I egister.
        """
        response = self.transmit([self.REG_WHO_AM_I], 1)

        # ... and check to make sure that the device accepted our enable-value.
        result = response[0]

        # If we couldn't enable the ADC, throw an IOError.
        if result != self.WHO_AMI_I:
            raise errors.ExternalDeviceError("{} not found".format(self.name))

        return response[0]

    def set_accel_output_data_rate(self, rate):
        reg_ctrl1_xl = self.transmit([self.REG_CTRL1_XL],1)
        reg_ctrl1_xl = reg_ctrl1_xl[0]
        reg_ctrl1_xl &= 0x0F    # Clear top nibble
        reg_ctrl1_xl |= rate.value

        status = self.write([self.REG_CTRL1_XL, reg_ctrl1_xl])
        return status

    def set_gyro_output_data_rate(self, rate):
        reg_ctrl2_g = self.transmit([self.REG_CTRL2_G],1)
        reg_ctrl2_g = reg_ctrl2_g[0]
        reg_ctrl2_g &= 0x0F
        reg_ctrl2_g |=  rate.value

        status = self.write([self.REG_CTRL2_G, reg_ctrl2_g])
        return status

    def set_accel_full_scale(self, scale):
        reg_ctrl1_xl = self.transmit([self.REG_CTRL1_XL],1)
        reg_ctrl1_xl = reg_ctrl1_xl[0]
        reg_ctrl1_xl &= 0xF3    # Clear bits 2-3
        reg_ctrl1_xl |= scale.value

        status = self.write([self.REG_CTRL1_XL, reg_ctrl1_xl])
        return status

    def set_gyro_full_scale(self, scale):
        reg_ctrl2_g = self.transmit([self.REG_CTRL2_G],1)
        reg_ctrl2_g = reg_ctrl2_g[0]
        reg_ctrl2_g &= 0xF0     # Clear bottom nibble
        reg_ctrl2_g |= scale.value

        status = self.write([self.REG_CTRL2_G, reg_ctrl2_g])
        return status

    def set_accel_bandwidth(self, bw):
        reg_ctrl1_xl = self.transmit([self.REG_CTRL1_XL],1)
        reg_ctrl1_xl = reg_ctrl1_xl[0]
        reg_ctrl1_xl &= 0xFC    # Clear bits 0-1
        reg_ctrl1_xl |= bw.value

        status = self.write([self.REG_CTRL1_XL, reg_ctrl1_xl])
        return status

    def set_accel_perf_mode(self, mode):
        reg_ctrl6_c = self.transmit([self.REG_CTRL6_C],1)
        reg_ctrl6_c = reg_ctrl6_c[0]
        reg_ctrl6_c &= 0xE0    # Clear bits 0-4
        reg_ctrl6_c |= mode.value

        status = self.write([self.REG_CTRL6_C, reg_ctrl6_c])
        return status

    def set_gyro_perf_mode(self, mode):
        reg_ctrl7_g = self.transmit([self.REG_CTRL7_G],1)
        reg_ctrl7_g = reg_ctrl7_g[0]
        reg_ctrl7_g &= 0x7F    # Clear bit 7
        reg_ctrl7_g |= mode.value

        status = self.write([self.REG_CTRL7_G, reg_ctrl7_g])
        return status

    def _write_reg_bits(self, addr, val, mask):
        reg = self.transmit([addr],1)
        reg = reg[0]
        reg &= (~mask & 0xFF)   # Clear bits to be written
        reg |= val

        status = self.write([addr, reg])
        return status

class OutputDataRate(Enum):
    ODR_POWER_DOWN = 0x00
    ODR_12_5_HZ = 0x10
    ODR_26_HZ = 0x20
    ODR_52_HZ = 0x30
    ODR_104_HZ = 0x40
    ODR_208_HZ = 0x50
    ODR_416_HZ = 0x60
    ODR_833_HZ = 0x70
    ODR_1660_HZ = 0x80
    ODR_3330_HZ = 0x90
    ODR_6660_HZ = 0xA0

class AccelFullScale(Enum):
    FS_2_G = 0x00
    FS_4_G = 0x08
    FS_8_G = 0x0C
    FS_16_G = 0x04

class GyroFullScale(Enum):
    FS_125_DPS = 0x02
    FS_250_DPS = 0x00
    FS_500_DPS = 0x04
    FS_1000_DPS = 0x08
    FS_2000_DPS = 0x0C

class AccelBandwidthFilter(Enum):
    BW_400_HZ = 0x00
    BW_200_HZ = 0x01
    BW_100_HZ = 0x10
    BW_50_HZ = 0x11

class AccelPerfMode(Enum):
    HM_MODE_ENABLED = 0x00
    HM_MODE_DISABLED = 0x10

class GyroPerfMode(Enum):
    HM_MODE_ENABLED = 0x00
    HM_MODE_DISABLED = 0x80


# Set data rate and power mode for accel
# Set data rate and power mode for gyro

# Set gyro performance mode
# Set accelerometer performance mode

# Set FIFO mode
# Set FIFO watermark
# Set INT1/INT2 control