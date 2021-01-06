#
# This file is part of GreatFET
#
"""
Chipcon programmer for devices supporting the CC1110/CC2430/CC2510
Debug and Programming Interface Specification as defined in SWRA124.
"""

import time

from enum import IntFlag

from ..programmer import GreatFETProgrammer


FLASH_PAGE_SIZE      = 1024 # 1KB
FLASH_WORD_SIZE      = 2    # 2 bytes
WORDS_PER_FLASH_PAGE = FLASH_PAGE_SIZE // FLASH_WORD_SIZE


def create_programmer(board, *args, **kwargs):
    """ Creates a representative programmer for this module. """

    return ChipconProgrammer(board, *args, **kwargs)



class ChipconProgrammer(GreatFETProgrammer):
    """ Class used to program chipcon devices."""


    def _split_linear_address(self, linear_address):
        """ Takes a linear address and returns a bank index, and the page address as separate bytes.

        Parameters:
            linear_address -- The address to convert.

            Returns -- A tuple of the three values: (bank, page_address_high, page_address_low)
        """

        # Each bank is a 15-bit address space.
        # The bank itself is 8 bits.
        bank = linear_address >> 15

        if bank > 0xFF:
            raise ValueError("Linear address is too big: must fit in 8 bits!")

        page_address = linear_address & 0x7FFF

        page_address_high = page_address >> 8
        page_address_low = page_address & 0x00FF

        return (bank, page_address_high, page_address_low)


    def __init__(self, board):
        self.board = board
        self.api = self.board.apis.swra124

        self.api.setup()


    def debug_init(self):
        """ Begin debugging. Should be called before any debug commands. """

        self.api.debug_init()


    def get_chip_id(self):
        """ Returns 16-bit chip ID and version number. """

        return(self.api.get_chip_id())


    def read_status(self):
        """ Reads status byte, returns a DebugStatus enum. """
        return(DebugStatus(self.api.read_status()))


    def resume(self):
        """ Resume CPU operation from a halted state. """

        self.api.resume()


    def run_instruction(self, *instruction):
        """ Executes a single instruction on the target without incrementing the program counter.

        Parameters:
            instruction -- A bytes instance or a list of integers representing the opcodes to execute.
        """

        if not isinstance(instruction[0], bytes):
            instruction = bytes(instruction)
        else:
            instruction = instruction[0]

        return self.api.debug_instr(instruction)


    def read_code_memory(self, linear_address, length):
        """ Reads a section of code memory.

        Parameters:
            linear_address -- The address in code memory to read from. It will be converted into an 8-bit bank
                index and a 15-bit address within that bank.
            length -- The amount of data to read.
        """

        # Assembly opcodes used as recommended in SWRA124.

        output = bytearray()

        bank, page_address_high, page_address_low = self._split_linear_address(linear_address)

        self.run_instruction(0x75, 0xc7, (bank * 16) + 1)               # MOV MEMCTR, (bank * 16) + 1
        self.run_instruction(0x90, page_address_high, page_address_low) # MOV DPTR, address

        for n in range(length):
            self.run_instruction(0xE4)                # CLR A
            output.append(self.run_instruction(0x93)) # MOVC A, @A+DPTR
            self.run_instruction(0xA3)                # INC DPTR

        return output


    def read_xdata_memory(self, linear_address, length):
        """ Reads a section of XDATA memory.

        Parameters:
            linear_address -- The address in code memory to read from.
            length -- The amount of data to read.
        """

        # Assembly opcodes used as recommended in SWRA124.

        output = bytearray()

        address_high = linear_address >> 8
        address_low  = linear_address & 0xFF

        self.run_instruction(0x90, address_high, address_low) # MOV DPTR, address

        for n in range(length):
            output.append(self.run_instruction(0xE0)) # MOVX A, @DPTR; (output[n] = A)
            self.run_instruction(0xA3) # INC DPTR

        return output


    def write_xdata_memory(self, linear_address, input_data):
        """ Writes data from input_data into XDATA memory, byte by byte.

        Parameters:
            linear_address -- The address in XDATA memory to write to.
            input_data -- The data to be written into XDATA memory
        """

        # Assembly opcodes used as recommended in SWRA124.add()

        address_high = linear_address >> 8
        address_low  = linear_address & 0xFF

        self.run_instruction(0x90, address_high, address_low) # MOV DPTR, address

        for byte in input_data:
            self.run_instruction(0x74, byte)    # MOV A, #inputArray[n]
            self.run_instruction(0xF0)          # MOVX @DPTR, A
            self.run_instruction(0xA3)          # INC DPTR


    def set_pc(self, linear_address):
        """ Modifies the program counter value.

        Parameters:
            linear_address -- The address in code memory to resume execution from.
        """

        # Assembly opcodes used as recommended in SWRA124

        address_high = linear_address >> 8
        address_low  = linear_address & 0xFF

        self.run_instruction(0x02, address_high, address_low) # LJMP


    def clock_init(self):
        """ Initializes the 32MHz crystal oscillator."""

        # Assembly opcodes used as recommended in SWRA124

        self.run_instruction(0x75, 0xC6, 0x00) # MOV CLKCON, #00H

        while True:
            sleep_reg = self.run_instruction(0xE5, 0xBE)    # MOV A, SLEEP (sleep_reg = A)
            if not sleep_reg & 0x40:
                break


    def write_flash_page(self, linear_address, input_data, erase_page=True):
        """ Writes a single flash page by loading the image into XDATA memory,
        together with an assembly routine that performs the actual update.
        This is done by using unified mapping.

        Parameters:
            linear_address -- The address in flash memory to read from.
            input_data -- The data to be written into flash memory.
            erase_page -- Boolean value for page erasing.
        """

        # Assembly opcodes used as recommended in SWRA124

        high_byte = WORDS_PER_FLASH_PAGE >> 8
        low_byte  = WORDS_PER_FLASH_PAGE & 0xFF

        # Note: The marked section, which performs page erasure,
        # should only be included in the routine when the erase_page_1 = 1.
        # The pseudo-code does not refer to this parameter!
        routine_part1 = [
            0x75, 0xAD, ((linear_address >> 8) // FLASH_WORD_SIZE) & 0x7E,  # MOV FADDRH, #imm
            0x75, 0xAC, 0x00,                                               # MOV FADDRL, #00
        ]
        routine_erase = [
            0x75, 0xAE, 0x01,                                               # marked; MOV FLC, #01H (ERASE)
                                                                            # marked; wait for flash erase to complete
            0xE5, 0xAE,                                                     # marked; erase_wait_loop: MOV A, FLC
            0x20, 0xE7, 0xFB,                                               # marked; JB ACC_BUSY, erase_wait_loop
        ]

        routine_part2 = [                                                   # initialize the data pointer
            0x90, 0xF0, 0x00,                                               # MOV DPTR, #0F000H
                                                                            # outer loops
            0x7F, high_byte,                                                # MOV R7, #imm
            0x7E, low_byte,                                                 # MOV R6, #imm
            0x75, 0xAE, 0x02,                                               # MOV FLC, #02H (WRITE)
                                                                            # inner loops
            0x7D, FLASH_WORD_SIZE,                                          # write_loop:   MOV R5, #imm
            0xE0,                                                           # write_word_loop:  MOVX A, @DPTR
            0xA3,                                                           #                   INC DPTR
            0xF5, 0xAF,                                                     #                   MOV FWDATA, A
            0xDD, 0xFA,                                                     #               DJNZ R5, write_word_loop
                                                                            #               wait for completion
            0xE5, 0xAE,                                                     # w_wait_loop:  MOV A, FLX
            0x20, 0xE6, 0xFB,                                               #               JB ACC_SWBSY, w_wait_loop
            0xDE, 0xF1,                                                     # DJNZ R6, write_loop
            0xDF, 0xEF,                                                     # DJNZ R7, write_loop
                                                                            # done, fake a breakpoint
            0xA5                                                            # DB 0xA5
        ]

        if erase_page:
            routine = routine_part1 + routine_erase + routine_part2
        else:
            routine = routine_part1 + routine_part2

        self.write_xdata_memory(0xF000, input_data[:FLASH_PAGE_SIZE])
        self.write_xdata_memory(0xF000 + FLASH_PAGE_SIZE, routine)
        self.run_instruction(0x75, 0xC7, 0x51)                              # MOV MEMCRT, (bank * 16) + 1
        self.set_pc(0xF000 + FLASH_PAGE_SIZE)
        self.resume()

        while True:
            status = self.read_status()
            if not (status & DebugStatus.CPU_HALTED):
                break


    def read_flash_page(self, linear_address):
        """ Reads from a single flash page.

        Parameters:
            linear_address -- The address in flash memory to read from. It will be converted into an 8-bit bank
                index and a 15-bit address within that bank.
        """

        return self.read_code_memory(linear_address & 0xFFFF, FLASH_PAGE_SIZE)


    def read_flash(self, *, start_address=0, length):
        """ Read a chunk of flash memory.

        Parameters:
            start_address -- The address in flash memory you want to begin reading data from.
            length -- The length (in bytes) of the amount of flash memory that you want to read.
        """
        flash_data = bytearray()
        for i in range(start_address, length, FLASH_PAGE_SIZE):
            flash_data.extend(self.read_flash_page(i))

        return flash_data[:length]


    def mass_erase_flash(self):
        """ Erase the entire flash memory.
        """
        self.run_instruction(0x00)
        self.api.chip_erase()

        while True:
            status = self.read_status()
            if not (status & DebugStatus.CHIP_ERASE_DONE):
                break


    def program_flash(self, image_array, *, erase=True, verify=True, start=0):
        """ Program the entire flash memory.

        Parameters:
            image_array -- The data to be written to the flash.
            erase -- Used to specify whether or not the flash needs to be erased before programming.
            verify -- Used to specify whether or not the data was flashed correctly.
            start -- The address to begin writing data to the flash.
        """

        if erase:
            self.mass_erase_flash()

        data = bytearray(image_array)
        address = start

        while data:
            time.sleep(0.1)
            # Grab a page...
            page = data[:FLASH_PAGE_SIZE]
            del data[:FLASH_PAGE_SIZE]

            # ... and write it to flash.
            self.write_flash_page(address - start, page, False)
            address += FLASH_PAGE_SIZE

        time.sleep(0.1)
        if verify:
            data = self.read_flash(length=len(image_array), start_address=start)
            if data != image_array:
                raise IOError("Flash did not pass verification after programming")



class DebugStatus(IntFlag):
    """ Enumeration for the values returned from the chip status. """

    CHIP_ERASE_DONE     = 0x80
    PCON_IDLE           = 0x40
    CPU_HALTED          = 0x20
    POWER_MODE_0        = 0x10
    HALT_STATUS         = 0x08
    DEBUG_LOCKED        = 0x04
    OSCILLATOR_STABLE   = 0x02
    STACK_OVERFLOW      = 0x01
