#
# This file is part of GreatFET
#

from __future__ import absolute_import

import usb
import time
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
    VBUS_ENABLED                 = 0x400

    READ_INCOMPLETE = 0xFFFFFFFF

    PRE_RESPONSE_DELAY = 0.01


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
        self.api = board.apis.glitchkit_usb


    @staticmethod
    def supports_board(board):
        """ Determines if this GreatFET supports GlitchKit via USB. """
        return board.supports_api("glitchkit_usb")


    def configure_future_requests(self, continue_despite_errors, disable_vbus_afterwards):
        """ Configure future requests made by this GlitchKit module.

        Arguments:
            continue_despite_errors -- True iff stimuli should continue even
                if errors occur.
            disable_vbus_afterwards -- If set, VBUS will be disconnected after
                a given USB request.
        """
        self.api.configure_requests(continue_despite_errors, disable_vbus_afterwards)


    @staticmethod
    def _split(value):
        # TODO: get rid of this

        value_high  = value >> 8
        value_low = value & 0xFF
        return [value_low, value_high]


    @staticmethod
    def build_request_type(is_in, type, recipient):

        # TODO: FIXME: clean up consts
        request_type = 0

        if is_in:
            request_type |= (1 << 7)

        request_type |= (type << 5)
        request_type |= (recipient)

        return request_type


    def build_setup_request(self, is_in=True, request_type=0, recipient=0, request=0, value=0, index=0, length=0):
        #       uint8_t request_type;
        #       uint8_t request;
        #       uint16_t value;
        #       uint16_t index;
        #       uint16_t length;

        # TODO: replace me with a call to struct.pack?
        setup_request = [self.build_request_type(is_in, request_type, recipient), request]
        setup_request.extend(self._split(value))
        setup_request.extend(self._split(index))
        setup_request.extend(self._split(length))
        return setup_request


    def capture_control_in(self, request_type=0, recipient=0, request=0, value=0, index=0, length=0, timeout=30, ui_event_call=False):

        # Build a setup packet...
        setup_packet = bytes(self.build_setup_request(True, request_type, recipient, request, value, index, length))

        # ... and issue the request.
        return self.api.control_in(setup_packet, timeout=timeout * 1024)

