#!/usr/bin/env python
#
# This file is part of GreatFET

from __future__ import print_function

import errno
import sys
import time
import argparse
import subprocess
from distutils.util import strtobool
import threading
import traceback

from greatfet import find_greatfet_asset
from greatfet import GreatFET
from greatfet.errors import DeviceNotFoundError
from greatfet.utils import log_silent, log_verbose
from greatfet.interfaces.i2c_device import I2CDevice
from greatfet.interfaces.gpio import GPIO
from greatfet.interfaces.led import LED
from greatfet.protocol import vendor_requests
import greatfet_firmware
import facedancer

class Narcissus:
    # left side is EUT pin
    # right side is what it is connected to on Narcissus/Tester
    # (x, y, z) designates io expander (x), port (y), and pin (z)
    gpio_pins = {
        #"J1_P1"   : GND
        #"J1_P2"   : EUT_VCC connected to ADC
        "J1_P3"    : (1, 4, 7),
        "J1_P4"    : "J2_P24",
        "J1_P5"    : (1, 4, 6),
        "J1_P6"    : "J2_P22",
        "J1_P7"    : "J1_P7",
        "J1_P8"    : (1, 4, 5),
        "J1_P9"    : (1, 4, 4),
        "J1_P10"   : (1, 4, 3),
        #"J2_P11"  : CLK0
        "J1_P12"   : (1, 4, 2),
        "J1_P13"   : (1, 4, 1),
        "J1_P14"   : (1, 4, 0),
        "J1_P15"   : (1, 3, 7),
        "J1_P16"   : (1, 3, 6),
        "J1_P17"   : (1, 3, 5),
        "J1_P18"   : (1, 3, 4),
        "J1_P19"   : (1, 3, 3),
        "J1_P20"   : (1, 3, 2),
        "J1_P21"   : (1, 3, 1),
        "J1_P22"   : (1, 3, 0),
        "J1_P23"   : (1, 2, 7),
        "J1_P24"   : (1, 2, 6),
        "J1_P25"   : (1, 2, 5),
        "J1_P26"   : (1, 2, 4),
        "J1_P27"   : (1, 2, 3),
        "J1_P28"   : (1, 2, 2),
        "J1_P29"   : (1, 2, 1),
        "J1_P30"   : (1, 2, 0),
        "J1_P31"   : (1, 1, 7),
        "J1_P32"   : (1, 1, 6),
        "J1_P33"   : (1, 1, 5),
        "J1_P34"   : (1, 1, 4),
        "J1_P35"   : (1, 1, 3),
        # P36"     : P4_7 I2S0_TX_SCK/I2S1_TX_SCK
        "J1_P37"   : "J1_P37",
        # P38"     : P1_19 SSP1_SCK
        "J1_P39"   : "J1_P40",
        "J1_P40"   : "J1_P39",
        # J2_P1"   : (2, 2, 4) GND on EUT
        # J2_P2"   : EUT_5V connected to ADC
        "J2_P3"    : (2, 2, 6),
        "J2_P4"    : (2, 2, 5),
        #"J2_P5"   : EUT_ADC0_0 connected to ADC
        "J2_P6"    : (2, 2, 7),
        "J2_P7"    : (2, 3, 1),
        "J2_P8"    : (2, 3, 0),
        "J2_P9"    : (2, 3, 3),
        "J2_P10"   : (2, 3, 2),
        #"J2_P11"  : P6_0 I2S1_TX_SCK
        #"J2_P12"  : P4_7 I2S1_RX_SCK
        "J2_P13"   : (2, 3, 5), #DFU
        "J2_P14"   : (2, 3, 4),
        "J2_P15"   : (2, 3, 7),
        "J2_P16"   : (2, 3, 6),
        #"J2_P17"  : (2, 4, 1) WAKEUP0 on EUT
        "J2_P18"   : (2, 4, 0),
        "J2_P19"   : (2, 4, 3),
        "J2_P20"   : (2, 4, 2),
        #"J2_P21"  : PF_4 SSP1_SCK
        "J2_P22"   : "J1_P6",
        "J2_P23"   : (2, 4, 4),
        "J2_P24"   : "J1_P4",
        "J2_P25"   : (2, 4, 5),
        #"J2_P26"  : P3_0 SSP0_SCK
        "J2_P27"   : (2, 4, 6),
        "J2_P28"   : "J2_P23",
        "J2_P29"   : (2, 4, 7),
        "J2_P30"   : "J2_P38",
        "J2_P31"   : (2, 0, 0),
        #"J2_P32"  : P3_3 SSP0_SCK
        "J2_P33"   : (2, 0, 2),
        "J2_P34"   : (2, 0, 1),
        "J2_P35"   : (2, 0, 4),
        "J2_P36"   : (2, 0, 3),
        "J2_P37"   : "J2_P25",
        "J2_P38"   : "J2_P30",
        #"J2_P39"  : "J2_P20" I2C0_SDA on EUT
        #"J2_P40"  : "J2_P19" I2C0_SDL on EUT
        #"J7_P1"   : (1, 0, 1) GND on EUT
        "J7_P2"    : (1, 0, 0),
        "J7_P3"    : (2, 2, 3),
        #"J7_P4"   : EUT_ADC0_5 connected to ADC
        #"J7_P5"   : EUT_ADC0_2 connected to ADC
        "J7_P6"    : (2, 2, 2),
        "J7_P7"    : (2, 2, 1),
        "J7_P8"    : (2, 2, 0),
        #"J7_P9"   : (2, 1, 7) RTC_ALARM on EUT
        #"J7_P10"  : (2, 1, 6) RTCX1 on EUT
        #"J7_P11"  : "J1_P31" RESET on EUT
        #"J7_P12"  : (2, 1, 5) VBAT on EUT
        "J7_P13"   : (2, 1, 4),
        "J7_P14"   : (2, 1, 3),
        "J7_P15"   : (2, 1, 2),
        "J7_P16"   : (2, 1, 1),
        "J7_P17"   : (2, 1, 0),
        "J7_P18"   : (2, 0, 7),
        #"J7_P19"  : (2, 0, 6) GND on EUT
        #"J7_P20"  : (2, 0, 5) VCC on EUT
    }

    other_pins = {
        "GND_J2_P1"       : (2, 2, 4), #GND
        "WAKEUP0_J2_P17"  : (2, 4, 1),
        "I2C0_SDA_J2_P39" : "J2_P20",
        "I2C0_SDL_J2_P40" : "J2_P19",
        "GND_J7_P1"       : (1, 0, 1), #GND
        "RTC_ALARM_J7_P9" : (2, 1, 7),
        "RTCX1_J7_P10"    : (2, 1, 6), #GND
        "RESET_J7_P11"    : "J1_P31",
        "VBAT_J7_P12"     : (2, 1, 5),
        "GND_J7_P19"      : (2, 0, 6), #GND
        "VCC_J7_P20"      : (2, 0, 5), #VCC
        "USB1_EN"         : "J1_P35",
        "TDO"             : "J1_P25",
        "TCK"             : "J1_P27",
        "DBGEN"           : "J1_P21",
        "U4_S0"           : "J2_P7",
        "U4_S1"           : "J2_P4",
        "U4_S2"           : "J2_P3",
        "U4_E"            : "J2_P6",
        "5V_EN"           : "J2_P8"
    }

    u1_pin_connections = {
        "J1_P1"  : "GND",
        "J1_P2"  : "VCC",
        "J1_P3"  : "U1 pin 33",
        "J1_P4"  : "U1 pin 32",
        "J1_P5"  : "U1 pin 35",
        "J1_P6"  : "U1 pin 34",
        "J1_P7"  : "U1 pin 38",
        "J1_P8"  : "U1 pin 37",
        "J1_P9"  : "U1 pin 39",
        "J1_P10" : "U1 pin 42",
        "J1_P11" : "U1 pin 45",
        "J1_P12" : "U1 pin 43",
        "J1_P13" : "U1 pin 48",
        "J1_P14" : "U1 pin 46",
        "J1_P15" : "U1 pin 50",
        "J1_P16" : "U1 pin 49",
        "J1_P17" : "U1 pin 52",
        "J1_P18" : "U1 pin 51",
        "J1_P19" : "U1 pin 54",
        "J1_P20" : "U1 pin 53",
        "J1_P21" : "U1 pin 56",
        "J1_P22" : "U1 pin 55",
        "J1_P23" : "U1 pin 58",
        "J1_P24" : "U1 pin 57",
        "J1_P25" : "U1 pin 61",
        "J1_P26" : "U1 pin 60",
        "J1_P27" : "U1 pin 63",
        "J1_P28" : "U1 pin 62",
        "J1_P29" : "U1 pin 65",
        "J1_P30" : "U1 pin 64",
        "J1_P31" : "U1 pin 67",
        "J1_P32" : "U1 pin 66",
        "J1_P33" : "U1 pin 69",
        "J1_P34" : "U1 pin 72",
        "J1_P35" : "U1 pin 75",
        "J1_P36" : "U1 pin 73",
        "J1_P37" : "U1 pin 70",
        "J1_P38" : "U1 pin 68",
        "J1_P39" : "U1 pin 47",
        "J1_P40" : "U1 pin 44",
        "J2_P1"  : "GND",
        "J2_P2"  : "5V",
        "J2_P3"  : "U1 pin 15",
        "J2_P4"  : "U1 pin 1",
        "J2_P5"  : "U1 pin 6",
        "J2_P6"  : "U1 pin 10",
        "J2_P7"  : "U1 pin 9",
        "J2_P8"  : "U1 pin 8",
        "J2_P9"  : "U1 pin 7",
        "J2_P10" : "U1 pin 11",
        "J2_P11" : "U1 pin 14",
        "J2_P12" : "U1 pin 99",
        "J2_P13" : "U1 pin 98",
        "J2_P14" : "U1 pin 96",
        "J2_P15" : "U1 pin 95",
        "J2_P16" : "U1 pin 140",
        "J2_P17" : "U1 pin 130",
        "J2_P18" : "U1 pin 91",
        "J2_P19" : "U1 pin 88",
        "J2_P20" : "U1 pin 87",
        "J2_P21" : "U1 pin 120",
        "J2_P22" : "U1 pin 116",
        "J2_P23" : "U1 pin 115",
        "J2_P24" : "U1 pin 114",
        "J2_P25" : "U1 pin 113",
        "J2_P26" : "U1 pin 112",
        "J2_P27" : "U1 pin 110",
        "J2_P28" : "U1 pin 119",
        "J2_P29" : "U1 pin 86",
        "J2_P30" : "U1 pin 123",
        "J2_P31" : "U1 pin 85",
        "J2_P32" : "U1 pin 118",
        "J2_P33" : "U1 pin 84",
        "J2_P34" : "U1 pin 83",
        "J2_P35" : "U1 pin 81",
        "J2_P36" : "U1 pin 79",
        "J2_P37" : "U1 pin 121",
        "J2_P38" : "U1 pin 122",
        "J2_P39" : "U1 pin 93",
        "J2_P40" : "U1 pin 92",
        "J7_P1"  : "GND",
        "J7_P2"  : "U1 pin 80",
        "J7_P3"  : "U1 pin 82",
        "J7_P4"  : "U1 pin 144",
        "J7_P5"  : "U1 pin 143",
        "J7_P6"  : "U1 pin 102",
        "J7_P7"  : "U1 pin 106",
        "J7_P8"  : "U1 pin 108",
        "J7_P9"  : "U1 pin 129",
        "J7_P10" : "GND",
        "J7_P11" : "U1 pin 128",
        "J7_P12" : "U1 pin 127",
        "J7_P13" : "U1 pin 105",
        "J7_P14" : "U1 pin 104",
        "J7_P15" : "U1 pin 100",
        "J7_P16" : "U1 pin 97",
        "J7_P17" : "U1 pin 78",
        "J7_P18" : "U1 pin 74",
        "J7_P19" : "GND",
        "J7_P20" : "VCC"
    }

    # how analog signals are connected to the multiplexer (U4)
    analog_signals = {
        "VBUS_BYPASS" : 0,
        "USB0_VBUS"   : 1,
        "USB1_VBUS"   : 2,
        "EUT_VCC"     : 3,
        "EUT_5V"      : 4,
        "EUT_ADC0_2"  : 5,
        "EUT_ADC0_0"  : 6,
        "EUT_ADC0_5"  : 7
    }

    voltage_divider = {
        "VBUS_BYPASS" : 2,
        "USB0_VBUS"   : 2,
        "USB1_VBUS"   : 2,
        "EUT_VCC"     : 2,
        "EUT_5V"      : 2,
        "EUT_ADC0_2"  : 1,
        "EUT_ADC0_0"  : 1,
        "EUT_ADC0_5"  : 1
    }

    logfile = None

    # tester is the GreatFET One for which the Narcissus is a neighbor
    tester = None

    # eut (equipment under test) is the GreatFET One being tested
    eut = None

    # I2C I/O expanders on Narcissus
    u1 = None
    u2 = None

    eut_pins = {}
    tester_pins = {}

    def ask_user(self, question):
        self.print('%s [y/n]' % question)
        while True:
            try:
                return strtobool(input())
            except ValueError:
                self.print('Please respond with \'y\' or \'n\'.')

    def prompt_for_id(self):
        self.print('Type or scan unique ID for EUT (or press Enter for no ID):')
        self.log('Unique ID: ' + input())

    def fail(self, message):
        self.print(message)
        sys.exit()

    def read_adc(self, device):
        data = device.adc.read_samples(1)
        result = data[0]
        return result

    # configure analog multiplexer
    def select_analog_signal(self, signal):
        self.tester_pins[self.other_pins["U4_E"]].write(1)
        self.tester_pins[self.other_pins["U4_S0"]].write(self.analog_signals[signal] & 0x1)
        self.tester_pins[self.other_pins["U4_S1"]].write((self.analog_signals[signal] >> 1) & 0x1)
        self.tester_pins[self.other_pins["U4_S2"]].write((self.analog_signals[signal] >> 2) & 0x1)
        self.tester_pins[self.other_pins["U4_E"]].write(0)

    def read_analog_voltage(self, signal):
        self.select_analog_signal(signal)
        voltage = self.read_adc(self.tester) * 3.3 * self.voltage_divider[signal] / 1024
        return voltage

    def report_all_analog_voltages(self):
        for signal in sorted(self.analog_signals.keys()):
            self.print("%12s %.3f" % (signal, self.read_analog_voltage(signal)))
        self.print()

    def read_io_expanders(self, expanders):
        state = []
        for expander in expanders:
            state.extend(expander.read(5))
        return state

    def read_io_expander_pin(self, expander, port, pin):
        return (self.read_io_expanders([expander])[port] >> pin) & 1

    def check_gpio_pin(self, name):
        self.tester_pins[name].set_direction(self.tester.gpio.DIRECTION_IN)
        return self.tester_pins[name].read()

    def setup_eut_pins(self):
        for eut_pin_name in list(self.gpio_pins):
            self.eut_pins[eut_pin_name] = self.eut.gpio.get_pin(eut_pin_name)

    def setup_tester_pins(self):
        for eut_pin_name in list(self.gpio_pins):
            if not isinstance(self.gpio_pins[eut_pin_name], tuple):
                self.tester_pins[self.gpio_pins[eut_pin_name]] = self.tester.gpio.get_pin(self.gpio_pins[eut_pin_name])
        for signal in list(self.other_pins):
            if not isinstance(self.other_pins[signal], tuple):
                self.tester_pins[self.other_pins[signal]] = self.tester.gpio.get_pin(self.other_pins[signal])

    def check_all_gpio_pins(self):
        pins = {}
        expander_state = self.read_io_expanders([self.u1, self.u2])
        for eut_pin_name in list(self.gpio_pins):
            if isinstance(self.gpio_pins[eut_pin_name], tuple):
                (expander, port, pin) = self.gpio_pins[eut_pin_name]
                port += (expander - 1) * 5
                pins[eut_pin_name] = (1 == (expander_state[port] >> pin) & 1)
            else:
                pins[eut_pin_name] = self.check_gpio_pin(self.gpio_pins[eut_pin_name])
        return pins

    def find_tester(self):
        devices = []
        timeout = time.time() + 30
        while len(devices) < 1:
            try:
                devices = GreatFET(find_all=True)
            except:
                self.log(traceback.format_exc())
                self.print("Unexpected error: %s" % sys.exc_info()[0])
                pass
            if time.time() >= timeout:
                self.print('FAIL 10: Tester not found. Connect Tester to Narcissus and connect only Tester to this host with USB. Ensure no EUT is present when connecting Tester to host via USB.')
                sys.exit(errno.ENODEV)
            time.sleep(1)

        if len(devices) > 1:
            self.fail('FAIL 20: More than one GreatFET found. Connect only Tester to this host with USB.')

        # Print the Tester's information...
        self.tester = devices[0]
        self.print("Found Tester {}.".format(self.tester.board_name()))
        self.print("  Board ID: {}".format(self.tester.board_id()))
        self.print("  Firmware version: {}".format(self.tester.firmware_version()))
        self.print("  Part ID: {}".format(self.tester.part_id()))
        self.print("  Serial number: {}".format(self.tester.serial_number()))
        self.print(" ")

        self.setup_tester_pins()
        self.tester_pins[self.other_pins["U4_E"]].write(1)
        self.tester_pins[self.other_pins["U4_E"]].set_direction(self.tester.gpio.DIRECTION_OUT)
        self.tester_pins[self.other_pins["U4_S0"]].set_direction(self.tester.gpio.DIRECTION_OUT)
        self.tester_pins[self.other_pins["U4_S1"]].set_direction(self.tester.gpio.DIRECTION_OUT)
        self.tester_pins[self.other_pins["U4_S2"]].set_direction(self.tester.gpio.DIRECTION_OUT)

    def initialize_jig(self):
        self.u1 = I2CDevice(self.tester.i2c, 0x4c>>1)
        self.u2 = I2CDevice(self.tester.i2c, 0x4e>>1)

        if self.read_io_expanders([self.u1]) == [255, 255, 255, 255, 255]:
            self.fail('FAIL 100: I2C I/O expander U1 not detected. Connect Tester to Narcissus.')
            sys.exit(errno.ENODEV)
        if self.read_io_expanders([self.u2]) == [255, 255, 255, 255, 255]:
            self.fail('FAIL 110: I2C I/O expander U2 not detected. Connect Tester to Narcissus.')
            sys.exit(errno.ENODEV)

    def detect_eut(self):
        self.print('Connect Equipment Under Test (EUT) to spring pins on Narcissus. Do not connect EUT USB cables.')

        eut_detected = False
        while True:
            if self.read_analog_voltage("EUT_VCC") > 0.2:
                eut_detected = True
            # debounce
            time.sleep(0.5)
            if eut_detected:
                if self.read_analog_voltage("EUT_VCC") > 0.2:
                    break

        self.print('Detected EUT.')
        self.report_all_analog_voltages()

        if self.read_analog_voltage("USB0_VBUS") > 4.0:
            self.fail('FAIL 150: USB0 power detected. Unplug USB cable from EUT J3/USB0.')
        if self.read_analog_voltage("USB1_VBUS") > 4.0:
            self.fail('FAIL 160: USB1 power detected. Ensure USB cable from EUT J4/USB1 is connected only to Tester.')
        if self.read_analog_voltage("EUT_VCC") > 2.5:
            self.fail('FAIL 165: EUT target power detected. Disconnect USB cables from EUT.')

    def activate_supply(self):
        self.tester_pins[self.other_pins["5V_EN"]].write(1)
        time.sleep(0.5)
        self.report_all_analog_voltages()

        if self.read_analog_voltage("EUT_5V") < 4.0:
            self.tester_pins[self.other_pins["5V_EN"]].write(0)
            self.fail('FAIL 170: EUT_5V too low. Check for shorts across C3 and C4.')
        if self.read_analog_voltage("EUT_VCC") < 0.6:
            self.tester_pins[self.other_pins["5V_EN"]].write(0)
            self.fail('FAIL 180: EUT_VCC power not detected. Check U3.')
        if self.read_analog_voltage("EUT_VCC") < 3.2:
            self.tester_pins[self.other_pins["5V_EN"]].write(0)
            self.fail('FAIL 190: EUT_VCC voltage too low. Check U3. Check for shorts across C3 and C4 and decoupling capacitors.')

    def test_pull_downs(self):
        if not self.tester_pins[self.other_pins["5V_EN"]].read():
            self.activate_supply()
        # check that all GND pins are low
        if self.read_io_expander_pin(self.u2, 2, 4):
            self.fail('FAIL 200: GND_J2_P1 voltage detected. Check connection of EUT to Narcissus. Check J1 pin 1. Check J2 pin 1.')
        if self.read_io_expander_pin(self.u1, 0, 1):
            self.fail('FAIL 210: GND_J7_P1 voltage detected. Check connection of EUT to Narcissus. Check J7 pin 1.')
        if self.read_io_expander_pin(self.u2, 0, 6):
            self.fail('FAIL 230: GND_J7_P19 voltage detected. Check connection of EUT to Narcissus. Check J7 pin 19.')

        # check that all signals with pull-down resistors are low
        # There is enough reverse leakage through D5 to trigger USB0_SENSE, so
        # we don't check that one.
        if self.check_gpio_pin('J1_P35'):
            self.fail('FAIL 290: USB1_EN voltage detected. Check R16.')
        if self.read_io_expander_pin(self.u1, 0, 5):
            self.fail('FAIL 250: USB1_SENSE voltage detected. Ensure USB cable from EUT J4/USB1 is connected only to Tester. Check U5, R16, R21, R22.')
        if self.read_io_expander_pin(self.u1, 4, 2):
            self.fail('FAIL 260: J1_P12 voltage detected. Check J1 pin 12, R8.')
        if self.read_io_expander_pin(self.u2, 2, 2):
            self.fail('FAIL 270: J7_P6 voltage detected. Check J7 pin 6, R7.')
        if self.read_io_expander_pin(self.u2, 3, 5):
            self.fail('FAIL 280: J2_P13 voltage detected. Check J2 pin 13, R6, SW1.')

    def test_pull_ups(self):
        if not self.tester_pins[self.other_pins["5V_EN"]].read():
            self.activate_supply()
        if not self.read_io_expander_pin(self.u2, 0, 5): #VCC_J7_P20
            self.fail('FAIL 310: EUT target power not detected. Check J1 pin 2, J7 pin 20, U3, C3, C4.')
        # check that all signals with pull-up resistors are high
        # We don't check RESET for now because we are pulling it low.
        #if not self.check_gpio_pin('J1_P31'):
            #self.fail('FAIL 320: RESET voltage not detected. Check J7 pin 11, R5, SW2, U1 pin 128.')
        if not self.read_io_expander_pin(self.u1, 4, 3):
            self.fail('FAIL 330: P1_1 voltage not detected. Check J1 pin 10, R26.')
        if not self.read_io_expander_pin(self.u2, 4, 1):
            self.fail('FAIL 340: WAKEUP0 voltage not detected. Check J2 pin 17, R14.')
        if not self.read_io_expander_pin(self.u1, 0, 3):
            self.fail('FAIL 350: USB1_FAULT voltage not detected. Check R24.')
        if not self.read_io_expander_pin(self.u2, 3, 4):
            self.fail('FAIL 360: P2_7 voltage not detected. Check J2 pin 14, R15.')
        if not self.check_gpio_pin('J2_P19'):
            self.fail('FAIL 370: I2C0_SCL voltage not detected. Check J2 pin 40, R18.')
        if not self.check_gpio_pin('J2_P20'):
            self.fail('FAIL 380: I2C0_SDA voltage not detected. Check J2 pin 39, R17.')
        if not self.check_gpio_pin('J1_P25'):
            self.fail('FAIL 390: TDO voltage not detected. Check R12.')
        if not self.check_gpio_pin('J1_P27'):
            self.fail('FAIL 390: TCK voltage not detected. Check R11.')
        if not self.check_gpio_pin('J1_P21'):
            self.fail('FAIL 400: DBGEN voltage not detected. Check R10.')
        if not self.read_io_expander_pin(self.u1, 0, 2):
            self.fail('FAIL 410: LED1 voltage not detected. Check D1, R1.')
        if not self.read_io_expander_pin(self.u1, 0, 4):
            self.fail('FAIL 420: LED2 voltage not detected. Check D2, R2.')
        if not self.read_io_expander_pin(self.u1, 1, 1):
            self.fail('FAIL 430: LED3 voltage not detected. Check D3, R3.')
        if not self.read_io_expander_pin(self.u1, 1, 2):
            self.fail('FAIL 440: LED4 voltage not detected. Check D4, R4.')

    def test_reset_button(self):
        self.print('Press RESET button (SW2) on EUT.')
        timeout = time.time() + 30
        while time.time() < timeout:
            if not self.check_gpio_pin(self.other_pins['RESET_J7_P11']):
                # debounce
                time.sleep(0.1)
                if (not self.check_gpio_pin(self.other_pins['RESET_J7_P11'])) and (self.read_analog_voltage("EUT_VCC") > 3.2):
                    self.print('Detected RESET button.')
                    self.tester_pins[self.other_pins["RESET_J7_P11"]].write(0)
                    self.tester_pins[self.other_pins["RESET_J7_P11"]].set_direction(self.tester.gpio.DIRECTION_OUT)
                    return
        self.fail('FAIL 600: Timeout while waiting for RESET button. Check SW2.')
        self.tester_pins[self.other_pins["RESET_J7_P11"]].write(0)
        self.tester_pins[self.other_pins["RESET_J7_P11"]].set_direction(self.tester.gpio.DIRECTION_OUT)

    def test_dfu_button(self):
        self.print('Press DFU button (SW1) on EUT.')
        timeout = time.time() + 30
        while time.time() < timeout:
            if self.read_io_expander_pin(self.u2, 3, 5):
                # debounce
                time.sleep(0.1)
                if self.read_io_expander_pin(self.u2, 3, 5) and (self.read_analog_voltage("EUT_VCC") > 3.2):
                    self.print('Detected DFU button.')
                    return
        self.fail('FAIL 610: Timeout while waiting for DFU button. Check SW1.')

    def flash_firmware(self, args, dfu_stub_path):
        self.tester_pins[self.other_pins["5V_EN"]].write(0)
        tester_serial = self.tester.serial_number()
        self.print('Connect USB cable between EUT USB1 and Tester USB1.')
        self.print('Connect EUT to this host with USB cable (J3/USB0) while pressing DFU button (SW1).')
        self.tester_pins[self.other_pins["RESET_J7_P11"]].set_direction(self.tester.gpio.DIRECTION_IN)

        timeout = time.time() + 30
        while self.read_analog_voltage("USB0_VBUS") < 4.4:
            time.sleep(1)
            if time.time() >= timeout:
                self.report_all_analog_voltages()
                self.fail('FAIL 1000: EUT VBUS not detected. Check USB0 cable, J3')

        self.report_all_analog_voltages()
        if self.read_analog_voltage("VBUS_BYPASS") < 4.4:
            self.fail('FAIL 1010: VBUS_BYPASS not detected. Check FB1.')

        timeout = time.time() + 10
        while True:
            try:
                greatfet_firmware.load_dfu_stub(dfu_stub_path)
                time.sleep(1)
                break
            except DeviceNotFoundError:
                if time.time() >= timeout:
                    self.print('FAIL 1020: EUT in DFU mode not found. Ensure that DFU button is pressed while connecting USB0 cable')
                    sys.exit(errno.ENODEV)
            except IOError:
                self.print('DFU IOError')
                pass

        devices = []
        timeout = time.time() + 10
        while len(devices) < 2:
            time.sleep(1)
            try:
                devices = GreatFET(find_all=True)
            except:
                self.log(traceback.format_exc())
                self.print("Unexpected error finding RAM EUT: %s" % sys.exc_info()[0])
                pass
            if time.time() >= timeout:
                self.print('FAIL 1030: EUT running from RAM not found.')
                sys.exit(errno.ENODEV)
            if not devices:
                self.print('FAIL 1040: Tester not found. Connect Tester to Narcissus and connect Tester to this host with USB.')
                sys.exit(errno.ENODEV)
        for device in devices:
            if device.serial_number() != tester_serial:
                eut = device

        # Print the EUT's information...
        self.print("Found EUT {}.".format(eut.board_name()))
        self.print("  Board ID: {}".format(eut.board_id()))
        self.print("  Firmware version: {}".format(eut.firmware_version()))
        self.print("  Part ID: {}".format(eut.part_id()))
        self.print("  Serial number: {}".format(eut.serial_number()))
        self.print(" ")

        self.print('Writing data to SPI flash.')
        try:
            greatfet_firmware.spi_flash_write(eut, args.write, args.address, log_verbose)
        except:
            self.log(traceback.format_exc())
            self.fail('FAIL 1050: Error reading from file or writing to flash.')
        self.print('Write complete.')
        self.print('Resetting EUT.')
        eut.reset(reconnect=False, is_post_firmware_flash=bool(args.write))
        self.print("Reset complete.")
        time.sleep(1)

        devices = []
        timeout = time.time() + 10
        while len(devices) < 2:
            time.sleep(1)
            try:
                devices = GreatFET(find_all=True)
            except:
                self.log(traceback.format_exc())
                self.print("Unexpected error finding flashed EUT: %s" % sys.exc_info()[0])
                pass
            if time.time() >= timeout:
                self.print('FAIL 1060: EUT running from flash not found.')
                sys.exit(errno.ENODEV)
            if not devices:
                self.print('FAIL 1070: Tester not found. Connect Tester to Narcissus and connect Tester to this host with USB.')
                sys.exit(errno.ENODEV)
        for device in devices:
            if device.serial_number() != tester_serial:
                self.eut = device

    def test_gpio(self):
        # set all GPIO pins output high on EUT
        for pin_name in list(self.gpio_pins):
            self.eut_pins[pin_name].set_direction(self.eut.gpio.DIRECTION_OUT)
            self.eut_pins[pin_name].write(1)
        pin_failure = False
        for pin_name, state in self.check_all_gpio_pins().items():
            if not state:
                self.print('Detected no voltage on pin %s (%s).' % (pin_name, self.u1_pin_connections[pin_name]))
                pin_failure = True
        if pin_failure:
                self.fail('FAIL 1100: Detected no voltage on GPIO pin when all pins were driven high. See pins listed above.')

        # set all GPIO pins output low on EUT
        for pin_name in list(self.gpio_pins):
            self.eut_pins[pin_name].write(0)
        for pin_name, state in self.check_all_gpio_pins().items():
            if state:
                self.print('Detected voltage on pin %s (%s).' % (pin_name, self.u1_pin_connections[pin_name]))
                pin_failure = True
        if pin_failure:
                self.fail('FAIL 1110: Detected voltage on GPIO pin when all pins were driven low. See pins listed above.')

        # test GPIO pins one at a time
        for test_pin in list(self.gpio_pins):
            self.eut_pins[test_pin].write(1)
            for pin_name, state in self.check_all_gpio_pins().items():
                if (pin_name != test_pin) and state:
                    self.print('Detected voltage on pin %s (%s) when %s (%s) driven high.' % (pin_name, self.u1_pin_connections[pin_name], test_pin, self.u1_pin_connections[test_pin]))
                    pin_failure = True
                if (pin_name == test_pin) and not state:
                    self.print('Detected no voltage on pin %s (%s) when driven high.' % (test_pin, self.u1_pin_connections[test_pin]))
                    pin_failure = True
            self.eut_pins[test_pin].write(0)
        if pin_failure:
                self.fail('FAIL 1120: Test of individual GPIO pins failed. See pins listed above.')

    def test_leds(self):
        self.eut.apis.heartbeat.stop()
        for led in range(0, 4):
            LED(self.eut, led+1).on()
        if not self.ask_user("Are LED1, LED2, LED3, and LED4 all illuminated?"):
            self.fail('FAIL 1200: User reported LED failure. Check LED1, LED2, LED3, LED4, R1, R2, R3, R4, and U1 pins 3, 132, 133, and 134.')
        for led in range(0, 4):
            LED(self.eut, led+1).off()
        self.eut.apis.heartbeat.start()

    def test_diode(self):
        diode_drop = self.read_analog_voltage("VBUS_BYPASS") - self.read_analog_voltage("EUT_5V")
        self.print("D5 voltage drop: %.2f V" % diode_drop)
        if diode_drop > 0.275:
            self.fail('FAIL 1300: Voltage drop across diode too high. Check D5. Check for hot spots due to excessive current draw.')

    def glitchkit_host_test(self):
        try:
            descriptor = self.eut.glitchkit.usb.capture_control_in(
                    request= self.eut.glitchkit.usb.GET_DESCRIPTOR,
                    value  = self.eut.glitchkit.usb.GET_DEVICE_DESCRIPTOR,
                    length = 18,
                    timeout = 1
                )
        except:
            #TODO reset and reconnect EUT so that other tests can be done
            self.log(traceback.format_exc())
            pass

    def test_usb1(self):
        if self.read_analog_voltage("USB1_VBUS") > 4.0:
            self.fail('FAIL 1400: USB1_VBUS detected. Check U5, R16, R21, R22, R23, C10, C11. Ensure USB cable from EUT J4/USB1 is connected only to Tester.')
        fdapp = facedancer.greatdancer.GreatDancerApp(device=self.tester)
        fd = facedancer.USBDevice.USBDevice(fdapp)
        fd.connect()
        time.sleep(0.5)
        self.log("initial USB status " + str(self.tester.comms._vendor_request_in(vendor_requests.USBHOST_GET_STATUS, index=0, length=4)))
        if self.tester.comms._vendor_request_in(vendor_requests.USBHOST_GET_STATUS, index=0, length=4)[0] != 133:
            fd.disconnect()
            self.fail('FAIL 1410: unexpected USB1 data line state. Ensure USB cable from EUT J4/USB1 is connected only to Tester.')
        gk_thread=threading.Thread(target=self.glitchkit_host_test)
        gk_thread.start()
        self.report_all_analog_voltages()
        if self.read_analog_voltage("USB1_VBUS") < 4.1:
            fd.disconnect()
            self.fail('FAIL 1420: USB1_VBUS too low. Check U5, R16, R21, R22, R23, C10, C11.')
        if not self.read_io_expander_pin(self.u1, 0, 5):
            self.fail('FAIL 1430: USB1_SENSE voltage not detected. Check R21, R22.')
        #TODO use facedancer on tester and confirm descriptor data
        gk_thread.join()
        self.log("final USB status " + str(self.tester.comms._vendor_request_in(vendor_requests.USBHOST_GET_STATUS, index=0, length=4)))
        if self.tester.comms._vendor_request_in(vendor_requests.USBHOST_GET_STATUS, index=0, length=4)[0] != 5:
            fd.disconnect()
            self.fail('FAIL 1440: USB1 data error. Check USB1 cable. Check J4, R19, R20, R21, R22, R29, R30, C8, C9.')
        fd.disconnect()

    def log(self, output=''):
        if self.logfile:
            print(output, file=self.logfile)

    def print(self, output=''):
        if self.logfile:
            print(output, file=self.logfile)
        print(output)

    def main(self):
        dfu_stub_path = find_greatfet_asset('flash_stub.bin')
        auto_firmware_path = find_greatfet_asset("greatfet_usb.bin")
        parser = argparse.ArgumentParser(
            description="Utility for testing GreatFET One")
        parser.add_argument('-a', '--address', metavar='<n>', type=int,
                            help="starting address (default: 0)", default=0)
        parser.add_argument('-f', '--logfile', metavar='<logfile>', type=str, help="logfile to append to", default='log')
        parser.add_argument('-w', '--write', dest='write', metavar='<filename>', type=str,
                            help="Write data from file")
        parser.add_argument('--dfu-stub', dest='dfu_stub', metavar='<stub.dfu>', type=str,
                            help="The stub to use for DFU programming. If not provided, the utility will attempt to automtaically find one.")
        args = parser.parse_args()

        try:
            self.logfile = open(args.logfile, 'a')
        except:
            self.print('Warning: Could not open log file.')

        self.print("git-" + subprocess.getoutput('git log -n 1 --format=%hk'))
        if args.dfu_stub:
            dfu_stub_path = args.dfu_stub
        if not args.write:
                args.write = auto_firmware_path

        self.find_tester()
        self.initialize_jig()

        # Hold EUT in reset for now.
        self.tester_pins[self.other_pins["RESET_J7_P11"]].write(0)
        self.tester_pins[self.other_pins["RESET_J7_P11"]].set_direction(self.tester.gpio.DIRECTION_OUT)

        # Do not power EUT yet.
        self.tester_pins[self.other_pins["5V_EN"]].write(0)
        self.tester_pins[self.other_pins["5V_EN"]].set_direction(self.tester.gpio.DIRECTION_OUT)
        self.report_all_analog_voltages()

        self.prompt_for_id()
        self.detect_eut()
        self.activate_supply()
        self.test_pull_downs()
        self.test_pull_ups()
        self.test_reset_button()
        self.test_dfu_button()
        self.flash_firmware(args, dfu_stub_path)
        self.setup_eut_pins()
        self.test_gpio()
        self.test_leds()
        self.test_diode()
        self.test_usb1()

        self.print('PASS')

if __name__ == '__main__':
    Narcissus().main()
