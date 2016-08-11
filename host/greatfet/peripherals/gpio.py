#
# Copyright (c) 2016 Dominic Spill <dominicgs@gmail.com>
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
GPIO pins
"""

class J1(object):
    # GND
    # VCC
    P3 = 0x050d
    P4 = 0x0000
    P5 = 0x050e
    P6 = 0x0001
    P7 = 0x0004
    P8 = 0x0209
    P9 = 0x020a
    P10 = 0x0008
    # CLK0
    P12 = 0x0009
    P13 = 0x0108
    P14 = 0x020b
    P15 = 0x0100
    P16 = 0x0109
    P17 = 0x0102
    P18 = 0x0101
    P19 = 0x020c
    P20 = 0x0103
    P21 = 0x0105
    P22 = 0x0104
    P23 = 0x020e
    P24 = 0x020d
    P25 = 0x0107
    P26 = 0x0106
    P27 = 0x020f
    P28 = 0x0002
    P29 = 0x0207
    P30 = 0x0003
    P31 = 0x000d
    P32 = 0x000c
    P33 = 0x0512
    P34 = 0x040b
    P35 = 0x0500
    # P6_0
    P37 = 0x000f
    # P1_19
    P39 = 0x000b
    P40 = 0x000a

class J2(object):
    # GND
    # VBUS
    P3 = 0x050c
    P4 = 0x0200
    # ADC0_0
    P6 = 0x0205
    P7 = 0x0204
    P8 = 0x0202
    P9 = 0x0203
    P10 = 0x0206
    # P4_7
    # CLK2
    P13 = 0x0507
    P14 = 0x0007
    P15 = 0x0506
    P16 = 0x030f
    # WAKEUP0
    P18 = 0x0505
    P19 = 0x0504
    P20 = 0x0503
    # PF_4
    P22 = 0x0509
    P23 = 0x030a
    P24 = 0x0508
    P25 = 0x0309
    # P3_0
    P27 = 0x0308
    P28 = 0x010e
    P29 = 0x0510
    P30 = 0x050a
    P31 = 0x050f
    # P3_3
    P33 = 0x0502
    P34 = 0x0005
    P35 = 0x0501
    P36 = 0x0302
    P37 = 0x010f
    P38 = 0x0006
    # I2C0_SDA
    # I2C0_SDL

class J7(object):
    # GND
    P2 = 0x0303
    P3 = 0x0304
    # ADC0_5
    # ADC0_2
    P6 = 0x010a
    P7 = 0x010c
    P8 = 0x010d
    # RTC_ALARM
    # GND
    # RESET#
    # VBAT
    P13 = 0x010b
    P14 = 0x000e
    P15 = 0x0306
    P16 = 0x0305
    P17 = 0x0301
    P18 = 0x0300
    # GND
    # VCC
