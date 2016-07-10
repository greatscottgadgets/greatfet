#
# Copyright 2016 Kyle J. Temkin <kyle@ktemkin.com>
#
# This file is part of GreatFET.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from ..board import GreatFETBoard
from ..peripherals.onboard_spi_flash import OnboardSPIFlash

class GreatFETAzalea(GreatFETBoard):
    """ Class representing GreatFET Azalea base-boards. """

    # Currently, all GreatFET Azalea boards have an ID of zero.
    HANDLED_BOARD_IDS = [0]
    BOARD_NAME = "GreatFET Azalea"


    def __init__(self, **device_identifiers):
        """ Initialize a new GreatFET Azalea connection. """

        # Set up the core connection.
        super(GreatFETAzalea, self).__init__(**device_identifiers)

        # Initialize the fixed peripherals that come on the board.
        # TODO: Use a self.add_peripheral mechanism, so peripherals can
        # be dynamically listed?
        self.onboard_flash = OnboardSPIFlash(self)
