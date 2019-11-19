#
# This file is part of GreatFET
#

from ..interface import GreatFETInterface

class ADC(GreatFETInterface):
    """
    Class representing a GreatFET ADC, which defaults to ADC 0 connected to J2_P5 on Azaela, with 10
    significant bits. """

    PIN_MAPPINGS = {}

    def __init__(self, board, board_pin='J2_P5', adc_num=0, significant_bits=10):

        # Sanity check:
        if adc_num != 0 and adc_num != 1:
            raise ValueError("Specified an unavailable ADC! (Valid values are 0 and 1).")

        self.board = board
        self.api = self.board.apis.adc

        # Get ADC pin number for board pin
        self.adc_number, self.pin_number = self.PIN_MAPPINGS[board_pin.upper()][adc_num]

        if self.pin_number is None:
            raise ValueError("Pin {} cannot connect to ADC {}!".format(board_pin, adc_num))


    @classmethod
    def register_adc(cls, name, pin_list):
        cls.PIN_MAPPINGS[name] = pin_list

    def read_samples(self, sample_count=1):
        """ Read the specified number of samples (default 1) from the ADC and return a tuple of all the sampled values. """
        return self.api.read_samples(self.adc_number, self.pin_number, sample_count)
