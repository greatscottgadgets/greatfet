#
# Copyright (c) 2016 Kyle J. Temkin <kyle@ktemkin.com>
# Copyright 2016 Jared Boone <jared@sharebrained.com>
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

requests = [
    # Internal programming requests.
    'INIT_SPIFLASH',
    'WRITE_SPIFLASH',
    'READ_SPIFLASH',
    'ERASE_SPIFLASH',

    # Board information API.
    'READ_BOARD_ID',
    'READ_VERSION_STRING',
    'READ_PARTID_SERIALNO',

    # Temporary, custom stuffs?
    'ENABLE_USB1',
    'LED_TOGGLE',

    'REGISTER_GPIO',
    'WRITE_GPIO',

    'SPI_INIT',
    'SPI_WRITE',
    'SPI_READ',
    'SPI_DUMP_FLASH',

    'I2C_START',
    'I2C_STOP',
    'I2C_XFER',
    'I2C_RESPONSE',

    'LOGIC_ANALYZER_START',
    'LOGIC_ANALYZER_STOP',

    'RESET',

    'ADC_INIT',
    'ADC_READ',
    'ADC_STREAM',

    'SDIR_START',
    'SDIR_STOP',
    'SDIR_TX',

    'GREATDANCER_CONNECT',
    'GREATDANCER_DISCONNECT',
    'GREATDANCER_BUS_RESET',
    'GREATDANCER_SET_ADDRESS',
    'GREATDANCER_SET_UP_ENDPOINTS',

    'GREATDANCER_GET_STATUS',
    'GREATDANCER_READ_SETUP',
    'GREATDANCER_STALL_ENDPOINT',
    'GREATDANCER_SEND_ON_ENDPOINT',
    'GREATDANCER_CLEAN_UP_TRANSFER',
    'GREATDANCER_START_NONBLOCKING_READ',
    'GREATDANCER_FINISH_NONBLOCKING_READ',
    'GREATDANCER_GET_NONBLOCKING_LENGTH'
]

# Get a reference (as an object) to this module (self)
this_module = globals()

for i in range(len(requests)):
    this_module[requests[i]] = i

