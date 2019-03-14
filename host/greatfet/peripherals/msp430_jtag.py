#
# This file is part of GreatFET
#

from itertools import chain

from ..peripheral import GreatFETPeripheral

class JTAG(GreatFETPeripheral):
    CoreID=0
    DeviceID=0
    JTAGID=0

    def __init__(self, board):
        """
            Initialize a new MSP430 JTAG instance.

            Args:
                board -- The GreatFET board connected to the target.
        """
        self.board = board

    def setup(self):
        """Initialise the JTAG hardware and target device."""
        self.board.apis.jtag.setup()
        
    def stop(self):
        """Stop debugging."""
        self.board.apis.jtag.stop()
    
    def get_deviceid(self, chip):
        """
            Get the Device ID.

            Args:
                chip -- Target chip number.
        """
        device_id = self.board.apis.jtag.get_device_id()
        return device_id
 
    def shift_ir_8(self, instruction):
        """Shift the 8-bit Instruction Register."""
        return self.board.apis.jtag.ir_shift(data)
    
    def shift_dr_16(self, data):
        """Shift the 16-bit Data Register."""
        return self.board.apis.jtag.dr_shift(data)


class JTAG_MSP430(JTAG):
    MSP430_ident = 0x00

    def __init__(self, board):
        """
            Initialize a new MSP430 JTAG instance.

            Args:
                board -- The GreatFET board connected to the target.
        """
        JTAG.__init__(self, board)
    
    def start(self):
        """Initialise the JTAG hardware and target device."""
        self.JTAGID = self.board.apis.jtag_msp430.start()
        if(not (self.JTAGID==0x89 or self.JTAGID==0x91)):
            #Try once more
            self.JTAGID = self.board.apis.jtag_msp430.start()
        
        if self.JTAGID in (0x89, 0x91):
            self.halt_cpu()
        return self.JTAGID
    
    def stop(self):
        """Stop debugging."""
        self.board.apis.jtag_msp430.stop()
    
    def peek(self, address, length=2):
        """
            Read a word at an address.

            Args:
                address -- The memory address to read from the target.
                length -- Number of bytes to read.
        """
        return self.board.apis.jtag_msp430.read_mem(address, length)
    
    def peek_block(self, address, block_size=0x400):
        """Grab a large block from an SPI Flash ROM."""
        data = self.peek(address, block_size)
        byte_pairs = [(x&0xFF, (x&0xFF00)>>8) for x in data]
        data_bytes = bytes(chain.from_iterable(byte_pairs))
        return data_bytes
    
    def poke(self, address, value):
        """
            Write the contents of memory at an address.

            Args:
                address -- The memory address to be written.
                value -- Value to write to location.
        """
        return self.board.apis.jtag_msp430.write_mem(address, value)
    
    def poke_flash_block(self, address, data):
        """
            Write the contents of flash memory at an address.

            Args:
                address -- The memory address to be written.
                data -- Words to write to flash
        """
        value = self.board.apis.jtag_msp430.write_flash(address, data, timeout=30000)
        return value
    
    def poke_flash(self, address, value):
        """
            Write a single word to flash at an address.

            Args:
                address -- The memory address to be written.
                value -- Valuse to write to location
        """
        value = self.poke_flash_block(address, (value,))
        return value
    
    def set_secret(self,value):
        """Set a secret word for later retreival.  Used by glitcher."""
        self.poke_flash(0xFFFE, value)

    def get_secret(self):
        """Get a secret word.  Used by glitcher."""
        return self.peek(0xfffe)
    
    def halt_cpu(self):
        """Halt the CPU."""
        self.board.apis.jtag_msp430.halt_cpu()
    
    def release_cpu(self):
        """Resume the CPU."""
        self.board.apis.jtag_msp430.relase_cpu()
    
    def set_instruction_fetch(self):
        """Set the instruction fetch mode."""
        self.board.apis.jtag_msp430.set_instruction_fetch()
    
    def ident(self):
        """Fetch self-identification word from 0x0FF0 as big endian."""
        if self.MSP430_ident == 0x00:
            if self.JTAGID == 0x89:
                i=self.peek(0x0ff0)
                
            if self.JTAGID == 0x91 :
                i=self.peek(0x1A04)
            if len(i) >= 1:
                self.MSP430_ident = ((i[0]&0xFF00)>>8)+((i[0]&0xFF)<<8)
        return self.MSP430_ident

    devices = {
        #MSP430F2xx
        0xf227: "MSP430F22xx",
        0xf213: "MSP430F21x1",
        0xf249: "MSP430F24x",
        0xf26f: "MSP430F261x",
        0xf237: "MSP430F23x0",
        0xf201: "MSP430F201x",
        #Are G's and F's distinct?
        0x2553: "MSP430G2553",
        
        #MSP430F1xx
        0xf16c: "MSP430F161x",
        0xf149: "MSP430F13x",  #or f14x(1)
        0xf112: "MSP430F11x",  #or f11x1
        0xf143: "MSP430F14x",
        0xf112: "MSP430F11x",  #or F11x1A
        0xf123: "MSP430F1xx",  #or F123x
        0x1132: "MSP430F1122", #or F1132
        0x1232: "MSP430F1222", #or F1232
        0xf169: "MSP430F16x",
        
        #MSP430F4xx
        0xF449: "MSP430F43x", #or F44x
        0xF427: "MSP430FE42x", #or FW42x, F415, F417
        0xF439: "MSP430FG43x",
        0xf46f: "MSP430FG46xx", #or F471xx
        0xF413: "MSP430F413", #or maybe others.
        }
    
    def ident_string(self):
        """Grab model string."""
        return self.devices.get(self.ident())

    def erase_flash(self):
        """Erase MSP430 flash memory."""
        self.board.apis.jtag_msp430.erase_flash()
    
    def erase_info(self):
        """Erase MSP430 info flash."""
        self.board.apis.jtag_msp430.erase_info()

    def set_pc(self, pc):
        """Set the program counter."""
        self.board.apis.jtag_msp430.set_pc(value)
    
    def set_reg(self,reg,val):
        """Set a register."""
        self.board.apis.jtag_msp430.set_reg(reg, value)
    
    def get_reg(self,reg):
        """Get a register."""
        return self.board.apis.jtag_msp430.get_reg(reg)

    def run(self):
        """Reset the MSP430 to run on its own."""
        self.board.apis.jtag_msp430.release_cpu()
    
    def dump_bsl(self):
        self.dump_memory(0xC00, 0xfff)
    
    def dump_all_memory(self):
        self.dump_memory(0x200, 0xffff)
    
    def dump_memory(self, begin, end):
        i=begin
        while i<end:
            print("%04x %04x" % (i, self.peek(i)))
            i+=2
