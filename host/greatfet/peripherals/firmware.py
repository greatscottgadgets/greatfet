#
# This file is part of GreatFET
#

import array

from ..peripheral import GreatFETPeripheral


# TODO: pull into libgreat
class DeviceFirmwareManager(GreatFETPeripheral):
    """
    Class representing a flash memory used to work with a libgreat device's firmware.
    """

    def __init__(self, board):
        """Set up a new device firmware management connection.

        Args:
            board -- The GreatFETBoard that will be programming our flash chip.
        """

        if not board.supports_api('firmware'):
            raise NotImplementedError("The target board does not support the relevant API!")

        # Store a reference to the parent board, via which we'll program the
        # the actual SPI flash.
        self.board = board
        self.api = board.apis.firmware

        # Ask the device to perform initialization, and then grab its extents.
        self.page_size, self.maximum_address = self.api.initialize()


    def erase(self):
        """Erases the GreatFET's onboard SPI flash, clearing its program.

        CAUTION: After running this function, you'll need to use DFU mode to
        load a new GreatFET program onto the board. Be careful!
        """
        self.api.full_erase(timeout=10000)


    def write(self, data, address=0, erase_first=False, progress_callback=None):
        """Calls a given method on each 'page' of a range of flash.

        Note that this flash is used to store the program that runs when your
        GreatFET is plugged in. If you accidentally destroy that, you can
        restore by putting the device into DFU mode. Still, be careful!

        Args:
            data -- The data to be written, as a byte array or any form
                that can be used to initialize a Python array.array.
            address -- The address at which the data should start.
            erase_first -- If set, the flash will automatically be erased
                before writing.
            progress_callback -- Optional function that should accept two
                arguments-- the current progress, in bytes, and the total bytes
                to be written. Can be used to provide a progress indicator.
        """

        def perform_write(data_offset, data_to_write):
            data_slice = data_array[data_offset : data_offset + data_to_write]
            self._write_page(address + data_offset, data_slice)

        # Convert the data to an array of binary data for transmission.
        data_array = array.array('B', data)
        length = len(data_array)

        if address < 0:
            raise ValueError("Trying to write before the beginning of flash!")

        if (address + length - 1) > self.maximum_address:
            raise ValueError("Attempting to write past the end of flash!")

        if erase_first:
            self.erase()

        # And execute our write callback on each of the data sections.
        self._run_method_on_flash_pages(perform_write, address, length, progress_callback)


    def read(self, address=0, length=None, progress_callback=None):
        """Calls a given method on each 'page' of a range of flash.

        Note that this flash is used to store the program that runs when your
        GreatFET is plugged in. If you accidentally destroy that, you can
        restore by putting the device into DFU mode. Still, be careful!

        Args:
            data -- The data to be written, as a byte array or any form
                that can be used to initialize a Python array.array.
            address -- The address at which the data should start; default to zero.
            length -- The length to read; defaults to the remainder of the flash.
            progress_callback -- Optional function that should accept two
                arguments-- the current progress, in bytes, and the total bytes
                to be read. Can be used to provide a progress indicator.
        """

        def perform_read(data_offset, data_to_read):
            return self._read_page(address + data_offset, data_to_read)

        # If no length is provided, assume the rest of the flash.
        if length is None:
            length = self.maximum_address - address

        if address < 0:
            raise ValueError("Trying to read before the beginning of flash!")

        if (address + length - 1) > self.maximum_address:
            raise ValueError("Attempting to read past the end of flash!")

        # And execute our write callback on each of the data sections.
        return self._run_method_on_flash_pages(perform_read, address, length, progress_callback=progress_callback)


    def _run_method_on_flash_pages(self, method, address, length, progress_callback=None):
        """Calls a given method on each 'page' of a range of flash.

        Note that this flash is used to store the program that runs when your
        GreatFET is plugged in. If you accidentally destroy that, you can
        restore by putting the device into DFU mode. Still, be careful!

        Args:
            method -- The method to be called. Should accept two parameters--
                the offset into the flash at which we should call the method,
            address -- The address at which the operation should start.
            progress_callback -- Optional function that should accept two
                arguments-- the current progress, in bytes, and the total bytes
                to be handled. Can be used to provide a progress indicator.
        """

        results = array.array('B')

        data_offset = 0
        left_to_handle = length

        while left_to_handle > 0:

            # Call our method on no more than page at a time.
            data_to_handle = min(self.page_size, left_to_handle)

            # Call our data-handling method.
            result = method(data_offset, data_to_handle)
            if result:
                results.extend(result)

            # ... and move forward by the total amount of data written.
            data_offset += data_to_handle
            left_to_handle -= data_to_handle

            # Issue our progress callback.
            if progress_callback:
                progress_callback(data_offset, length)

        return results



    def _write_page(self, address, data):
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
        data_array = bytearray(data)
        length = len(data_array)

        if address < 0:
            raise ValueError("Trying to write before the beginning of flash!")

        if (address + length - 1) > self.maximum_address:
            raise ValueError("Attempting to write past the end of flash!")

        if (length > self.page_size):
            raise ValueError("Attempting to use page function to write more than a page!")

        # If our data doesn't fill a full page (e.g. this is the last page in a long write),
        # pad it out to the maximum length.
        if length < self.page_size:
            pad_length = self.page_size - length
            data_array.extend(b"\0" * pad_length)

        # Perform the actual write. Note that this may take time, as we have to
        # wait for the flash chip to perform the write.
        self.api.write_page(address, bytes(data_array), timeout=30000)


    def _read_page(self, address, length):
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
        if (address + length - 1) > self.maximum_address:
            raise ValueError("Attempting to read past the end of flash!")

        if (length > self.page_size):
            raise ValueError("Attempting to use page function to read more than a page!")

        # Perform the actual write. Note that this may take time, as we have to
        # wait for the flash chip to perform the write.
        return self.api.read_page(address)
