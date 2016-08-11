#
# Copyright (c) 2016 Kyle J. Temkin <kyle@ktemkin.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
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
INIT_SPIFLASH = 0
WRITE_SPIFLASH = INIT_SPIFLASH + 1
READ_SPIFLASH = WRITE_SPIFLASH + 1
ERASE_SPIFLASH = READ_SPIFLASH + 1

# Board information API.
READ_BOARD_ID = ERASE_SPIFLASH + 1
READ_VERSION_STRING = READ_BOARD_ID + 1
READ_PARTID_SERIALNO = READ_VERSION_STRING + 1

# Temporary, custom stuffs?
ENABLE_USB1 = READ_PARTID_SERIALNO + 1
LED_TOGGLE = ENABLE_USB1 + 1

REGISTER_GPIO = LED_TOGGLE + 1
WRITE_GPIO = REGISTER_GPIO + 1

SPI_INIT = WRITE_GPIO + 1
SPI_WRITE = SPI_INIT + 1
SPI_READ = SPI_WRITE + 1
SPI_DUMP_FLASH = SPI_READ + 1

I2C_START = SPI_DUMP_FLASH + 1
I2C_STOP = I2C_START + 1
I2C_XFER = I2C_STOP + 1
I2C_RESPONSE = I2C_XFER + 1
