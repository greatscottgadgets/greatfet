#
# Blocks for I2C for "software defined everything."
#

import ast
import array

import numpy as np
from gnuradio import gr

from greatfet import GreatFET

from .block import GreatFETStreamingSource

class I2CSourceBlock(GreatFETStreamingSource):
    """ Block that reads from an I2C device as a data source. """


    def set_up_streaming(self, address, data_to_write, read_length):
        # TODO: accept a prelude file to set things up?

        # Convert our 'data to write' argument to a set of bytes to be written.
        data_to_write = bytes(ast.literal_eval(data_to_write))
        return self.gf.apis.i2c.stream_periodic_read(self.sample_rate, address, read_length, data_to_write)


    def tear_down_streaming(self):
        self.gf.apis.i2c.stop_periodic_read()