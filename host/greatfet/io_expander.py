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

from abc import ABCMeta, abstractmethod, abstractproperty

class GreatFEFDIOExpander(object):
    """Class representing a generic digital IO expander.
    Values for the pin states are integers with the least significant bit
    corresponding to pin 0 on the expander.
    """
    __metaclass__ = ABCMeta

    @abstractproperty
    def get_number_pins(self):
        """Return the number of expanded pins as an integer."""
        return 0

    @abstractmethod
    def set_direction(self, directions):
        """Set the direction of the pins.
        0 represents an input
        1 represents an output
        """
        return

    @abstractmethod
    def read(self):
        """Return the value of the pins."""
        return

    @abstractmethod
    def write(self, value):
        """Set the value of the pins."""
        return

    def _validate_pin_values(self, value):
        """Ensure the value is in a range that can represent the states of the pins."""
        if value < 0 or value > self.get_number_pins ** 2 - 1:
            raise ValueError("Tried to set an invalid value.")
        return value
