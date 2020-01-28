#
# Gladiolus source/sink for Software Defined IR
#

import array

import numpy as np
from gnuradio import gr

from greatfet import GreatFETSingleton
from .block import GreatFETStreamingSource

class GladiolusSource(GreatFETStreamingSource):

    def set_up_streaming(self, gain, dc_coupled):
        self.coupling = dc_coupled
        self.gf.sdir.set_gain(gain)
        self.gf.sdir.set_coupling(not dc_coupled)
        print(dc_coupled)
        return self.gf.sdir.start_receive()

    def tear_down_streaming(self):
        self.gf.sdir.stop()
