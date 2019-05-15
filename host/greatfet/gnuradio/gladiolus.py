#
# Gladiolus source/sink for Software Defined IR
#

import array

import numpy as np
from gnuradio import gr

from greatfet import GreatFETSingleton

class gladiolus_source(gr.sync_block):

    def __init__(self, sample_rate=10.2e6):

        gr.sync_block.__init__(
            self,
            name='GreatFET Gladiolus Sink',   # will show up in GRC
            in_sig=None,
            out_sig=[np.uint8]
        )

        # if an attribute with the same name as a parameter is found,
        # a callback is registered (properties work, too).
        self.sample_rate = sample_rate
        self.extra_samples = array.array('B')


    def work(self, input_items, output_items):

        out = output_items[0]

        greatfet = GreatFETSingleton()
        samples = greatfet.sdir.read(timeout=100, autostart=True)

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
