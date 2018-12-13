#
# This file is part of GreatFET
#

from ..board import GreatFETBoard

from ..peripherals.firmware import DeviceFirmwareManager


class GreatFETFlashStub(GreatFETBoard):
    """ Class representing any boards in Flash Stub mode. """

    # Currently, all GreatFET One boards have an ID of zero.
    HANDLED_BOARD_IDS = [10000]
    BOARD_NAME = "libgreat compatible board in flash-stub mode"

    def initialize_apis(self):
        """ Initialize the flash stub's supported APIs. """

        # Set up the core connection.
        super(GreatFETFlashStub, self).initialize_apis()

        # The flash stub only supports an onboard flash.
        if self.supports_api('firmware'):
            self.onboard_flash = DeviceFirmwareManager(self)

