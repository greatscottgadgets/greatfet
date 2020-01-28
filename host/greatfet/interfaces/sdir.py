
#
# This file is part of GreatFET
#

import array
import usb

from ..interface import GreatFETInterface
from greatfet.protocol import vendor_requests


class SDIRTransceiver(GreatFETInterface):
    """
        Data source for scanning out software-defined IR data.
    """

    USB_TIMEOUT_ERRNO = 110

    # Gain parameters in
    MAX_GAIN_SAMPLE  = 116
    MAX_GAIN_DB      = 50

    DEFAULT_GAIN     = 0
    DEFAULT_DAC_ADDR = 0x60

    def __init__(self, board, dac_i2c_addr=DEFAULT_DAC_ADDR, initial_gain=DEFAULT_GAIN):
        self.board        = board
        self.api          = board.apis.sdir
        self.dac_i2c_addr = dac_i2c_addr
        self.gain         = initial_gain

        self.coupling_pin = None
        self.running      = False


    def set_coupling(self, ac_coupled):
        """ Sets whether the SDIR sampling is AC or DC coupling. """

        if self.coupling_pin is None:
            self.coupling_pin = self.board.gpio.get_pin('J2_P27')

        if ac_coupled:
            self.coupling_pin.low()
        else:
            self.coupling_pin.high()


    def start_receive(self):
        self.set_gain(self.gain)
        pipe = self.api.start_receive()
        self.running = True
        return pipe


    def stop(self):
        self.api.stop()
        self.running = False


    def set_gain(self, db_gain):
        """ Sets the gain of the receiver, in dB. """
        self.gain = db_gain

        # Convert our gain in decibels to the relevant DAC samples, and
        # configure the DAC with it.
        sample = (db_gain * self.MAX_GAIN_SAMPLE) // self.MAX_GAIN_DB
        self.board.i2c.write(self.dac_i2c_addr, (0, sample))


    def read(self, timeout=1000, max_data=0x4000, autostart=False, allow_timeout=False):
        """ Reads all available samples from the GreatFET. """

        if not self.running and autostart:
            self.start_receive()

        try:
            return self.board.comms.device.read(0x81, max_data, timeout=timeout)
        except usb.core.USBError as e:
            if (e.errno == self.USB_TIMEOUT_ERRNO) and allow_timeout:
                return None
            else:
                raise

