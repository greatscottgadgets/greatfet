import unittest
import greatfet.programmers.microchipEEPROM as microchipEEPROM

class Fakebus: 
    """crude little mock for I2C bus object"""
    def __init__(self): 
        self.devices = [] 
        self.writes = [] 
        self.reads = [] 
        self.buffer_size = 127 
  
    def attach_device(self, d): 
         self.devices.append(d) 
          
    def write(self, addr, b): 
         self.writes.append((addr, b)) 
          
    def read(self, addr, l): 
         self.reads.append((addr, l)) 
         return tuple([0xAA]*l) 
          

class TestSum(unittest.TestCase):
    def test_creation(self):
        """Can we create an object with specified parameters?"""
        b = Fakebus()
        e = microchipEEPROM.EEPROMDevice(b, 65536, 128 )
        self.assertEqual(e.capacity, 65536)
        self.assertEqual(e.page_size, 128)
        self.assertEqual(len(e.blocks), 1)
        device = e.blocks[0]
        self.assertEqual(device.address, 0x50)
        self.assertEqual(e.address_bits, 16)

    def test_addressing(self):
        b = Fakebus()
        # Three address pins, bits 0, 1, 2
        for i in range(0,8):
            e = microchipEEPROM.EEPROMDevice(b, 256, 16, bitmask="AAA", slave_address=i)
            device = e.blocks[0]
            self.assertEqual(device.address, 0x50 | i)

        # Two address pins, bits 0 and 1
        e = microchipEEPROM.EEPROMDevice(b, 256, 16, bitmask="0AA", slave_address=0b11)
        device = e.blocks[0]
        self.assertEqual(device.address, 0x50 | 0b011)

        # Two address pins, bits 1 and 2
        e = microchipEEPROM.EEPROMDevice(b, 256, 16, bitmask="AA0", slave_address=0b11)
        device = e.blocks[0]
        self.assertEqual(device.address, 0x50 | 0b110)

        # Try to set a two bit address with only one address bit
        # This should fail
        with self.assertRaises(AssertionError):
            e = microchipEEPROM.EEPROMDevice(b, 256, 16, bitmask="A00", slave_address=0b10)

    def test_block_addressing(self):
        b = Fakebus()
        # Create a 512 byte device with 1 block address bit
        # This will be arranged as two 256 byte devices
        # at device slave addresses 0x50 and 0x51,
        # each addressed by a single word.
        e = microchipEEPROM.EEPROMDevice(b, 512, 16, bitmask="00B")
        self.assertEqual(len(e.blocks), 2)
        addresses = [ x.address for x in e.blocks]
        self.assertTrue(0x50 in addresses)
        self.assertTrue(0x51 in addresses)
        self.assertEqual(e.address_bits, 8)

    def test_reads(self):
        # Set up two block device again
        b = Fakebus()
        e = microchipEEPROM.EEPROMDevice(b, 512, 16, bitmask="00B")

        # Perform a read that spans multiple blocks
        # By reading all 256 bytes of block 0
        # and one byte of block 1
        rb = e.read_bytes(0,256)

        # Ensure read returns expected number of bytes
        self.assertEqual(len(rb), 257)

        # Ensure read addresses were correctly translated
        read_bytes={}
        # Read occur in chunks determined the bus object's buffer size
        # So we need to sum up the total bytes read from each device
        for (device, bytes_read) in b.reads:
            read_bytes[device] = read_bytes.get(device, 0) + bytes_read

        # Block 0 (device slave address 0x50)
        self.assertEqual(read_bytes[0x50], 256)

        # Block 1 device slave address 0x51)
        self.assertEqual(read_bytes[0x51], 1)

    def test_address_encoding(self):
        b = Fakebus()

        # A 256 byte ROM requires a single byte to address words
        e = microchipEEPROM.EEPROMDevice(b, 256, 16)
        self.assertEqual(e.encode_address(0xFF), b'\xff')

        # A >256 byte ROM with no block addressing requires
        # two bytes to address words
        e = microchipEEPROM.EEPROMDevice(b, 1024, 16)
        self.assertEqual(e.encode_address(0xFF), b'\x00\xff')

    def test_writes(self):
        # Set up two block device again
        b = Fakebus()
        e = microchipEEPROM.EEPROMDevice(b, 512, 16, bitmask="00B")

        # Perform a read that spans multiple blocks
        # By reading all 256 bytes of block 0
        # and one byte of block 1
        data = bytes([0xAA] * 257)
        e.write_bytes(0, data, attempts = 0)

        # Ensure written addresses were correctly translated
        written_bytes={}
        # Read occur in chunks determined the bus object's buffer size
        # So we need to sum up the total bytes read from each device
        for (device, bytes_written) in b.writes:
            addr = bytes_written[0]
            bytes_written = bytes_written[1:]
            written_bytes[device] = written_bytes.get(device, 0) + len(bytes_written)

        # Block 0 (device slave address 0x50)
        self.assertEqual(written_bytes[0x50], 256)

        # Block 1 device slave address 0x51)
        self.assertEqual(written_bytes[0x51], 1)



if __name__ == '__main__':
    unittest.main()
