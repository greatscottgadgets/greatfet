#
# This file is part of GreatFET
#

from ..i2c_device import I2CDevice

class I2CRegisterBasedDevice(I2CDevice):
    """ Class representing devices which follow a standard I2C 'register' model.

    This class is meant to provide a register-like interface for devices that primarily interface via two operations:

        register write: [ write_addr register_number data ]
        register read:  [ write_addr register_number [ read_addr data ]

    where [ indicates a start bit, and ] indicates a stop bit.
"""
    #
    # The following fields provide some 'sane' defaults for subclasses. You'll want to override these as
    # appropriate for your subclass-represented device.
    #

    # Don't make any assumptions about device address, here, but allow subclasses to provide this.
    # This should be the core address without the read or write bit.
    DEVICE_ADDRESS = None

    # For most devices, assume that register addresses are one byte wide, as this is the most common.
    REGISTER_ADDRESS_WIDTH_BYTES = 1

    # Assume that registers are one byte wide, as well. This is common enough; but less common than
    # single-byte addresses. This will likely be overridden, more.
    REGISTER_WIDTH_BYTES = 1

    # Assume little-endian writes by default. This should be set to false
    DATA_IS_LITTLE_ENDIAN = True

    # This dictionary field allows you to provide a basic mapping of register name strings to their addresses.
    # If provided, this will be used to provide simple read/write properties for each of the subordinate registers.
    # As an example, {'DIRECTION': 0x01} would create a .DIRECTION register that can be read from or written to to
    # read/write from the register at address 0x01.
    REGISTER_MAP = {}


    def __init__(self, i2c_bus, device_address=None, register_width_bytes=None,
        register_address_width_bytes=None, is_little_endian=None):
        """ Creates a new register-based I2C device.

        I2CRegisterBasedDevice is directly instantiable, to allow for quick prototyping; but for longer-term use you
        should create new class that subclasses it.

        Parameters:
            i2c_bus -- The I2C bus to connect the device on.
            device_address -- The address to use for the device, without the read or write bits. If this isn't provided,
                the class' default will be used. For I2CRegisterBasedDevice, there is no default, and this
                must be provided.
            register_width_bytes -- The width of a register, in bytes. If not provided, a class default will be assumed.
            register_address_width_bytes -- The width of a register address. If not provided, a class default will be used.
            is_little_endian -- True iff the device accepts its addresses and data with little endian byte ordering.
                If not provided, a class default will be used.
        """

        # Store our basic communications parameters.
        self._register_width = register_width_bytes if register_width_bytes else self.REGISTER_WIDTH_BYTES
        self._register_address_width = register_address_width_bytes if register_address_width_bytes \
                else self.REGISTER_ADDRESS_WIDTH_BYTES
        self._data_is_little_endian = is_little_endian if (is_little_endian is not None) else self.DATA_IS_LITTLE_ENDIAN
        self._register_names = dict(self.REGISTER_MAP)

        # Initialize our inner I2C device.
        super(I2CRegisterBasedDevice, self).__init__(i2c_bus, device_address, name=self.__class__.__name__)

        # Finally, once our initial implementation is complete, apply our __setattr__ handler.
        self.__setattr__ = self.__setattr_to_apply__


    def _get_byte_order(self):
        """ Returns a byteorder description suitable for use as a parameter to to_bytes / from_bytes. """
        return 'little' if self._data_is_little_endian else 'big'


    def write(self, register_address, register_value):
        """ Writes the provided value to the given register address. """

        # Build the I2C payload for the relevant write command...
        byte_order = self._get_byte_order()
        i2c_payload = \
            register_address.to_bytes(self._register_address_width, byteorder=byte_order) + \
            register_value.to_bytes(self._register_width, byteorder=byte_order)

        # ... and issue the write to the device.
        super(I2CRegisterBasedDevice, self).write(i2c_payload)


    def read(self, register_address):
        """ Reads the data from the provided register address. """

        # Build the I2C payload for the relevant write command...
        byte_order = self._get_byte_order()
        read_command = register_address.to_bytes(self._register_address_width, byteorder=byte_order)

        # ... and issue the read command to the device.
        value = super(I2CRegisterBasedDevice, self).transmit(read_command, self._register_width)

        # ... finally, convert the raw bytes received back into a register value.
        return int.from_bytes(value, byteorder=byte_order)


    def __getitem__(self, key):
        """ Allows device[addr] or device[name] to read the relevant register. """

        if isinstance(key, str):
            key = self._register_names[key]

        return self.read(key)


    def __setitem__(self, key, value):
        """ Allows device[addr] or device[name] to be used to assign values to the relevant register. """

        if isinstance(key, str):
            key = self._register_names[key]

        self.write(key, value)


    def __getattr__(self, name):
        """ Allows device.register_name to be used to read the register with the given name. """

        addr = self._register_names[name]
        return self.read(addr)


    def __setattr_to_apply__(self, name, value):
        """ Allows device.register_name to be used to read the register with the given name. """

        # But for any write where the parameter isn't found, go ahead.
        addr = self._register_names[name]
        self.write(addr, value)


    def __dir__(self):
        """ Returns a list of the interesting attributes of this object."""

        # Start off with any attributes that represent registers for our device.
        attrs = list(self._register_names.keys())

        # ... and add in our read and write functions.
        attrs.extend(['read', 'write'])

        return attrs


    def _set_bit_in_register(self, register, bit_number, set_to_one):
        """ Sets the provided bit in the given register.

        Parameters:
            register   -- The name or address of the register to work with.
            bit_number -- The bit number to set.
            set_to_one -- If true (or equivalent), the bit will be set to one. If falsey,
                the bit will be cleared.
        """
        pin_mask = (1 << bit_number)

        # Generate a number that's equivalent to having a register-width worth of 1's.
        # This gives us a number we can subtract from in order to do a proper-width inversion.
        register_width_bits     = (self._register_width * 8)
        register_inversion_mask = (1 << register_width_bits) - 1

        if set_to_one:
            self[register] |= pin_mask
        else:
            self[register] &= register_inversion_mask - pin_mask


    def _get_bit_in_register(self, register, bit_number):
        """ Returns the given bit in the provided register.

        Parameters:
            register   -- The name or address of the register to work with.
            bit_number -- The bit number to get.

        Returns True iff the given bit is one, or False otherwise.
        """
        pin_mask = (1 << bit_number)
        return bool(self[register] & pin_mask)
