#
# This file is part of libgreat
#

"""
Module containing the core definitions for a libgreat-driven board.
"""

# FIXME: remove dependencies 
import usb

import time
import pygreat

# Use the GreatFET comms API, and the standard (core) API.
from pygreat.comms import CommsBackend

from .errors import DeviceNotFoundError

# Total seconds we should wait after a reset before reconnecting.
RECONNECT_DELAY = 3

# Quirk constant that helps us identify libusb's pipe errors, which bubble
# up as generic USBErrors with errno 32 on affected platforms.
# FIXME: remove
LIBUSB_PIPE_ERROR = 32

class GreatBoard(object):
    """
    Class representing a USB-connected GreatFET device.
    """

    """
    The libgreat board IDs handled by this class. Used by the default
    implementation of accepts_connected_device() to determine if a given subclass
    handles the given board ID.
    """
    HANDLED_BOARD_IDS = []

    """
    The display name of the given board. Subclasses should override
    this with a more appropriate name.
    """
    BOARD_NAME = "Unknown libgreat board"

    # Default device identifiers.
    BOARD_VENDOR_ID = 0
    BOARD_PRODUCT_ID = 0

    @classmethod
    def autodetect(cls, **device_identifiers):
        """
        Attempts to create a new instance of the GreatBoard subclass
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

                # Create an instance of the device to return,
                # and ensure that device has fully populated comms APIs.
                board = subclass(**device_identifiers)
                board.initialize_apis()

                return board

        # If we couldn't find a board, raise an error.
        raise DeviceNotFoundError()


    @classmethod
    def autodetect_all(cls, **device_identifiers):
        """
        Attempts to create a new instance of the GreatBoard subclass
        most applicable for each board present on the system-- similar to the
        behavior of autodetect.

        Accepts the same arguments as pyusb's usb.find() method, allowing narrowing
        to a more specific GreatFET by e.g. serial number.

        Returns a list of GreatFET devices, which may be empty if none are found.
        """

        devices = []

        # Iterate over each subclass of GreatFETBoard until we find a board
        # that accepts the given board ID.
        for subclass in cls.__subclasses__():

            # Get objects for all devices accepted by the given subclass.
            subclass_devices = subclass.all_accepted_devices(**device_identifiers)

            # FIXME: It's possible that two classes may choose to both advertise support
            # for the same device, in which case we'd wind up with duplicats here. We could
            # try to filter out duplicates using e.g. USB bus/device, but that assumes
            # things are USB connected.
            devices.extend(subclass_devices)

        # Ensure each device has its comms objects fully populated.
        for device in devices:
            device.initialize_apis()

        # Return the list of all subclasses.
        return devices


    @classmethod
    def all_accepted_devices(cls, **device_identifiers):
        """
        Returns a list of all devices supported by the given class. This should be
        overridden if the device connects via anything other that USB.

        Accepts the same arguments as pyusb's usb.find() method, allowing narrowing
        to a more specific GreatFET by e.g. serial number.
        """

        devices = []

        # Grab the list of all devices that we theoretically could use.
        # FIXME: use the comms backend for this!
        identifiers = cls.populate_default_identifiers(device_identifiers, find_all=True)
        raw_devices = usb.core.find(**identifiers)

        # Iterate over all of the connected devices, and filter out the devices
        # that this class doesn't connect.
        for raw_device in raw_devices:

            # We need to be specific about which device in particular we're
            # grabbing when we query things-- or we'll get the first acceptable
            # device every time. The trick here is to populate enough information
            # into the identifier to uniquely identify the device. The address
            # should do, as pyusb is only touching enmerated devices.
            identifiers['address'] = raw_device.address
            identifiers['find_all'] = False

            # If we support the relevant device _instance_, and it to our list.
            if cls.accepts_connected_device(**identifiers):
                devices.append(cls(**identifiers))

        return devices



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
        except usb.core.USBError as e:

            # A pipe error here likely means the device didn't support a start-up
            # command, and STALLED.
            # We'll interpret that as a "we don't accept this device" by default.
            if e.errno == LIBUSB_PIPE_ERROR:
                return False
            else:
                raise e

        try:
            board_id = potential_device.board_id()
        finally:
            potential_device.close()

        # Accept only GreatFET devices whose board IDs are handled by this
        # class. This is mostly used by subclasses, which should override
        # HANDLED_BOARD_IDS.
        return board_id in cls.HANDLED_BOARD_IDS


    @classmethod
    def populate_default_identifiers(cls, device_identifiers, find_all=False):
        """
        Populate a dictionary of default identifiers-- which can
        be overridden or extended by arguments to the function.

        device_identifiers -- any user-specified identifers; will override
            the default identifiers in the event of a conflit
        """

        # By default, accept any device with the default vendor/product IDs.
        identifiers = {
            'idVendor': cls.BOARD_VENDOR_ID,
            'idProduct': cls.BOARD_PRODUCT_ID,
            'find_all': find_all,
        }
        identifiers.update(device_identifiers)

        return identifiers



    def __init__(self, **device_identifiers):
        """
        Instantiates a new connection to a libgreat device; by default connects
        to the first available board.

        Accepts the same arguments as pyusb's usb.find() method, allowing narrowing
        to a more specific board by serial number.

        FIXME: accept an identifier string or a comms_backend object instead of the
        array above
        """

        # By default, accept any device with the default vendor/product IDs.
        self.identifiers = self.populate_default_identifiers(device_identifiers)

        # For convenience, allow serial_number=None to be equivalent to not
        # providing a serial number: a board with any serial number will be
        # accepted.
        if 'serial_number' in self.identifiers and self.identifiers['serial_number'] is None:
            del self.identifiers['serial_number']

        # TODO: replace this with a comms_string
        # Create our backend connection to the device.
        self.comms = CommsBackend.from_device_uri(**self.identifiers)

        # Get an object that allows easy access to each of our APIs.
        self.apis = self.comms.generate_api_object()

        # TODO: optionally use the core API to discover other APIs

        # Final sanity check: if we don't handle this board ID, bail out!
        if self.HANDLED_BOARD_IDS and (self.board_id() not in self.HANDLED_BOARD_IDS):
            raise DeviceNotFoundError()


    def initialize_apis(self):
        """ Hook-point for sub-boards to initialize their APIs after
            we have comms up and running and auto-enumeration is complete.
        """

        # Run the auto-enumeration.
        self.comms.run_autoenumeration()


    def supports_api(self, class_name):
        """ Returns true iff the board supports the given API class. """
        return (class_name in self.comms.apis)


    def board_id(self):
        """Reads the board ID number for the device."""
        return self.apis.core.read_board_id()


    def board_name(self):
        """Returns the human-readable product-name for the device."""
        return self.BOARD_NAME


    def firmware_version(self):
        """Reads the board's firmware version."""
        return self.apis.core.read_version_string(encoding='UTF-8')


    def serial_number(self, as_hex_string=True):
        """Reads the board's unique serial number."""

        result = self.apis.core.read_serial_number()

        # If we've been asked to convert this to a hex string, do so.
        if as_hex_string:
            result = _to_hex_string(result)

        return result


    def part_id(self, as_hex_string=True):
        """Reads the board's unique serial number."""

        result = self.apis.core.read_part_id()

        if as_hex_string:
            result = _to_hex_string(result)

        return result


    def reset(self, reconnect=True, switch_to_external_clock=False):
        """
        Reset the device.

        Arguments:
            reconect -- If True, this method will wait for the device to
                finish the reset and then attempt to reconnect.
            switch_to_external_clock -- If true, the device will accept a 12MHz
                clock signal on P4_7 (J2_P11 on the GreatFET one) after the reset.
        """

        # FIXME: abstract
        reset_type = 1 if switch_to_external_clock else 0

        try:
            self.apis.core.request_reset(reset_type)
        except usb.core.USBError as e:
            pass

        # If we're to attempt a reconnect, do so.
        connected = False
        if reconnect:
            time.sleep(RECONNECT_DELAY)
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
        Dispose resources allocated by this connection.  This connection
        will no longer be usable.
        """
        self.comms.close()


    def read_debug_ring(self, max_length=4096, clear=False, encoding='latin1'):
        """ Requests the GreatFET's debug ring.

        Args:
            max_length -- The maximum length to respond with. Must be less than 65536.
            clear -- True iff the dmesg buffer should be cleared after the request.
        """

        if not self.supports_api('debug'):
            return "The connected board doesn't suppor the debug API. This usually isn't right."

        # Determine which command we should use to read the debug ring, and then use it.
        command = self.apis.debug.clear_dmesg if clear else self.apis.debug.read_dmesg
        return command(max_response_length=max_length, encoding=encoding)


    def dmesg(self, max_length=4096, clear=False, encoding='latin1'):
        """ Prints the GreatFET's debug ring. Convenience function.
        To grab the debug ring's contents without printing, see read_debug_ring().

        Args:
            max_length -- The maximum length to respond with. Must be less than 65536.
            clear -- True iff the dmesg buffer should be cleared after the request.
        """
        print(self.read_debug_ring(max_length, clear, encoding))


def _to_hex_string(byte_array):
    """Convert a byte array to a hex string."""
    hex_generator = ('{:02x}'.format(x) for x in byte_array)
    return ''.join(hex_generator)

