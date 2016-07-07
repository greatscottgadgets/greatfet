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

from .board import GreatFETBoard

# Ensure that we have access to all GreatFET boards. Normally, we'd avoid
# importing an entire namespace, but in this case, this allows us to ensure
# that all board modules are loaded for autoidentification.
from .boards import *


def GreatFET(**board_identifiers):
    """
            Attempts to create a new instance of GreatFET board (sub)class
            most applicable to the given device. For example, if the attached
            board is a GreatFET Azalea, this will automatically create a
            GreatFETAzalea object.

            Accepts the same arguments as pyusb's usb.find() method, allowing narrowing
            to a more specific GreatFET by e.g. serial number.

            Throws a DeviceNotFoundError if no device is avaiable.
    """
    return GreatFETBoard.autodetect(**board_identifiers)
