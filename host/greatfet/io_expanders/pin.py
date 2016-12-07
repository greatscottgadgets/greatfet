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

from __future__ import print_function
from ..io_expander import DIOExpander

class PinExpander(DIOExpander):
    """Example class representing a pin based IO expander"""

    def __init__(self, number_pins=4):
        super(PinExpander, self).__init__()

        self.__number_pins = int(number_pins)
        print("Initialized PinExpander")

    @property
    def number_pins(self):
        return self.__number_pins


    def set_direction(self, directions):
        super(PinExpander, self).set_direction(directions)

    def set_pin_direction(self, pin_num, direction):
        pin_num, value = self._validate_pin_args(pin_num, value)

        # Once the direction has been successfully changed on the IO expander,
        # `_directions` has to be updated.
        self._directions = self._set_bit(self._directions, pin_num, direction)

        print("Directing PinExpander:  pin: {} direction: {}".format(pin_num, direction))


    def read(self):
        return super(PinExpander, self).read()

    def read_pin(self, pin_num):
        pin_num = self._validate_pin_number(pin_num)

        pin_value = self._git_bit(self._pins, pin_num)
        print("Reading PinExpander:  pin: {} value: {}".format(pin_num, pin_value))

        return pin_value


    def write(self, value):
        super(PinExpander, self).write(value)

    def write_pin(self, pin_num, value):
        pin_num, value = self._validate_pin_args(pin_num, value)

        # Once the value has been successfully sent to the IO expander, `_pins`
        # has to be updated.
        self._pins = self._set_bit(self._pins, pin_num, value)

        print("Writing PinExpander:  pin: {} value: {}".format(pin_num, value))
