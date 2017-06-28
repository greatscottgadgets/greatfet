#
# Copyright (c) 2017 Kyle J. Temkin <kyle@ktemkin.com>
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

class SPIBus(GreatFETPeripheral):
    """
        Class representing a GreatFET SPI bus.

        For now, supports only the second SPI bus (SPI1), as the first controller
        is being used to control the onboard flash.
    """

    def __init__(self, board, name='spi bus', buffer_size=255):
        """
        Initialize a new SPI bus.

        Args:
            board -- The GreatFET board whose SPI bus we want to control.
            name -- The display name for the given SPI bus.
            buffer_size -- The size of the SPI receive buffer on the GreatFET.
        """

        # Store a reference to the parent board.
        self.board = board

        # Store our limitations.
        self.buffer_size = buffer_size

        # Create a list that will store all connected devices.
        self.devices = []

        # Set up the SPI bus for communications.
        board.vendor_request_out(vendor_requests.SPI_INIT)



    def attach_device(self, device):
        """
        Attaches a given SPI device to this bus. Typically called
        by the SPI device as it is constructed.

        Arguments:
            device -- The device object to attach to the given bus.
        """

        # TODO: Check for select pin conflicts; and handle chip select pins.

        self.devices.append(device)



    def transmit(self, data, receive_length=None):
        """
        Sends (and typically receives) data over the SPI bus.

        Args:
            data -- The data to be sent to the given device.
            receive_length -- Returns the total amount of data to be read. If longer
                than the data length, the transmit will automatically be extended
                with zeroes.

        TODO: Support more than one chip-select for more than one device on the bus!
        """

        if receive_length is None:
            receive_length = len(data)

        # If we need to receive more than we've transmitted, extend the data out.
        if receive_length > len(data):
            padding = receive_length - len(data)
            data.extend([0] * padding)

        if len(data) > self.buffer_size:
            raise ValueError("Tried to send/receive more than the size of the receive buffer.");

        # Perform the core transfer...
        self.board.vendor_request_out(vendor_requests.SPI_WRITE, data=data)

        # If reciept was requested, return the received data.
        if receive_length > 0:
            result = self.board.vendor_request_in(vendor_requests.SPI_READ,
                length=receive_length)
        else:
            result = []

        return result
