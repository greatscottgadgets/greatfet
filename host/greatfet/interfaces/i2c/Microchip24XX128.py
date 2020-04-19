from ..i2c_device import I2CDevice
import time

ROM_SIZE            = 0x4000
ROM_PAGE_SIZE       = 64
READ_BUFFER_SIZE    = 127
BASE_DEVICE_ADDRESS = 0x50

class Microchip24XX128(I2CDevice):
    """
        Class representing a 24XX128 128k EEPROM connected to a GreatFET I2C bus.
    """

    @staticmethod
    def encodeaddress(address):
        h = address >> 8
        l = address & 0xFF
        return bytes([h, l])

    def __init__(self, i2c_bus, chip_select_bits=0):
        """ Creates a new Microchip 24XX128 EEPROM I2C Device

        Microchip24XX128 is directly instantiable.

        Parameters:
            i2c_bus -- The I2C bus to connect the device on.
            chip_select_bits -- The value of chip select pins A1-A13 that determine the lower bits of the device address. Defaults to 0b000.
        """

        if chip_select_bits > 7 or chip_select_bits < 0:
            raise ValueError("chip_select_bits must be in range 0b000 to 0b111.")

        device_address = BASE_DEVICE_ADDRESS | (0x07 & chip_select_bits)
        
        # Initialize our inner I2C device.
        super().__init__(i2c_bus, device_address, name=self.__class__.__name__)
    
    def read_bytes(self, start_address, end_address):
        """
            Read bytes sequentially from a specified memory address as a bytestring.
            
            Parameters:
                start_address -- Address of the start of the block to read.
                end_address   -- Address of the end of the block, inclusive.
        """

        if start_address<0 or start_address>=ROM_SIZE:
            raise ValueError("Invalid start address.")
        if end_address<0 or end_address>=ROM_SIZE:
            raise ValueError("Invalid end address.")
        if end_address<start_address:
            raise ValueError("End address must come after start.")

        buff = b''

        # Write out start memory address to the address pointer register
        self.write(Microchip24XX128.encodeaddress(start_address))
        addr = start_address

        buff_size = self.bus.buffer_size

        # Read bytes in chunks matched to the read buffer size (default 127 bytes)
        while addr<=end_address:
            bytes_to_read = (end_address - addr) + 1
            read_data = self.read(min(bytes_to_read, buff_size))
            buff = buff + bytes(read_data)
            addr = addr + buff_size

        return buff

    def write_bytes(self, memory_address, data, write_cycle_length=0.005, attempts=2):
        """
            Write bytes sequentially starting at a specified memory address. Will read data back to assure write was successful.

            Parameters:
                memory_address     -- Address of the start of the block to write.
                data               -- Data to write, as a bytestring
                write_cycle_length -- Length to pause for each page write in seconds. Defaults to 5ms, ie 0.005
                attempts           -- The number of attempts made to write to the ROM and verify the contents before giving up. Defaults to 2.
        """

        bytes_to_write = len(data)
        data = bytes(data)              # What if it's something weird and not a bytestring

        # We need to break the bytes up so that we don't write past the edge of a page buffer
        i = 0
        retries = attempts
        while i<=bytes_to_write-1:
            addr = memory_address + i
            writeable_bytes = addr % ROM_PAGE_SIZE
            writeable_bytes = writeable_bytes if writeable_bytes != 0 else 64
            l = min(bytes_to_write - i, writeable_bytes)
            b = Microchip24XX128.encodeaddress(addr) + data[i:i + l]
            self.write(b)
            time.sleep(write_cycle_length)
            written_bytes = self.read_bytes(addr, addr+(l-1))
            if b[2:] == written_bytes:
                i = i + l
                retries = attempts
            else:
                retries = retries - 1
                if retries == 0:
                    raise RuntimeError("Could not write to EEPROM.")