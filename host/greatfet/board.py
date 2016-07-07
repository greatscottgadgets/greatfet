#
# Copyright 2016 Kyle J. Temkin <kyle@ktemkin.com>
#
# This file is part of GreatFET.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
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

import usb

GREATFET_VENDOR_ID = 0x1d50
GREATFET_PRODUCT_ID = 0x60e6

# XXX: Move me!
usb_vendor_request_erase_spiflash = 0
usb_vendor_request_write_spiflash = 1
usb_vendor_request_read_spiflash = 2
usb_vendor_request_read_board_id = 3
usb_vendor_request_read_version_string = 4
usb_vendor_request_read_partid_serialno = 5
usb_vendor_request_enable_usb1 = 6
usb_vendor_request_led_toggle = 7

#TODO: Move this.
class DeviceNotFoundError(IOError):
    pass

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

        # TODO: Initialize package drivers.


    def _initialize_usb(self):
        """
        Sets up our USB connection to the GreatFET device.
        """

        # For now, the GreatFET is only providing a single configuration, so we
        # can accept the first configuration provided.
        self.device.set_configuration()


    def board_id(self):
        """
        Reads the board ID number for the GreatFET device.
        """

        # Query the board for its ID number.
        response = self._vendor_request_in(usb_vendor_request_read_board_id)
        return response[0]


    def board_name(self):
        """
        Returns the human-readable product-name for the GreatFET device.
        """
        return self.BOARD_NAME


    def _vendor_request(self, request, direction, length=0, val=0):
        """
        """
        pass

    def _vendor_request_in(self, request, length=1):
        """
        """
        return self.device.ctrl_transfer(
            usb.ENDPOINT_IN | usb.TYPE_VENDOR | usb.RECIP_DEVICE,
            request, 0, 0, length)

    def _vendor_request_out(self, request, val=0):
        return self.device.ctrl_transfer(
            usb.ENDPOINT_OUT | usb.TYPE_VENDOR | usb.RECIP_DEVICE,
            request, val, 0)



