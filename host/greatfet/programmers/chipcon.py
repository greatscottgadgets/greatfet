#
# This file is part of GreatFET
#

from ..programmer import GreatFETProgrammer


def create_programmer(board, *args, **kwargs):
    """ Creates a representative programmer for this module. """

    return ChipconProgrammer(board, *args, **kwargs)


class ChipconProgrammer(GreatFETProgrammer):
    """
    Class representing a GreatFET TODO: name it
    """


    def __init__(self, board):
        self.board = board
        self.api = self.board.apis.swra124

        self.api.setup()


    def debug_init(self):
        self.api.debug_init()


    def get_chip_id(self):
        return(self.api.get_chip_id())


    def read_status(self):
        return(self.api.read_status())


    def run_instruction(self, *instruction):
        """ Executes a single instruction on the target without
        incrementing the program counter.
        Paramters:
            instruction -- A bytes instance or a list of integers representing the opcodes to execute.
        """

        if not isinstance(instruction[0], bytes):
            instruction = bytes(*instruction)
        else:
            instruction = instruction[0]

        return self.api.debug_instr(instruction)


    def read_code_memory(self, linear_address, length):
        """ Reads a section of code memory. """

        # Assembly opcodes used as recommended in SWRA124.

        output = bytearray()

        bank = linear_address >> 15
        page_address = linear_address & 0x7FFF

        page_address_high = page_address >> 8
        page_address_low  = page_address & 0xFF

        self.run_instruction(0x75, 0xc7, (bank * 16) + 1)               # MOV MEMCTR, (bank * 16) + 1
        self.run_instruction(0x90, page_address_high, page_address_low) # MOV DPTR, address

        for n in range(length):
            self.run_instruction(0xE4)                # CLR A
            output.append(self.run_instruction(0x93)) # MOVC A, @A+DPTR
            self.run_instruction(0xA3)                # INC DPTR

        return output


