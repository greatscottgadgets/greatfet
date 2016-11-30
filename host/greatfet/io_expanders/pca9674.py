# coding: utf-8
#
# Copyright (c) 2016 Schuyler St. Leger <schuyler.st.leger@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

from ..io_expander import DIOExpander
from ..peripherals.i2c_device import I2CDevice

class PCA9674(I2CDevice, DIOExpander):
    """PCA9674 I²C I/O expander"""

    def __init__(self, bus, address=0x20):
        """Initialize a new PCA9674 I²C I/O expander.

        Argsuments:
            bus: I²C bus the IO PCA9674 is connected to. For a
                GreatFET One, this would typically be (board.i2c).
            address: Optional; used to specify a non-default address.
        """

        super(PCA9674, self).__init__(bus, address, 'PCA9674')

    @property
    def get_number_pins(self):
        return 8

    def set_direction(self, value):
        # Invert value
        pin_states = ~value & 2**self.get_number_pins - 1
        self.transmit(pin_states, 0)

    def read(self):
        return self.transmit([], 1)[0]

    def write(self, value):
        self.transmit([self._validate_pin_values(value)])
