#
# Debug definitions for the LPC43xx.
#

from cmsis_svd.parser import SVDParser
from .svd import DebugTarget, DebugPeripheral

class VirtualLPC43xxTarget(DebugTarget):
    """ Debug target for working with LPC43xx peripherals. """

    def __init__(self, device):
        """
        Create a new instance of LPC43xxDebugTarget. Note that this class is empty; and is meant to be
        created using the LPC43xxDebugTarget virtual method, below.
        """

        # Store a reference to our device, for peek and poke.
        self.__dict__['device'] = device
        self.__dict__['api']    = device.apis.debug


    def peek(self, address):
        return self.api.peek(address)


    def poke(self, address, value):
        self.api.poke(address, value)



def LPC43xxTarget(device):
    """ Factory function that creates a low-level LPC43xx target. """

    from .. import find_greatfet_asset

    svd_file = find_greatfet_asset('LPC43xx_43Sxx.svd')
    svd = SVDParser.for_xml_file(svd_file)

    return VirtualLPC43xxTarget.from_svd(svd.get_device(), device)
