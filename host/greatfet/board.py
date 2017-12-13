#
# This file is part of GreatFET
#

"""
Module containing the core definitions for a GreatFET board.
"""

import usb

from .protocol import vendor_requests
from .errors import DeviceNotFoundError
from .peripherals.led import LED
from .peripherals.gpio import GPIO

# Default device identifiers.
GREATFET_VENDOR_ID = 0x1d50
GREATFET_PRODUCT_ID = 0x60e6

# Quirk constant that helps us identify libusb's pipe errors, which bubble
# up as generic USBErrors with errno 32 on affected platforms.
LIBUSB_PIPE_ERROR = 32


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


    """
    The mappings from GPIO names to port numbers. Paths in names can be delineated
    with underscores to group gpios. For example, if Jumper 7, Pin 3 is Port 5, Pin 11,
    you could add an entry that reads "J7_P3": (5, 11).
    """
    GPIO_MAPPINGS = {}


    @classmethod
    def autodetect(cls, **device_identifiers):
        """
        Attempts to create a new instance of the GreatFETBoard subclass
        most applicable to the given device. For example, if the attached
        board is a GreatFET One, this will automatically create a
        GreatFET One object.

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
        except DeviceNotFoundError:
            return False

        try:
            board_id = potential_device.board_id()
        finally:
            potential_device.close()

        # Accept only GreatFET devices whose board IDs are handled by this
        # class. This is mostly used by subclasses, which should override
        # HANDLED_BOARD_IDS.
        return board_id in cls.HANDLED_BOARD_IDS


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
        identifiers.update(device_identifiers)

        # For convenience, allow serial_number=None to be equivalent to not
        # providing a serial number: a  GreatFET with any serail number will be
        # accepted.
        if 'serial_number' in identifiers and identifiers['serial_number'] is None:
            del identifiers['serial_number']

        # Connect to the first available GreatFET device.
        try:
            self.device = usb.core.find(**identifiers)
        except usb.core.USBError as e:
            # On some platforms, providing identifiers that don't match with any
            # real device produces a USBError/Pipe Error. We'll convert it into a
            # DeviceNotFoundError.
            if e.errno == LIBUSB_PIPE_ERROR:
                raise DeviceNotFoundError()
            else:
                raise e

        # If we couldn't find a GreatFET, bail out early.
        if self.device is None:
            raise DeviceNotFoundError()

        # Ensure that we have an active USB connection to the device.
        self._initialize_usb()

        # Final sanity check: if we don't handle this board ID, bail out!
        if self.HANDLED_BOARD_IDS and (self.board_id() not in self.HANDLED_BOARD_IDS):
            raise DeviceNotFoundError()


    def _initialize_usb(self):
        """Sets up our USB connection to the GreatFET device."""

        # For now, the GreatFET is only providing a single configuration, so we
        # can accept the first configuration provided.
        self.device.set_configuration()


    def board_id(self):
        """Reads the board ID number for the GreatFET device."""

        # Query the board for its ID number.
        response = self.vendor_request_in(vendor_requests.READ_BOARD_ID, length=1)
        return response[0]


    def board_name(self):
        """Returns the human-readable product-name for the GreatFET device."""
        return self.BOARD_NAME


    def firmware_version(self):
        """Reads the board's firmware version."""

        # Query the board for its firmware version, and convert that to a string.
        return self.vendor_request_in_string(vendor_requests.READ_VERSION_STRING, length=255)


    def serial_number(self, as_hex_string=True):
        """Reads the board's unique serial number."""
        result = self.vendor_request_in(vendor_requests.READ_PARTID_SERIALNO, length=24)

        # The serial number starts eight bytes in.
        result = result[8:]

        # If we've been asked to convert this to a hex string, do so.
        if as_hex_string:
            result = _to_hex_string(result)

        return result


    def part_id(self, as_hex_string=True):
        """Reads the board's unique serial number."""
        result = self.vendor_request_in(vendor_requests.READ_PARTID_SERIALNO, length=24)

        # The part ID constitues the first eight bytes of the response.
        result = result[0:7]
        if as_hex_string:
            result = _to_hex_string(result)

        return result


    def reset(self):
        """Reset the GreatFET device."""
        self.vendor_request_out(vendor_requests.RESET)


    def close(self):
        """
        Dispose pyUSB resources allocated by this connection.  This connection
        will no longer be usable.
        """
        usb.util.dispose_resources(self.device)


    def _vendor_request(self, direction, request, length_or_data=0, value=0, index=0, timeout=1000):
        """Performs a USB vendor-specific control request.

        See also _vendor_request_in()/_vendor_request_out(), which provide a
        simpler syntax for simple requests.

        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            value -- The value to be passed to the vendor request.

        For IN requests:
            length_or_data -- The length of the data expected in response from the request.
        For OUT requests:
            length_or_data -- The data to be sent to the device.
        """
        return self.device.ctrl_transfer(
            direction | usb.TYPE_VENDOR | usb.RECIP_DEVICE,
            request, value, index, length_or_data, timeout)


    def vendor_request_in(self, request, length, value=0, index=0, timeout=1000):
        """Performs a USB control request that expects a respnose from the GreatFET.

        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            length -- The length of the data expected in response from the request.
        """
        return self._vendor_request(usb.ENDPOINT_IN, request, length,
            value=value, index=index, timeout=timeout)


    def vendor_request_in_string(self, request, length=255, value=0, index=0, timeout=1000):
        """Performs a USB control request that expects a respnose from the GreatFET.

        Interprets the result as a UTF-8 encoded string.

        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            length -- The length of the data expected in response from the request.
        """
        raw = self._vendor_request(usb.ENDPOINT_IN, request, length_or_data=length,
            value=value, index=index, timeout=timeout)
        return raw.tostring().decode('utf-8')


    def vendor_request_out(self, request, value=0, index=0, data=None, timeout=1000):
        """Performs a USB control request that provides data to the GreatFET.

        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            value -- The value to be passed to the vendor request.
        """
        return self._vendor_request(usb.ENDPOINT_OUT, request, value=value,
            index=index, length_or_data=data, timeout=timeout)


    def _populate_leds(self, led_count):
        """Adds the standard set of LEDs to the board object.

        Args:
            led_count -- The number of LEDS present on the board.
        """
        self.leds = {}
        for i in range(1, led_count + 1):
            self.leds[i] = LED(self, i)


    def _populate_gpio(self):
        """Adds GPIO pin definitions to the board's main GPIO object."""

        self.gpio = GPIO(self)

        # Handle each GPIO mapping.
        for name, pin in self.GPIO_MAPPINGS.iteritems():
            self.gpio.register_gpio(name, pin)


def _to_hex_string(byte_array):
    """Convert a byte array to a hex string."""
    hex_generator = ('{:02x}'.format(x) for x in byte_array)
    return ''.join(hex_generator)
