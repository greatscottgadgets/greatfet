#
# Blocks for I2C for "software defined everything."
#

import ast
import array

import numpy as np
from gnuradio import gr

from greatfet import GreatFET

from .block import GreatFETStreamingSource

class ADCSourceBlock(GreatFETStreamingSource):
    """ Block that reads from the GreatFET's ADC. """

    SAMPLE_SIZE_BYTES = 2
    OUTPUT_MAX_SCALE=(1024 / 3.3)

    def set_up_streaming(self, address, prelude_script='', prelude=''):
        self.handle_preludes(prelude, prelude_script)
        return self.gf.apis.adc.stream_periodic_read(round(self.sample_rate))

    def tear_down_streaming(self):
        self.gf.apis.adc.stop_periodic_read()

