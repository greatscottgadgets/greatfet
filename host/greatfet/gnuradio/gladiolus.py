#
# Gladiolus source/sink for Software Defined IR
#

import array

import numpy as np
from gnuradio import gr

from greatfet import GreatFETSingleton
from .block import GreatFETStreamingSource

class GladiolusSource(GreatFETStreamingSource):

    def set_up_streaming(self):
        return self.gf.apis.sdir.start_receive()

    def tear_down_streaming(self):
        self.gf.apis.sdir.stop()
