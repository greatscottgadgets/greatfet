#
# Copyright (c) 2016 Kyle J. Temkin <kyle@ktemkin.com>
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

from ..peripheral import GreatFETPeripheral


class I2CDevice(GreatFETPeripheral):
    """
        Class representing an generic I2C device connected to a GreatFET I2C Bus.

        This acts both as the base class for I2C devices, and as a generic class
        that can be used to access I2C devices for which no existing driver exists.
    """

    def __init__(self, bus, address, name='i2c device'):
        """
            Initialize a new generic I2C device.

            Args:
                bus -- An object representing the I2C bus on which this device
                    resides.
                address - The address for the given I2C device on the bus.
                name -- The display name for the given I2C device.
        """

        # Note: this will have to change if we decide to support 10-bit I2C addresses.
        if address > 127 or address < 0:
            raise ValueError("Tried to attach a device to an unsupported I2C address.")

        # Store our device parameters.
        self.bus = bus
        self.address = address
        self.name = name

        # Attach our device to the parent bus.
        self.bus.attach_device(self)


    def transmit(self, data, receive_length=0):
        """
            Sends data over the I2C bus, and optionally recieves
            data in response.

            Args:
                data -- The data to be sent to the given device.
                receive_length -- If provided, the I2C controller will attempt
                        to read the provided amount of data, in bytes.
        """
        return self.bus.transmit(self.address, data, receive_length)
