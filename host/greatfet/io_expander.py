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

class DIOExpander(object):
    """Class representing a generic digital IO expander.
    Values for the pin states are integers with the least significant bit
    corresponding to pin 0 on the expander.
    """
    __metaclass__ = ABCMeta

    @abstractmethod
    def __init__(self):
        self._pins = 0
        self._directions = 0

    @abstractproperty
    def number_pins(self):
        """Return the number of expanded pins as an integer."""
        return 0


    @abstractmethod
    def set_direction(self, directions):
        """Set the direction of the pins.
        0 represents an input
        1 represents an output
        """
        directions = self._validate_port_value(directions)
        write_directions = directions

        for i in range(self.number_pins):
            self.set_pin_direction(i, directions & 1)
            directions = (directions >> 1)

        self._directions = write_directions

    @abstractmethod
    def set_pin_direction(self, pin_num, direction):
        pin_num, direction = self._validate_pin_args(pin_num, direction)

        directions = self._set_bit(self._directions, pin_num, direction)

        self.set_direction(directions)
        self._directions = directions


    @abstractmethod
    def read(self):
        """Return the value of the pins."""
        pin_values = 0
        for i in range(self.number_pins - 1, -1, -1):
            pin_values = (pin_values << 1) | self.read_pin(i)

        return pin_values

    @abstractmethod
    def read_pin(self, pin_num):
        pin_num = self._validate_pin_number(pin_num)

        return self._git_bit(self.read(), pin_num)


    @abstractmethod
    def write(self, value):
        """Set the value of the pins."""
        value = self._validate_port_value(value)
        write_value = value

        for i in range(self.number_pins):
            self.write_pin(i, value & 1)
            value = (value >> 1)

        self._pins = write_value

    @abstractmethod
    def write_pin(self, pin_num, value):
        pin_num, value = self._validate_pin_args(pin_num, value)
        write_value = self._set_bit(self._pins, pin_num, value)

        self.write(write_value)
        self._pins = write_value


    def _validate_pin_number(self, pin_num):
        if pin_num < 0 or pin_num >= self.number_pins:
            raise ValueError("Tried to access an invalid pin.")
        return pin_num

    @staticmethod
    def _validate_pin_value(value):
        value = int(value)
        if value < 0 or value > 1:
            raise ValueError("Tried to set an invalid value for a pin.")
        return value

    def _validate_port_value(self, value):
        """Ensure the value is in a range that can represent the state of the pins."""
        if value < 0 or value > 2 ** self.number_pins - 1:
            raise ValueError("Tried to set an invalid value.")
        return value

    def _validate_pin_args(self, pin_num, value):
        return (self._validate_pin_number(pin_num), self._validate_pin_value(value))


    @staticmethod
    def _set_bit(bits, bit_num, bit_value):
        if bit_value:
            return bits | (1 << bit_num)
        else:
            return bits & ~(1 << bit_num)

    @staticmethod
    def _git_bit(bits, bit_num):
        return (bits >> bit_num) & 1
