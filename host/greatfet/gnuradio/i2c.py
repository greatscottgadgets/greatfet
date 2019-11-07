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


    def set_up_streaming(self, address, data_to_write, read_length, normalize_by, prelude_script=''):
        self.sample_size_bytes = read_length
        self.normalization_max = normalize_by

        # If we were provided with a 'prelude script', run it before we execute our main block.
        if prelude_script:
            with open(prelude_script) as f:
                exec(f.read(), {'gf': self.gf })


        # Convert our 'data to write' argument to a set of bytes to be written.
        data_to_write = bytes(ast.literal_eval(data_to_write))
        return self.gf.apis.i2c.stream_periodic_read(self.sample_rate, address, read_length, data_to_write)


    def tear_down_streaming(self):
        self.gf.apis.i2c.stop_periodic_read()


    def get_sample_size(self):
        return self.sample_size_bytes


    def get_sample_max_scale(self):
        return self.normalization_max
