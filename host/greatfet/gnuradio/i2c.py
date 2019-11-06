#
# Blocks for I2C for "software defined everything."
#

import ast
import array

import numpy as np
from gnuradio import gr

from greatfet import GreatFET

class i2c_source(gr.sync_block):

    def __init__(self, sample_rate, address, data_to_write, read_length):

        gr.sync_block.__init__(
            self,
            name='GreatFET I2C Sink',   # will show up in GRC
            in_sig=None,
            out_sig=[np.uint8]
        )

        # Copy in our input arguments.
        self.sample_rate = sample_rate
        self.extra_samples = array.array('B')

        # Create a local GreatFET object to work with.
        self.gf = GreatFET()
        self.gf.comms.get_exclusive_access()

        # Prepare our stream by notifying the GreatFET of our arguments.
        data_to_write = bytes(ast.literal_eval(data_to_write))
        self.endpoint = self.gf.apis.i2c.stream_periodic_read(sample_rate, address, read_length, data_to_write)

        self.buffer   = array.array('B', bytes(4096))


    def work(self, input_items, output_items):

        out = output_items[0]

        # FIXME: abstract
        num_samples = self.gf.comms.device.read(self.endpoint, self.buffer, 100)
        samples = self.buffer[0:num_samples]

        # If we have samples left over from last time, use them.
        if self.extra_samples:
            samples = samples + self.extra_samples

        # If we don't have any samples, return an empty length.
        if not samples:
            return 0

        # If we don't have enough buffer to grab the relevant samples, save any extra we have for next time.
        if len(samples) > len(out):
            self.extra_samples = samples[len(out):]
            samples = samples[:len(out)]

        # Copy our sample array to GNURadio.
        out[:len(samples)] = samples
        return len(samples)

