#
# This file is part of GreatFET
#

from .. import errors
from ..sensor import GreatFETSensor
from ..interfaces.i2c_device import I2CDevice
from enum import IntEnum


class OutputDataRate(IntEnum):
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


class AccelFullScale(IntEnum):
    FS_2_G = 0x00
    FS_4_G = 0x08
    FS_8_G = 0x0C
    FS_16_G = 0x04


class GyroFullScale(IntEnum):
    FS_125_DPS = 0x02
    FS_250_DPS = 0x00
    FS_500_DPS = 0x04
    FS_1000_DPS = 0x08
    FS_2000_DPS = 0x0C


class AccelBandwidthFilter(IntEnum):
    BW_400_HZ = 0x00
    BW_200_HZ = 0x01
    BW_100_HZ = 0x10
    BW_50_HZ = 0x11


class AccelPerfMode(IntEnum):
    HM_MODE_ENABLED = 0x00
    HM_MODE_DISABLED = 0x10


class GyroPerfMode(IntEnum):
    HM_MODE_ENABLED = 0x00
    HM_MODE_DISABLED = 0x80


class FifoMode(IntEnum):
    FIFO_MODE_BYPASS = 0x00
    FIFO_MODE_FIFO = 0x01
    FIFO_MODE_CONTINUOUS = 0x06
    FIFO_MODE_CONTINUOUS_TO_FIFO = 0x03
    FIFO_MODE_BYPASS_TO_CONTINUOUS = 0x04


class FifoOutputDataRate(IntEnum):
    FIFO_ODR_DISABLED = 0x00
    FIFO_ODR_12_5_HZ = 0x08
    FIFO_ODR_26_HZ = 0x10
    FIFO_ODR_52_HZ = 0x18
    FIFO_ODR_104_HZ = 0x20
    FIFO_ODR_208_HZ = 0x28
    FIFO_ODR_416_HZ = 0x30
    FIFO_ODR_833_HZ = 0x38
    FIFO_ODR_1660_HZ = 0x40
    FIFO_ODR_3330_HZ = 0x48
    FIFO_ODR_6660_HZ = 0x50


class AccelDecRate(IntEnum):
    DEC_RATE_NONE = 0x00
    DEC_RATE_1 = 0x01
    DEC_RATE_2 = 0x02
    DEC_RATE_3 = 0x03
    DEC_RATE_4 = 0x04
    DEC_RATE_8 = 0x05
    DEC_RATE_16 = 0x06
    DEC_RATE_32 = 0x07


class GyroDecRate(IntEnum):
    DEC_RATE_NONE = 0x00
    DEC_RATE_1 = 0x08
    DEC_RATE_2 = 0x10
    DEC_RATE_3 = 0x18
    DEC_RATE_4 = 0x20
    DEC_RATE_8 = 0x28
    DEC_RATE_16 = 0x30
    DEC_RATE_32 = 0x38


class BlockDataUpdate(IntEnum):
    BDU_CONT = 0x00
    BDU_READ_LSB_MSB = 0x40


class DataEndian(IntEnum):
    BLE_LSB_LOWER_ADDR = 0x00
    BLE_MSB_LOWER_ADDR = 0x02


class PinOutputMode(IntEnum):
    MODE_PP = 0x00
    MODE_OD = 0x10


class Int1Ctrl(IntEnum):
    INT1_DRDY_XL = 0x01
    INT1_DRDY_G = 0x02
    INT1_BOOT = 0x04
    INT1_FTH = 0x08
    INT1_FIFO_OVR = 0x10
    INT1_FULL_FLAG = 0x20
    INT1_SIGN_MOT = 0x40
    INT1_STEP_DETECTOR = 0x80


class Int2Ctrl(IntEnum):
    INT2_DRDY_XL = 0x01
    INT2_DRDY_G = 0x02
    INT2_DRDY_TEMP = 0x04
    INT2_FTH = 0x08
    INT2_FIFO_OVR = 0x10
    INT2_FULL_FLAG = 0x20
    INT2_STEP_COUNT_OV = 0x40
    INT2_STEP_DELTA = 0x80


class AccelAxis(IntEnum):
    X = 0x08
    Y = 0x10
    Z = 0x20


class GyroAxis(IntEnum):
    X = 0x08
    Y = 0x10
    Z = 0x20


class FifoDataset(IntEnum):
    ACCEL = 1
    GYRO = 2
    PEDO = 3
    TEMP = 4


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
    REG_CTRL7_G = 0x16
    REG_CTRL8_XL = 0x17
    REG_CTRL9_XL = 0x18
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

    # Register masks
    MASK_ODR_XL = 0xF0
    MASK_ODR_G = 0xF0
    MASK_BW_XL = 0x03
    MASK_FS_XL = 0x0C
    MASK_FS_G = 0x0F
    MASK_XL_HM_MODE = 0x10
    MASK_G_HM_MODE = 0x80
    MASK_FIFO_MODE = 0x07
    MASK_DEC_FIFO_XL = 0x07
    MASK_DEC_FIFO_G = 0x38
    MASK_FTH_0_7 = 0xFF
    MASK_FTH_8_11 = 0x0F
    MASK_BDU = 0x40
    MASK_BLE = 0x02
    MASK_PP_OD = 0x10
    MASK_DIFF_FIFO_0_7 = 0xFF
    MASK_DIFF_FIFO_8_11 = 0x0F
    MASK_INT1_CTRL_MODE = 0xFF
    MASK_INT2_CTRL_MODE = 0xFF
    MASK_FIFO_EMPTY = 0x10
    MASK_FIFO_FULL = 0x20
    MASK_FIFO_OVER_RUN = 0x40
    MASK_FTH = 0x80
    MASK_XEN_XL = 0x08
    MASK_YEN_XL = 0x10
    MASK_ZEN_XL = 0x20
    MASK_XEN_G = 0x08
    MASK_YEN_G = 0x10
    MASK_ZEN_G = 0x20
    MASK_ODR_FIFO = 0x78

    # Accel/gyro output scaling constants/sensitivity
    XL_FS_2G_MG_PER_LSB = 0.061
    XL_FS_4G_MG_PER_LSB = 0.122
    XL_FS_8G_MG_PER_LSB = 0.244
    XL_FS_16G_MG_PER_LSB = 0.488

    G_FS_125DPS_MDPS_PER_LSB = 4.375
    G_FS_250DPS_MDPS_PER_LSB = 8.75
    G_FS_500DPS_MDPS_PER_LSB = 17.50
    G_FS_1000DPS_MDPS_PER_LSB = 35.0
    G_FS_2000PS_MDPS_PER_LSB = 70.0

    # FIFO scalar names
    FIFO_SCALARS_ACCEL = ['Accel_X', 'Accel_Y', 'Accel_Z']
    FIFO_SCALARS_GYRO = ['Gyro_X', 'Gyro_Y', 'Gyro_Z']
    FIFO_SCALARS_PED = ['Ped_0', 'Ped_1', 'Ped_2']
    FIFO_SCALARS_TEMP = ['Temp_0', 'Temp_1', 'Temp_2']

    FIFO_SCALARS_PER_DATASET = 3

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

        self._fifo_dataset_pattern = None
        self._fifo_mode = FifoMode.FIFO_MODE_BYPASS
        self._int1_read = None
        self._int2_read = None
        self._data_endian = DataEndian.BLE_LSB_LOWER_ADDR

    @classmethod
    def create_sensor(cls, board, options=None):
        """
        Factory method that creates a LSM6DS33 using the sensor API.

        This will eventually provide compatibility with e.g. datalogger
        applications.

        Args:
            board -- The GreatFET being configured using the sensor API.
            options -- A dictionary of any options to be provided to
            the device.
                To be used e.g. on application command lines.
        """

        if not options:
            options = {}

        address = options['address'] if (
            'address' in options) else cls.DEFAULT_ADDRESS

        default_suffix = "@{}".format(address)
        suffix = options['suffix'] if ('suffix' in options) else default_suffix

        # Create and return our raw sensor.
        return cls(board.i2c, address, suffix)

    def _who_am_i(self):
        """
        Returns the raw contents of the LSM6DS33 Who Am I egister.
        """
        response = self.transmit([self.REG_WHO_AM_I], 1)

        # ... and check to make sure that the device accepted our enable-value.
        result = response[0]

        # If we don't receive the correct response, throw an IOError.
        if result != self.WHO_AMI_I:
            raise errors.ExternalDeviceError("{} not found".format(self.name))

        return response[0]

    def set_accel_output_data_rate(self, rate):
        return self._write_reg_bits(self.REG_CTRL1_XL, rate, self.MASK_ODR_XL)

    def set_gyro_output_data_rate(self, rate):
        return self._write_reg_bits(self.REG_CTRL2_G, rate, self.MASK_ODR_G)

    def set_accel_full_scale(self, scale):
        return self._write_reg_bits(self.REG_CTRL1_XL, scale, self.MASK_FS_XL)

    def set_gyro_full_scale(self, scale):
        return self._write_reg_bits(self.REG_CTRL2_G, scale, self.MASK_FS_G)

    def set_accel_bandwidth(self, bw):
        return self._write_reg_bits(self.REG_CTRL1_XL, bw, self.MASK_BW_XL)

    def set_accel_perf_mode(self, mode):
        return self._write_reg_bits(self.REG_CTRL6_C,
                                    mode,
                                    self.MASK_XL_HM_MODE)

    def set_gyro_perf_mode(self, mode):
        return self._write_reg_bits(self.REG_CTRL7_C,
                                    mode,
                                    self.MASK_XL_HM_MODE)

    def set_fifo_mode(self, mode):
        return self._write_reg_bits(self.REG_FIFO_CTRL5,
                                    mode,
                                    self.MASK_FIFO_MODE)

    def set_fifo_output_data_rate(self, odr):
        return self._write_reg_bits(self.REG_FIFO_CTRL5,
                                    odr,
                                    self.MASK_ODR_FIFO)

    def set_accel_dec_rate(self, rate):
        return self._write_reg_bits(self.REG_FIFO_CTRL3, rate,
                                    self.MASK_DEC_FIFO_XL)

    def set_gyro_dec_rate(self, rate):
        return self._write_reg_bits(self.REG_FIFO_CTRL3,
                                    rate,
                                    self.MASK_DEC_FIFO_G)

    def set_fifo_threshold_level(self, level):
        write_status_ctrl1 = self._write_reg_bits(
            self.REG_FIFO_CTRL1, level & 0xFF, self.MASK_FTH_0_7)
        write_status_ctrl2 = self._write_reg_bits(
            self.REG_FIFO_CTRL2, level >> 8, self.MASK_FTH_8_11)
        return (write_status_ctrl1 | write_status_ctrl2)

    def set_block_data_update(self, update):
        return self._write_reg_bits(self.REG_FIFO_CTRL3, update, self.MASK_BDU)

    def set_data_endian(self, endian):
        self._data_endian = endian
        return self._write_reg_bits(self.REG_FIFO_CTRL3, endian, self.MASK_BLE)

    def set_pin_output_mode(self, mode):
        return self._write_reg_bits(self.REG_FIFO_CTRL3, mode, self.MASK_PP_OD)

    def set_int1_ctrl(self, ctrl):
        return self._write_reg_bits(self.REG_INT1_CTRL,
                                    ctrl,
                                    self.MASK_INT1_CTRL_MODE)

    def set_int2_ctrl(self, ctrl):
        return self._write_reg_bits(self.REG_INT2_CTRL,
                                    ctrl,
                                    self.MASK_INT2_CTRL_MODE)

    def set_accel_axis_output_enable(self, axis):
        return self._write_reg_bits(self.REG_CTRL9_XL,
                                    axis,
                                    axis)

    def set_gyro_axis_output_enable(self, axis):
        return self._write_reg_bits(self.REG_CTRL10_C,
                                    axis,
                                    axis)

    def get_fifo_num_unread_words(self):
        fifo_unread_words_lsb = (self.transmit(
            [self.REG_FIFO_STATUS1], 1))[0] & self.MASK_DIFF_FIFO_0_7
        fifo_unread_words_msb = (self.transmit(
            [self.REG_FIFO_STATUS2], 1))[0] & self.MASK_DIFF_FIFO_8_11
        return (fifo_unread_words_lsb | (fifo_unread_words_msb << 8))

    def get_fifo_pattern_index(self):
        fifo_pattern_lsb = (self.transmit([self.REG_FIFO_STATUS3], 1))[0]
        fifo_pattern_msb = (self.transmit([self.REG_FIFO_STATUS4], 1))[0]
        return (fifo_pattern_lsb | (fifo_pattern_msb << 8))

    def get_fifo_watermark_status(self):
        return (self.transmit([self.REG_FIFO_STATUS2], 1)[0] & self.MASK_FTH)

    def get_fifo_full_status(self):
        return (self.transmit([self.REG_FIFO_STATUS2], 1)[0] &
                self.MASK_FIFO_FULL)

    def get_fifo_empty_status(self):
        return (self.transmit([self.REG_FIFO_STATUS2], 1)[0] &
                self.MASK_FIFO_EMPTY)

    def get_fifo_overrun_status(self):
        return (self.transmit([self.REG_FIFO_STATUS2], 1)[0] &
                self.MASK_FIFO_OVER_RUN)

    def get_fifo_word(self):
        return (self.transmit([self.REG_FIFO_DATA_OUT_L], 2))

    def get_data_endian(self):
        self._data_endian = ((self.transmit([self.REG_FIFO_CTRL3], 1))[0] |
                             self.MASK_BLE)
        return self._data_endian

    def get_fifo_data(self):
        data = {}

        for p in self._fifo_scalar_pattern:
            data[p] = []

        # Get number of words to be read
        num_fifo_words = self.get_fifo_num_unread_words()

        # In continuous mode, one data set must remain
        # in the FIFO to prevent data misalignment
        # and FIFO mode being reset to bypass by hardware
        # See sec. 7.2.2 of the ST app note AN4682
        if (self._fifo_mode == FifoMode.FIFO_MODE_CONTINUOUS):
            num_fifo_words -= (self.FIFO_SCALARS_PER_DATASET *
                               self._get_num_fifo_pattern_datasets())

        base_pattern_idx = self.get_fifo_pattern_index()

        for i in range(num_fifo_words):
            pattern_idx = ((base_pattern_idx + i) %
                           (self._get_num_fifo_pattern_scalars()))
            scalar = self._get_fifo_pattern_scalar(pattern_idx)
            data_word_bytes = self.get_fifo_word()
            data_word = self._word_bytes_to_int(data_word_bytes)
            data[scalar].append(data_word)

        return data

    def set_fifo_dataset_pattern(self, pattern):
        self._fifo_dataset_pattern = pattern
        self._fifo_scalar_pattern = []
        for p in pattern:
            if (p == FifoDataset.ACCEL):
                self._fifo_scalar_pattern.extend(self.FIFO_SCALARS_ACCEL)
            elif (p == FifoDataset.GYRO):
                self._fifo_scalar_pattern.extend(self.FIFO_SCALARS_GYRO)
            elif (p == FifoDataset.PEDO):
                self._fifo_scalar_pattern.extend(self.FIFO_SCALARS_PED)
            elif (p == FifoDataset.TEMP):
                self._fifo_scalar_pattern.extend(self.FIFO_SCALARS_TEMP)
            else:
                pass
        return

    def set_int1_read(self, handler):
        self._int1_read = handler
        return

    def set_int2_read(self, handler):
        self._int2_read = handler
        return

    def int1_read(self):
        return self._int1_read()

    def int2_read(self):
        return self._int2_read()

    def _write_reg_bits(self, addr, val, mask):
        reg = self.transmit([addr], 1)  # Get current register value
        reg = reg[0]
        reg &= (~mask & 0xFF)   # Clear bits to be written
        reg |= val
        self.write([addr, reg])
        return 0

    def _write_reg(self, addr, val):
        self.write([addr, val])

    def _raw_to_eng(self, raw, scale):
        return raw * scale

    def _word_to_raw(self, word):
        return int(word[0] + (word[1] << 8))

    def _word_to_eng(self, word, scale):
        raw = self._word_to_raw(word)
        eng = self._raw_to_eng(raw, scale)
        return eng

    def _get_num_fifo_pattern_datasets(self):
        return len(self._fifo_dataset_pattern)

    def _get_num_fifo_pattern_scalars(self):
        return len(self._fifo_scalar_pattern)

    def _get_fifo_pattern_scalar(self, idx):
        return self._fifo_scalar_pattern[idx]

    def _word_bytes_to_int(self, word_bytes):
        if (self._data_endian == DataEndian.BLE_LSB_LOWER_ADDR):
            return (int.from_bytes(word_bytes,
                                   byteorder='little',
                                   signed=True))
        else:
            return (int.from_bytes(word_bytes,
                                   byteorder='big',
                                   signed=True))
