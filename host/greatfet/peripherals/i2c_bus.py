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

from ..protocol import vendor_requests
from ..peripheral import GreatFETPeripheral

class I2CBus(GreatFETPeripheral):
    """
        Class representing a GreatFET I2C bus.

        For now, supports only the primary I2C bus (I2C0), but will be
        expanded when the vendor commands are.
    """

    def __init__(self, board, name='i2c bus', buffer_size=255):
        """
            Initialies a new I2C bus.

            Args:
                board -- The GreatFET board whose I2C bus we want to control.
                name -- The display name for the given I2C bus.
                buffer_size -- The size of the I2C receive buffer on the GreatFET.
        """

        # Store a reference to the parent board.
        self.board = board

        # Store our limitations.
        self.buffer_size = buffer_size

        # Create a list that will store all connected devices.
        self.devices = []

        # Set up the I2C bus for communications.
        board.vendor_request_out(vendor_requests.I2C_START)



    def attach_device(self, device):
        """
            Attaches a given I2C device to this bus. Typically called
            by the I2C device as it is constructed.

            Arguments:
                device -- The device object to attach to the given bus.
        """

        # TODO: Check for address conflicts!

        self.devices.append(device)



    def transmit(self, address, data, receive_length=0):
        """
            Sends data over the I2C bus, and optionally recieves
            data in response.

            Args:
                address -- The I2C address for the target device.
                    Should not contain read/write bits. Can be used to address
                    special addresses, for now; but this behavior may change.
                data -- The data to be sent to the given device.
                receive_length -- If provided, the I2C controller will attempt
                        to read the provided amount of data, in bytes.
        """

        if (not isinstance(receive_length, int)) or receive_length < 0:
            raise ValueError("invalid receive length!")

        if receive_length > self.buffer_size:
            raise ValueError("Tried to receive more than the size of the receive buffer.");

        if address > 127 or address < 0:
            raise ValueError("Tried to transmit to an invalid I2C address!")

        # Perform the core transfer...
        self.board.vendor_request_out(vendor_requests.I2C_XFER, value=address,
                index=receive_length, data=data)

        # If reciept was requested, return the received data.
        if receive_length:
            data = self.board.vendor_request_in(vendor_requests.I2C_RESPONSE,
                length=receive_length)
        else:
            data = []

        return data
