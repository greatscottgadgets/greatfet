#
# This file is part of GreatFET
#

class GreatFETPeripheral(object):
    """
    Generic base class for GreatFET peripherals.
    """


    def __init__(self, device):
        """ Default peripheral initializer -- just stores a reference to the relevant GreatFET. """

        self.device = device
