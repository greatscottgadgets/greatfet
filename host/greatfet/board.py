#
# Copyright 2016 Kyle J. Temkin <kyle@ktemkin.com>
#
# This file is part of GreatFET.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)gt
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#
"""
Module containing the core definitions for a GreatFET board.
"""

import usb

from .protocol import vendor_requests
from .errors import DeviceNotFoundError

# Default device identifiers.
GREATFET_VENDOR_ID = 0x1d50
GREATFET_PRODUCT_ID = 0x60e6


class GreatFETBoard(object):
    """
    Class representing a USB-connected GreatFET device.
    """

    """
    The GreatFET board IDs handled by this class. Used by the default
    implementation of accepts_connected_device() to determine if a given subclass
    handles the given board ID.
    """
    HANDLED_BOARD_IDS = []

    """
    The display name of the given GreatFET board. Subclasses should override
    this with a more appropriate name.
    """
    BOARD_NAME = "Unknown GreatFET"


    @classmethod
    def autodetect(cls, **device_identifiers):
        """
        Attempts to create a new instance of the GreatFETBoard subclass
        most applicable to the given device. For example, if the attached
        board is a GreatFET Azalea, this will automatically create a
        GreatFET Azalea object.

        Accepts the same arguments as pyusb's usb.find() method, allowing narrowing
        to a more specific GreatFET by e.g. serial number.

        Throws a DeviceNotFoundError if no device is avaiable.
        """

        # Iterate over each subclass of GreatFETBoard until we find a board
        # that accepts the given board ID.
        for subclass in cls.__subclasses__():
            if subclass.accepts_connected_device(**device_identifiers):
                return subclass(**device_identifiers)

        # If we couldn't find a board, raise an error.
        raise DeviceNotFoundError()


    @classmethod
    def accepts_connected_device(cls, **device_identifiers):
        """
        Returns true iff the provided class is appropriate for handling a connected
        GreatFET.

        Accepts the same arguments as pyusb's usb.find() method, allowing narrowing
        to a more specific GreatFET by e.g. serial number.
        """
        try:
            potential_device = cls(**device_identifiers)

            # Accept only GreatFET devices whose board IDs are handled by this
            # class. This is mostly used by subclasses, which should override
            # HANDLED_BOARD_IDS.
            return potential_device.board_id() in cls.HANDLED_BOARD_IDS
        except DeviceNotFoundError:
            return False
        finally:
            pass # TODO: close the board, here; or otherwise release resources?


    def __init__(self, **device_identifiers):
        """
        Instantiates a new connection to a GreatFET device; by default connects
        to the first available GreatFET.

        Accepts the same arguments as pyusb's usb.find() method, allowing narrowing
        to a more specific GreatFET by serial number.
        """

        # By default, accept any device with the default vendor/product IDs.
        identifiers = {
            'idVendor': GREATFET_VENDOR_ID,
            'idProduct': GREATFET_PRODUCT_ID,
        }

        # Connect to the first available GreatFET device.
        identifiers.update(device_identifiers)
        self.device = usb.core.find(**identifiers)

        # If we couldn't find a GreatFET, bail out early.
        if self.device is None:
            raise DeviceNotFoundError()

        # Ensure that we have an active USB connection to the device.
        self._initialize_usb()


    def _initialize_usb(self):
        """Sets up our USB connection to the GreatFET device."""

        # For now, the GreatFET is only providing a single configuration, so we
        # can accept the first configuration provided.
        self.device.set_configuration()


    def board_id(self):
        """Reads the board ID number for the GreatFET device."""

        # Query the board for its ID number.
        response = self._vendor_request_in(vendor_requests.READ_BOARD_ID)
        return response[0]


    def board_name(self):
        """Returns the human-readable product-name for the GreatFET device."""
        return self.BOARD_NAME


    def firmware_version(self):
        """Reads the board's firmware version."""

        # Query the board for its firmware version, and convert that to a string.
        return self._vendor_request_in_string(vendor_requests.READ_VERSION_STRING, length=255)


    def serial_number(self, as_hex_string=True):
        """Reads the board's unique serial number."""
        result = self._vendor_request_in(vendor_requests.READ_PARTID_SERIALNO, length=255)

        # If we've been asked to convert this to a hex string, do so.
        if as_hex_string:
            hex_generator = ('{:02x}'.format(x) for x in result)
            result = ''.join(hex_generator)

        return result



    def _vendor_request(self, direction, request, length=0, value=0):
        """Performs a USB vendor-specific control request.

        See also _vendor_request_in()/_vendor_request_out(), which provide a
        simpler syntax for simple requests.

        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            value -- The value to be passed to the vendor request.
            length -- The length of the data expected in response from the request.
        """
        return self.device.ctrl_transfer(
            direction | usb.TYPE_VENDOR | usb.RECIP_DEVICE,
            request, value, 0, length)


    def _vendor_request_in(self, request, length=1):
        """Performs a USB control request that expects a respnose from the GreatFET.

        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            length -- The length of the data expected in response from the request.
        """
        return self._vendor_request(usb.ENDPOINT_IN, request, length)


    def _vendor_request_in_string(self, request, length=255):
        """Performs a USB control request that expects a respnose from the GreatFET.

        Interprets the result as a UTF-8 encoded string.

        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            length -- The length of the data expected in response from the request.
        """
        raw = self._vendor_request(usb.ENDPOINT_IN, request, length)
        return raw.tostring().decode('utf-8')


    def _vendor_request_out(self, request, value=0):
        """Performs a USB control request that provides data to the GreatFET.

        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            value -- The value to be passed to the vendor request.
        """
        return self._vendor_request(usb.ENDPOINT_OUT, request, value=value)



