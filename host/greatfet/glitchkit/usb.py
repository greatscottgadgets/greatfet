#
# This file is part of GreatFET
#

from __future__ import absolute_import

import usb
import codecs

from .base import GlitchKitModule
from ..protocol import vendor_requests


# Quirk constant that helps us identify libusb's pipe errors, which bubble
# up as generic USBErrors with errno 60 on affected platforms.
LIBUSB_TIMEOUT = 60
LIBUSB_IO_ERROR = 5


class GlitchKitUSB(GlitchKitModule):
    """
    """

    SHORT_NAME = 'usb'

    HOST_TRANSFER_QUEUED         = 0x002
    HOST_SETUP_TRANSFER_QUEUED   = 0x004
    HOST_IN_TRANSFER_QUEUED      = 0x008
    HOST_OUT_TRANSFER_QUEUED     = 0x010

    HOST_TRANSFER_COMPLETE       = 0x020
    HOST_SETUP_TRANSFER_COMPLETE = 0x040
    HOST_IN_TRANSFER_COMPLETE    = 0x100
    HOST_OUT_TRANSFER_COMPLETE   = 0x080

    DEVICE_TRANSFER_COMPLETE     = 0x200

    READ_INCOMPLETE = 0xFFFFFFFF

    # TODO: Figure out what should be in here vs in FaceDancer.
    GET_DESCRIPTOR        = 0x6
    GET_DEVICE_DESCRIPTOR = 1 << 8

    def __init__(self, board):
        """
        Create a new GlitchKit module allowing inducing or waiting for USB
        events, and then glitching.

        Args:
            board -- A representation of the GreatFET that will perform the actual
                triggering.
        """

        # Store a reference to the parent board.
        self.board = board


    @staticmethod
    def _decode_reg(reg):
        status_hex = codecs.encode(reg[::-1], 'hex')
        return int(status_hex, 16)


    @staticmethod
    def _split(value):
        value_high  = value >> 8
        value_low = value & 0xFF
        return [value_low, value_high]


    @staticmethod
    def build_request_type(is_in, type, recipient):

        request_type = 0

        if is_in:
            request_type |= (1 << 7)

        request_type |= (type << 5 )
        request_type |= (recipient)

        return request_type


    def build_setup_request(self, is_in=True, request_type=0, recipient=0, request=0, value=0, index=0, length=0):
        #       uint8_t request_type;
        #       uint8_t request;
        #       uint16_t value;
        #       uint16_t index;
        #       uint16_t length;

        setup_request = [self.build_request_type(is_in, request_type, recipient), request]
        setup_request.extend(self._split(value))
        setup_request.extend(self._split(index))
        setup_request.extend(self._split(length))
        return setup_request


    def capture_control_in(self, request_type=0, recipient=0, request=0, value=0, index=0, length=0, timeout=30):

        # Build a setup packet...
        setup_packet = self.build_setup_request(True, request_type, recipient, request, value, index, length)

        # ... start the request...
        self.board.vendor_request_out(vendor_requests.GLITCHKIT_USB_CONTROL_IN_START, data=setup_packet)

        # ... wait for a response...
        length_read = self.READ_INCOMPLETE
        while length_read == self.READ_INCOMPLETE:
            try:
                raw = self.board.vendor_request_in(vendor_requests.GLITCHKIT_USB_RESULT_LENGTH, length=4, timeout=timeout)
                length_read = self._decode_reg(raw)
            except usb.core.USBError as e:
                if e.errno in [LIBUSB_TIMEOUT, LIBUSB_IO_ERROR]:
                    pass
                else:
                    raise e

        # If we didn't read anything, don't bother querying the GreatFET; we know the result is an empty string.
        if length_read == 0:
            return b''

        #... and then read the response back.
        data = self.board.vendor_request_in(vendor_requests.GLITCHKIT_USB_READ_RESULT, length=length_read)
        return data




