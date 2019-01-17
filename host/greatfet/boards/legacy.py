#
# This file is part of GreatFET
#

import array
import struct

from ..board import GreatFETBoard
from ..peripherals.firmware import DeviceFirmwareManager
from ..errors import DeviceNotFoundError

class GreatFETLegacy(GreatFETBoard):
    """ Class representing legacy-firmware (pre-libgreat) GreatFET boards.
    This class exists so Ancient GreatFETs still show up in GreatFET info and can
    be upgraded to the modern firmware stack.
    """

    # Legacy USB vendor requests.
    REQUEST_READ_BOARD_ID        = 4
    REQUEST_READ_VERSION_STRING  = 5
    REQUEST_READ_PARTID_SERIALNO = 6
    REQUEST_RESET                = 22
    REQUEST_READ_DMESG           = 64

    # Currently, all GreatFET One boards have an ID of zero.
    HANDLED_BOARD_IDS = [0]
    BOARD_NAME = "GreatFET [legacy FW]"

    # Reconnect delay after reset.
    RECONNECT_DELAY = 5

    # LibUSB constant for a stall / pipe error.
    LIBUSB_PIPE_ERROR = 32

    def version_warnings(self):
        """ Notify the user that their GreatFET requires an urgent upgrade.  """
        return "This device's firmware is too out of date! It must be upgraded before it can be used. " + \
            "See https://github.com/greatscottgadgets/greatfet/wiki for more information."


    def __init__(self, **device_identifiers):
        """
        Intiailize our connection to the device.
        """
        import usb

        # By default, accept any device with the default vendor/product IDs.
        self.identifiers = self.populate_default_identifiers(device_identifiers)

        # For convenience, allow serial_number=None to be equivalent to not
        # providing a serial number: a  GreatFET with any serail number will be
        # accepted.
        if 'serial_number' in self.identifiers and self.identifiers['serial_number'] is None:
            del self.identifiers['serial_number']

        # Connect to the first available GreatFET device.
        try:
            self.device = usb.core.find(**self.identifiers)
        except usb.core.USBError as e:
            # On some platforms, providing identifiers that don't match with any
            # real device produces a USBError/Pipe Error. We'll convert it into a
            # DeviceNotFoundError.
            if e.errno == self.LIBUSB_PIPE_ERROR:
                raise DeviceNotFoundError()
            else:
                raise e

        # If we couldn't find a GreatFET, bail out early.
        if self.device is None:
            raise DeviceNotFoundError()

        # Ensure that we have an active USB connection to the device.
        self.device.set_configuration()

        # Final sanity check: if we don't handle this board ID, bail out!
        if self.HANDLED_BOARD_IDS and (self.board_id() not in self.HANDLED_BOARD_IDS):
            raise DeviceNotFoundError()

        # Final sanity check: if we don't handle this board ID, bail out!
        if self.HANDLED_BOARD_IDS and (self.board_id() not in self.HANDLED_BOARD_IDS):
            raise DeviceNotFoundError()


    def initialize_apis(self):
        """ We support only a LegacyFirmware API, which we use for upgrading the board. """

        # Create an adapter firmware collection that allows us to provide compatible API
        # objects with future APIs.
        self.apis = LegacyAPICollection(self)

        # Create an adapter that behaves like a CommsBackend, so most tools can work with
        # this object propertly.
        apis_dict = {'firmware': self.apis.firmware}
        self.comms = type('LegacyCommsAdapter', (), {'apis': apis_dict})()

        # Finally, add a DeviceFirmwareManager object to the given device.
        # This doesn't need anything special, as our LegacyAPICollection presents a
        # fully-API-compatible firmware API.
        if self.supports_api('firmware'):
            self.onboard_flash = DeviceFirmwareManager(self)


    def supports_api(self, class_name):
        """ Returns true iff the board supports the given API class. """
        return class_name == "firmware"


    def board_id(self):
        """Reads the board ID number for the GreatFET device."""

        # Query the board for its ID number.
        response = self.vendor_request_in(self.REQUEST_READ_BOARD_ID, length=1)
        return response[0]


    def board_name(self):
        """Returns the human-readable product-name for the GreatFET device."""
        return self.BOARD_NAME


    def firmware_version(self):
        """Reads the board's firmware version."""

        # Query the board for its firmware version, and convert that to a string.
        return self.vendor_request_in_string(self.REQUEST_READ_VERSION_STRING, length=255)


    def serial_number(self, as_hex_string=True):
        """Reads the board's unique serial number."""
        result = self.vendor_request_in(self.REQUEST_READ_PARTID_SERIALNO, length=24)

        # The serial number starts eight bytes in.
        result = result[8:]

        # If we've been asked to convert this to a hex string, do so.
        if as_hex_string:
            result = _to_hex_string(result)

        return result


    def usb_serial_number(self):
        """ Reports the device's USB serial number. """
        return self.device.serial_number


    def part_id(self, as_hex_string=True):
        """Reads the board's unique serial number."""
        result = self.vendor_request_in(self.REQUEST_READ_PARTID_SERIALNO, length=24)

        # The part ID constitues the first eight bytes of the response.
        result = result[0:7]
        if as_hex_string:
            result = _to_hex_string(result)

        return result


    def reset(self, reconnect=True, switch_to_external_clock=False):
        """
        Reset the GreatFET device.
        Arguments:
            reconect -- If True, this method will wait for the device to
                finish the reset and then attempt to reconnect.
            switch_to_external_clock -- If true, the device will accept a 12MHz
                clock signal on P4_7 (J2_P11 on the GreatFET one) after the reset.
        """

        import usb
        import time

        type = 1 if switch_to_external_clock else 0

        try:
            self.vendor_request_out(self.REQUEST_RESET, value=type)
        except usb.core.USBError:
            pass

        # If we're to attempt a reconnect, do so.
        if reconnect:
            time.sleep(self.RECONNECT_DELAY)
            self.__init__(**self.identifiers)

            # FIXME: issue a reset to all device peripherals with state, here?


    def switch_to_external_clock(self):
        """
        Resets the GreatFET, and starts it up again using an external clock
        source, rather than the onboard crystal oscillator.
        """
        self.reset(switch_to_external_clock=True)



    def close(self):
        """
        Dispose pyUSB resources allocated by this connection.  This connection
        will no longer be usable.
        """
        import usb

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
        import usb

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
        import usb

        return self._vendor_request(usb.ENDPOINT_IN, request, length,
            value=value, index=index, timeout=timeout)


    def vendor_request_in_string(self, request, length=255, value=0, index=0, timeout=1000,
            encoding='utf-8'):
        import usb

        """Performs a USB control request that expects a respnose from the GreatFET.
        Interprets the result as an encoded string.
        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            length -- The length of the data expected in response from the request.
        """
        raw = self._vendor_request(usb.ENDPOINT_IN, request, length_or_data=length,
            value=value, index=index, timeout=timeout)
        return raw.tostring().decode(encoding)


    def vendor_request_out(self, request, value=0, index=0, data=None, timeout=1000):
        """Performs a USB control request that provides data to the GreatFET.
        Args:
            request -- The number of the vendor request to be performed. Usually
                a constant from the protocol.vendor_requests module.
            value -- The value to be passed to the vendor request.
        """
        import usb

        return self._vendor_request(usb.ENDPOINT_OUT, request, value=value,
            index=index, length_or_data=data, timeout=timeout)


    def read_debug_ring(self, max_length=2047, clear=False, encoding='latin1'):
        """ Requests the GreatFET's debug ring.
        Args:
            max_length -- The maximum length to respond with. Must be less than 65536.
            clear -- True iff the dmesg buffer should be cleared after the request.
        """
        import usb

        return self.vendor_request_in_string(self.REQUEST_READ_DMESG,
                length=max_length, value= 1 if clear else 0, encoding=encoding)


def _to_hex_string(byte_array):
    """Convert a byte array to a hex string."""

    hex_generator = ('{:02x}'.format(x) for x in byte_array)
    return ''.join(hex_generator)


class LegacyFirmwareAdapter(object):
    """ API adapter for supporting firmware updates via the legacy comms protocol."""

    CLASS_NAME = "firmware"

    # Legacy vendor request numberse.
    REQUEST_SPIFLASH_INIT  = 0
    REQUEST_SPIFLASH_WRITE = 1
    REQUEST_SPIFLASH_READ  = 2
    REQUEST_SPIFLASH_ERASE = 3

    def __init__(self, board):
        """ Set up our adapter. """

        # Store our communications path to the given device.
        self.board = board

        # Store properties of the default SPI flash.
        self.page_size = 256
        self.pages = 8192
        self.maximum_address = 0x0FFFFF
        self.device_id = 0x14
        self.chip_select = 0x050B


    def initialize(self):
        """ Sets up our firmware-flashing interface. """

        # Configure the limitations for the legacy SPI flash.
        data = struct.pack("<HHIHB", self.page_size, self.pages, self.page_size * self.pages,
                self.chip_select, self.device_id)
        self.board.vendor_request_out(self.REQUEST_SPIFLASH_INIT, value=0, index=0, data=data, timeout=1000)

        # And return our knowledge of our limitations.
        return self.page_size, self.maximum_address


    def full_erase(self, timeout=1000):
        """ Erases the GreatFET's firmware. """
        self.board.vendor_request_out(self.REQUEST_SPIFLASH_ERASE)


    def write_page(self, address, data, timeout=1000):
        """Writes a page (or less) to the GreatFET's onboard SPI flash.
        Args:
            address -- The byte address to which the data should be written.
                Intended to be page-aligned, but this function will work even
                for unaligned inputs.
            length -- The length of the data to write. Must be equal to or less
                than this flash's page size, which is queryable via the page_size
                property.
            data -- The data to be written. Accepts any sequence that can be used
                as a parameter for the array __init__ method.
        """

        # Convert the data to an array of binary data for transmission.
        # (The vendor request function can do this for us, but doing it here
        #  allows us to compute the real byte-array length.)
        data_array = array.array('B', data)
        length = len(data_array)

        if address < 0:
            raise ValueError("Trying to write before the beginning of flash!")

        if (address + length - 1) > self.maximum_address:
            raise ValueError("Attempting to write past the end of flash!")

        if (length > self.page_size):
            raise ValueError("Attempting to use page function to write more than a page!")

        # Break the address down into two 16-bit words, as the target device expects
        # the address to be sent via the 16-bit value and index fields.
        address_high = address >> 16
        address_low = address & 0xFFFF

        # Perform the actual write. Note that this may take time, as we have to
        # wait for the flash chip to perform the write.
        self.board.vendor_request_out(self.REQUEST_SPIFLASH_WRITE,
            value=address_high, index=address_low, data=data_array, timeout=0)


    def read_page(self, address, timeout=1000):
        """Reads a page (or less) from the GreatFET's onboard SPI flash.
        Args:
            address -- The byte address from which the data should be read.
                Intended to be page-aligned, but this function will work even
                for unaligned inputs.
            length -- The length of the data to read. Must be equal to or less
                than this flash's page size, which is queryable via the page_size
                property.
        Returns the read data as an array of bytes.
        """

        length = self.page_size

        if (address + length - 1) > self.maximum_address:
            raise ValueError("Attempting to read past the end of flash!")

        if (length > self.page_size):
            raise ValueError("Attempting to use page function to read more than a page!")

        # Break the address down into two 16-bit words, as the target device expects
        # the address to be sent via the 16-bit value and index fields.
        address_high = address >> 16
        address_low = address & 0xFFFF

        # Perform the actual read.
        result = self.board.vendor_request_in(self.REQUEST_SPIFLASH_READ,
            value=address_high, index=address_low, length=length)
        return result


class LegacyAPICollection(object):

    def __init__(self, board):
        self.firmware = LegacyFirmwareAdapter(board)

