#
# This file is part of GreatFET
#

import time

from pygreat.comms import CommandFailureError

from ..neighbor import GreatFETNeighbor
from ..interfaces.i2c.pca6408a import PCA6048A
from ..interfaces.spi.dac084s085 import DAC084S085
from ..programmers.ecp5 import ECP5SlaveSPI, ECP5MasterSerialDirect


class Foxglove(GreatFETNeighbor):
    """ Class representing a Foxglove FPGA/level-shifting neighbor. """

    # The address of our I2C I/O expander.
    IO_EXPANDER_I2C_ADDRESS = 0x21
    IO_EXPANDER_PIN_NAMES = {
        'P0': 'AUX_VREG_EN',
        'P1':  None,
        'P2': 'DIV_EN',
        'P3': 'DAC_CS',
        'P4': 'COMPARATOR1_RESULT',
        'P5': 'COMPARATOR2_RESULT',
        'P6': 'VCCB_VREG_EN',
        'P7': 'VCCA_VREG_EN',
    }

    # I/O pins that select the FPGA's configuration mode.
    CFG_PINS = ('J1_P27', 'J1_P12', 'J1_P13')

    # Pin that triggers a (re)configuration. Places the FPGA into a state where it will accept programming.
    PROGRAM_PIN = 'J1_P25'

    # The chip select on the Foxglove board's flash chip.
    FLASH_CHIP_SELECT_PIN = 'J1_P24'

    # Store the number of discrete values the DAC can represent. We have an 8-bit DAC.
    DAC_POSSIBLE_SAMPLES = 256

    # Mapping that converts supply/reference rail names to their equivalent DAC voltage.
    VREG_DAC_CHANNELS = {
        'VCCA': 'A',
        'VCCB': 'C',
        'AUX':  'B'
    }

    # Mapping that maps supply/reference rail names to their minimum and maximum voltages.
    VREG_VOLTAGE_RANGES = {
        'VCCA': (1.20, 5.0),
        'VCCB': (1.20, 3.3),
        'AUX':  (1.20, 5.0),
    }


    def __init__(self, board, verbose_function=None, set_up_hardware=True):
        """ Set up a new Foxglove neighbor."""

        self.board = board

        # Attach an object represneting the flash chip to program.
        flash_cs_gpio = board.gpio.get_pin(self.FLASH_CHIP_SELECT_PIN)

        # Create an object that represents the on-board I/O expander.
        self._io_expander = PCA6048A(board.i2c, device_address=self.IO_EXPANDER_I2C_ADDRESS,
                name_mappings=self.IO_EXPANDER_PIN_NAMES)

        # Create our GPIO objects.
        self._cfg_port = board.gpio.get_port(*self.CFG_PINS)
        self._program_pin = board.gpio.get_pin(self.PROGRAM_PIN)
        self._vreg_enables = {
            'VCCA': self._io_expander.get_pin('VCCA_VREG_EN'),
            'VCCB': self._io_expander.get_pin('VCCB_VREG_EN'),
            'AUX':  self._io_expander.get_pin('AUX_VREG_EN')
        }

        # Grab a copy of the board's main SPI bus.
        self._spi_bus = board.spi

        # Create an object representing our SPI flash.
        try:
            self.flash_programmer = ECP5MasterSerialDirect(board, chip_select=flash_cs_gpio,
                    cfg_pins=self._cfg_port, program_pin=self._program_pin, verbose_function=verbose_function)
        except CommandFailureError:
            self.flash_programmer = None

        # Create an object used for directly programming our FPGA.
        self.spi_programmer = ECP5SlaveSPI(board, cfg_pins=self._cfg_port,
                program_pin=self._program_pin, verbose_function=verbose_function)

        # Create an object that represents our DAC.
        self._dac = DAC084S085(self._spi_bus, chip_select=self._io_expander.get_pin('DAC_CS'), reference_voltage=5)

        # If the user has requested we set up the hardware, do so.
        if set_up_hardware:
            self.set_up_hardware()


    def set_up_hardware(self):
        """ Initializes the hardware; bringing the Foxglove board into a post-reset state. """

        # Start with our DAC outputs powered down, and our voltage regulators off.
        self._dac.power_down_outputs()
        self.use_external_vcca()
        self.use_external_vccb()



    def program_flash(self, bitstream, configure_after=False):
        """ Programs the given bitstream data to the target flash chip. The program will be retained across resets. """

        if self.flash_programmer is None:
            raise IOError("This Foxglove board does not appear to have an SPI flash populated.")

        # Program the target flash with the relevant bitstream.
        self.flash_programmer.program(bitstream)

        # If the caller wants us to reconfigure the FPGA as well, trigger reconfiguration.
        if configure_after:
            self._spi_bus.disable_drive()
            self.flash_programmer.trigger_reconfiguration()
            time.sleep(1)
            self._spi_bus.enable_drive()



    def configure_fpga(self, bitstream):
        """ Configures the onboard FPGA with the given program. Program will not be retained on reset. """
        self.spi_programmer.configure(bitstream)


    def _dac_value_for_rail_voltage(self, rail, voltage):
        """ Identifies the DAC input value that will produce the target voltage. """

        # Look up our maximum and minimum voltages, and compute the voltage step per DAC bit.
        v_min, v_max = self.VREG_VOLTAGE_RANGES[rail]
        volts_per_quanta = (v_min-v_max) / self.DAC_POSSIBLE_SAMPLES

        # Sanity check our input voltage.
        if voltage < v_min:
            raise ValueError("Rail {} cannot be set to voltages lower than {}.".format(rail, v_min))
        if voltage > v_max:
            raise ValueError("Rail {} cannot be set to voltages higher than {}.".format(rail, v_max))

        # Providing a sample value of N yields a voltage out such that v_out = (volts_per_quanta * N) + v_max.
        # With some algebra, we get a required sample value of:
        target_sample = round((voltage - v_max) / volts_per_quanta)
        target_sample = (target_sample - 1) if (target_sample == self.DAC_POSSIBLE_SAMPLES) else target_sample
        return target_sample


    def _set_rail_voltage(self, rail, voltage):
        """ Sets the voltage of the provided rail.

        Parameters:
            rail       -- The rail name (A, B, or DAC).
            voltage    -- The voltage to set; or None to disable the relevant rail.
        """

        # If this isn't a valid rail, fail out.
        if rail not in self.VREG_DAC_CHANNELS:
            raise ValueError("{} is not a valid rail name".format(voltage))

        dac_channel = self.VREG_DAC_CHANNELS[rail]
        regulator_enable  = self._vreg_enables[rail]

        # If our voltage is None, disable the rail.
        if voltage is None:
            regulator_enable.low()

        # Otherwise, set the DAC to the value that produces the target voltage.
        else:

            # Compute the DAC value we'll need to apply, and then apply it.
            target_sample_value = self._dac_value_for_rail_voltage(rail, voltage)
            self._dac.set_channel_value(dac_channel, target_sample_value)

            # And ensure that the regulator is on.
            regulator_enable.high()



    def apply_configuration(self, configuration):
        """ Applies a FoxgloveConfiguration object to the attached Foxglove.

        This method generates a set of gateware, and then uploads that gatware
        to the attached Foxglove board. This resets the FPGA, and reconfigures it
        to provide the connections and interfaces described in the target configuration.
        """

        try:
            import nmigen
        except ImportError:
            raise EnvironmentError("Cannot import nMigen, so we can't dynamically generate Gateware.")

        # FIXME: read VCC properties and etc. from the relevant bitstream

        # Convert our configuration to a bitstream, and then configure the FPGA with it.
        bitstream = configuration.to_bitstream()
        self.configure_fpga(bitstream)


    def provide_vcca(self, voltage):
        """ Set VCCA to the provided voltage. """
        self._set_rail_voltage('VCCA', voltage)


    def use_external_vcca(self):
        """ Disable the VCCA regulator. """
        self._set_rail_voltage('VCCA', None)


    def provide_vccb(self, voltage):
        """ Set VCCB to the provided voltage. """
        self._set_rail_voltage('VCCB', voltage)


    def use_external_vccb(self):
        """ Disable the VCCA regulator. """
        self._set_rail_voltage('VCCB', None)


    def set_aux_rail_voltage(self, voltage):
        """ Set VCCA to the provided voltage. """
        self._set_rail_voltage('AUX', voltage)


    def disable_aux_rail(self):
        """ Disable the VCCA regulator. """
        self._set_rail_voltage('AUX', None)


    def provide_clock(self, clk_pin=2):
        """ Provide a clock to the target FPGA. """

        # FIXME: abstract this into a clock generator interface
        CLOCK_SOURCE_60MHZ = 0x0D
        self.board.apis.clock_gen.output_clock(clk_pin, CLOCK_SOURCE_60MHZ)
