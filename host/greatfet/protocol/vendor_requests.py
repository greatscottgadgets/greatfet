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
"""
Vendor request numbers-- used for sending USB control requests to
GreatFET boards for non-bulk communication. These are still tenative.

When a final set is created, these should separate global requests (e.g.
requests that every GreatFET base should implement) from per-base requests.

Ideally we'll add an offset (100?) to per-board vendor requests to separate
requests that may differ from base-board to base-board.
"""

# Internal programming requests.
ERASE_SPIFLASH = 0
WRITE_SPIFLASH = 1
READ_SPIFLASH = 2

# Board information API.
READ_BOARD_ID = 3
READ_VERSION_STRING = 4
READ_PARTID_SERIALNO = 5

# Temporary, custom stuffs?
ENABLE_USB1 = 6
LED_TOGGLE = 7
