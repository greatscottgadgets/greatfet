#
# Copyright (c) 2016 Dominic Spill <dominicgs@gmail.com>
# Copyright (c) 2017 Mike Naberezny <mike@naberezny.com>
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

def _gpio(port, pin):
    """Pack an LPC GPIO port and pin into a 16-bit number for USB transfers"""
    return (port << 8) + pin;

class J1(object):
    """GreatFET One header J1 pins and their LPC GPIO equivalents"""
    # P1 = GND
    # P2 = VCC
    P3 = _gpio(5, 13)
    P4 = _gpio(0, 0)
    P5 = _gpio(5, 14)
    P6 = _gpio(0, 1)
    P7 = _gpio(0, 4)
    P8 = _gpio(2, 9)
    P9 = _gpio(2, 10)
    P10 = _gpio(0, 8)
    # P11 = CLK0
    P12 = _gpio(0, 9)
    P13 = _gpio(1, 8)
    P14 = _gpio(2, 11)
    P15 = _gpio(1, 0)
    P16 = _gpio(1, 9)
    P17 = _gpio(1, 2)
    P18 = _gpio(1, 1)
    P19 = _gpio(2, 12)
    P20 = _gpio(1, 3)
    P21 = _gpio(1, 5)
    P22 = _gpio(1, 4)
    P23 = _gpio(2, 14)
    P24 = _gpio(2, 13)
    P25 = _gpio(1, 7)
    P26 = _gpio(1, 6)
    P27 = _gpio(2, 15)
    P28 = _gpio(0, 2)
    P29 = _gpio(2, 7)
    P30 = _gpio(0, 3)
    P31 = _gpio(0, 13)
    P32 = _gpio(0, 12)
    P33 = _gpio(5, 18)
    P34 = _gpio(4, 11)
    P35 = _gpio(5, 0)
    # P36 = P6_0
    P37 = _gpio(0, 15)
    # P38 = P1_19
    P39 = _gpio(0, 11)
    P40 = _gpio(0, 10)

class J2(object):
    """GreatFET One header J2 pins and their LPC GPIO equivalents"""
    # P1 = GND
    # P2 = VBUS
    P3 = _gpio(5, 12)
    P4 = _gpio(2, 0)
    # P5 = ADC0_0
    P6 = _gpio(2, 5)
    P7 = _gpio(2, 4)
    P8 = _gpio(2, 2)
    P9 = _gpio(2, 3)
    P10 = _gpio(2, 6)
    # P11 = P4_7
    # P12 = CLK2
    P13 = _gpio(5, 7)
    P14 = _gpio(0, 7)
    P15 = _gpio(5, 6)
    P16 = _gpio(3, 15)
    # P17 = WAKEUP0
    P18 = _gpio(5, 5)
    P19 = _gpio(5, 4)
    P20 = _gpio(5, 3)
    # P21 = PF_4
    P22 = _gpio(5, 9)
    P23 = _gpio(3, 10)
    P24 = _gpio(5, 8)
    P25 = _gpio(3, 9)
    # P26 = P3_0
    P27 = _gpio(3, 8)
    P28 = _gpio(1, 14)
    P29 = _gpio(5, 16)
    P30 = _gpio(5, 10)
    P31 = _gpio(5, 15)
    # P32 = P3_3
    P33 = _gpio(5, 2)
    P34 = _gpio(0, 5)
    P35 = _gpio(5, 1)
    P36 = _gpio(3, 2)
    P37 = _gpio(1, 15)
    P38 = _gpio(0, 6)
    # P39 = I2C0_SDA
    # P40 = I2C0_SDL

class J7(object):
    """GreatFET One header J7 pins and their LPC GPIO equivalents"""
    # P1 = GND
    P2 = _gpio(3, 3)
    P3 = _gpio(3, 4)
    # P4 = ADC0_5
    # P5 = ADC0_2
    P6 = _gpio(1, 10)
    P7 = _gpio(1, 12)
    P8 = _gpio(1, 13)
    # P9 = RTC_ALARM
    # P10 = GND
    # P11 = RESET
    # P12 = VBAT
    P13 = _gpio(1, 11)
    P14 = _gpio(0, 14)
    P15 = _gpio(3, 6)
    P16 = _gpio(3, 5)
    P17 = _gpio(3, 1)
    P18 = _gpio(3, 0)
    # P19 = GND
    # P20 = VCC
