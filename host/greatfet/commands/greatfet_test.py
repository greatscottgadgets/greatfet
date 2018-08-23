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

from greatfet import GreatFET
from greatfet.errors import DeviceNotFoundError
from greatfet.utils import log_silent, log_verbose
from greatfet.peripherals.i2c_device import I2CDevice
from greatfet.peripherals.gpio import GPIO
from greatfet.peripherals.led import LED
from greatfet.protocol import vendor_requests
import greatfet_firmware

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
    "J1_P39"   : "J1_P39",
    "J1_P40"   : "J1_P40",
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
    "J2_P13"   : (2, 3, 5),
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
    #"J7_P10"  : (2, 1, 6) GND on EUT
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
    "GND_J7_P10"      : (2, 1, 6), #GND
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

# how analog signals are connected to the multiplexer (U4)
analog_signals = {
    "VBUS_BYPASS" : 0, # 50% voltage divider
    "USB0_VBUS"   : 1, # 50% voltage divider
    "USB1_VBUS"   : 2, # 50% voltage divider
    "EUT_VCC"     : 3, # 50% voltage divider
    "EUT_5V"      : 4, # 50% voltage divider
    "EUT_ADC0_2"  : 5,
    "EUT_ADC0_0"  : 6,
    "EUT_ADC0_5"  : 7
}

eut_pins = {}
tester_pins = {}

def ask_user(question):
    print('%s [y/n]' % question)
    while True:
        try:
            return strtobool(input())
        except ValueError:
            print('Please respond with \'y\' or \'n\'.')

def fail(message):
    print(message, file=sys.stderr)
    sys.exit()

def read_adc(device):
    data = device.vendor_request_in(vendor_requests.ADC_READ, length=2)
    result = (data[1] << 8) | data[0]
    return result

# configure analog multiplexer
def select_analog_signal(tester, signal):
        tester_pins[other_pins["U4_E"]].write(1)
        tester_pins[other_pins["U4_S0"]].write(analog_signals[signal] & 0x1)
        tester_pins[other_pins["U4_S1"]].write((analog_signals[signal] >> 1) & 0x1)
        tester_pins[other_pins["U4_S2"]].write((analog_signals[signal] >> 2) & 0x1)
        tester_pins[other_pins["U4_E"]].write(0)

def read_analog_signal(tester, signal):
    select_analog_signal(tester, signal)
    time.sleep(0.1)
    return read_adc(tester)

def read_io_expanders(expanders):
    state = []
    for expander in expanders:
        state.extend(expander.transmit([], 2))
    #print(state)
    return state

def read_io_expander_pin(expander, port, pin):
    return (read_io_expanders([expander])[port] >> pin) & 1

def check_gpio_pin(tester, name):
    tester_pins[name].set_direction(tester.gpio.DIRECTION_IN)
    return tester_pins[name].read()

def setup_eut_pins(eut):
    for eut_pin_name in list(gpio_pins):
        eut_pins[eut_pin_name] = eut.gpio.get_pin(eut_pin_name)

def setup_tester_pins(tester):
    for eut_pin_name in list(gpio_pins):
        if not isinstance(gpio_pins[eut_pin_name], tuple):
            tester_pins[gpio_pins[eut_pin_name]] = tester.gpio.get_pin(gpio_pins[eut_pin_name])
    for signal in list(other_pins):
        if not isinstance(other_pins[signal], tuple):
            tester_pins[other_pins[signal]] = tester.gpio.get_pin(other_pins[signal])

def check_all_gpio_pins(tester, expander1, expander2):
    pins = {}
    expander_state = read_io_expanders([expander1, expander2])
    for eut_pin_name in list(gpio_pins):
        if isinstance(gpio_pins[eut_pin_name], tuple):
            (expander, port, pin) = gpio_pins[eut_pin_name]
            port += (expander - 1) * 5
            pins[eut_pin_name] = (1 == (expander_state[port] >> pin) & 1)
        #FIXME why does this fail?
        #else:
            #pins[eut_pin_name] = check_gpio_pin(tester, gpio_pins[eut_pin_name])
    return pins

def test_gpio(eut, tester, expander1, expander2):
    # set all GPIO pins output high on EUT
    for pin_name in list(gpio_pins):
        eut_pins[pin_name].set_direction(eut.gpio.DIRECTION_OUT)
        eut_pins[pin_name].write(1)
    pin_failure = False
    for pin_name, state in check_all_gpio_pins(tester, expander1, expander2).items():
        if not state:
            print('Detected no voltage on pin %s.' % pin_name, file=sys.stderr)
            pin_failure = True
    if pin_failure:
            fail('FAIL 1140: Detected no voltage on GPIO pin when all pins were driven high. See pins listed above.')

    # set all GPIO pins output low on EUT
    for pin_name in list(gpio_pins):
        eut_pins[pin_name].write(0)
    for pin_name, state in check_all_gpio_pins(tester, expander1, expander2).items():
        if state:
            print('Detected voltage on pin %s.' % pin_name, file=sys.stderr)
            pin_failure = True
    if pin_failure:
            fail('FAIL 1150: Detected voltage on GPIO pin when all pins were driven low. See pins listed above.')

    # test GPIO pins one at a time
    for test_pin in list(gpio_pins):
        eut_pins[test_pin].write(1)
        for pin_name, state in check_all_gpio_pins(tester, expander1, expander2).items():
            if (pin_name != test_pin) and state:
                print('Detected voltage on pin %s when %s driven high.' % (pin_name, test_pin), file=sys.stderr)
                pin_failure = True
            if (pin_name == test_pin) and not state:
                print('Detected no voltage on pin %s when driven high.' % test_pin, file=sys.stderr)
                pin_failure = True
        eut_pins[test_pin].write(0)
    if pin_failure:
            fail('FAIL 1160: Test of individual GPIO pins failed. See pins listed above.')

def test_leds(eut):
    for led in range(1, 4):
        LED(eut, led+1).set()
    if not ask_user("Are LED2, LED3, and LED4 illuminated?"):
        fail('FAIL 1200: User reported LED failure. Check LED2, LED3, LED4, R2, R3, R4, and U1 pins 3, 132, and 133.')
    for led in range(1, 4):
        LED(eut, led+1).clear()
    if not ask_user("Is LED1 blinking?"):
        fail('FAIL 1210: User reported LED1 failure. Check LED1, R1, and U1 pin 134.')

def test_usb1(tester, u1):
    tester_pins[other_pins["USB1_EN"]].set_direction(tester.gpio.DIRECTION_OUT)
    tester_pins[other_pins["USB1_EN"]].write(1)
    time.sleep(10)
    #FIXME analog
    if not read_io_expander_pin(u1, 0, 5):
        fail('FAIL 1300: USB1_VBUS voltage not detected. Check U5, R16, R21, R22, R23, R24, C10, C11.')
    tester_pins[other_pins["USB1_EN"]].write(0)
    if read_io_expander_pin(u1, 0, 5):
        fail('FAIL 1310: USB1_VBUS voltage detected. Unplug USB cable from USB1. Check U5, R16, R21, R22.')

    print('Connect USB cable between EUT USB1 and Tester USB1.')
    timeout = time.time() + 30
    while(not read_io_expander_pin(u1, 0, 5)):
        if time.time() >= timeout:
            fail("FAIL 1320: Timeout while waiting for USB1 cable. Check J4, R21.")
        pass
    print('Detected USB1 cable.')
    tester_pins[other_pins["USB1_EN"]].set_direction(tester.gpio.DIRECTION_IN)

def find_tester():
    devices = []
    timeout = time.time() + 30
    while len(devices) < 1:
        try:
            devices = GreatFET(find_all=True)
        except:
            print("Unexpected error:", sys.exc_info()[0])
            pass
        if time.time() >= timeout:
            print('FAIL 10: Tester not found. Connect Tester to Narcissus and connect Tester to this host with USB.', file=sys.stderr)
            sys.exit(errno.ENODEV)
        time.sleep(1)


    if len(devices) > 1:
        fail('FAIL 20: More than one GreatFET found. Connect only Tester to this host with USB.')

    # Print the Tester's information...
    tester = devices[0]
    print("Found Tester {}.".format(tester.board_name()))
    print("  Board ID: {}".format(tester.board_id()))
    print("  Firmware version: {}".format(tester.firmware_version()))
    print("  Part ID: {}".format(tester.part_id()))
    print("  Serial number: {}".format(tester.serial_number()))
    print(" ")

    setup_tester_pins(tester)
    tester_pins[other_pins["U4_E"]].set_direction(tester.gpio.DIRECTION_OUT)
    tester_pins[other_pins["U4_E"]].write(1)
    tester_pins[other_pins["U4_S0"]].set_direction(tester.gpio.DIRECTION_OUT)
    tester_pins[other_pins["U4_S1"]].set_direction(tester.gpio.DIRECTION_OUT)
    tester_pins[other_pins["U4_S2"]].set_direction(tester.gpio.DIRECTION_OUT)
    tester.vendor_request_out(vendor_requests.ADC_INIT)
    return tester

def initialize_jig(tester):
    u1 = I2CDevice(tester.i2c, 0x41>>1)

    # if read_io_expanders([u1]) == [255, 255, 255, 255, 255]:
    #     fail('FAIL 100: I2C I/O expander U1 not detected. Connect Tester to Narcissus.')
    #     sys.exit(errno.ENODEV)
    read_io_expanders([u1])

    return u1

def flash_firmware(args, tester):
    timeout = time.time() + 30
    while True:
        try:
            greatfet_firmware.load_dfu_stub(args)
            time.sleep(1)
            break
        except DeviceNotFoundError:
            if time.time() >= timeout:
                print("FAIL 1100: EUT in DFU mode not found.", file=sys.stderr)
                sys.exit(errno.ENODEV)
        except IOError:
            print('DFU IOError', file=sys.stderr)
            pass

    devices = GreatFET(find_all=True)
    timeout = time.time() + 10
    while len(devices) < 2:
        if time.time() >= timeout:
            print("FAIL 1110: EUT running from RAM not found.", file=sys.stderr)
            sys.exit(errno.ENODEV)
        if not devices:
            print('FAIL 1120: Tester not found. Connect Tester to Narcissus and connect Tester to this host with USB.', file=sys.stderr)
            sys.exit(errno.ENODEV)
        time.sleep(1)
        devices = GreatFET(find_all=True)
    for device in devices:
        if device.serial_number() != tester.serial_number():
            eut = device

    print('Writing data to SPI flash.')
    greatfet_firmware.spi_flash_write(eut, args.write, args.address, log_verbose)
    print('Write complete.')
    print('Resetting EUT')
    eut.reset(reconnect=False)
    print("Reset complete!")
    time.sleep(1)

    devices = GreatFET(find_all=True)
    timeout = time.time() + 10
    while len(devices) < 2:
        if time.time() >= timeout:
            print("FAIL 1130: EUT running from flash not found.", file=sys.stderr)
            sys.exit(errno.ENODEV)
        if not devices:
            print('FAIL 1140: Tester not found. Connect Tester to Narcissus and connect Tester to this host with USB.', file=sys.stderr)
            sys.exit(errno.ENODEV)
        time.sleep(1)
        devices = GreatFET(find_all=True)
    for device in devices:
        if device.serial_number() != tester.serial_number():
            eut = device

    # Print the EUT's information...
    print("Found EUT {}.".format(eut.board_name()))
    print("  Board ID: {}".format(eut.board_id()))
    print("  Firmware version: {}".format(eut.firmware_version()))
    print("  Part ID: {}".format(eut.part_id()))
    print("  Serial number: {}".format(eut.serial_number()))
    print(" ")

    return eut

def main():
    parser = argparse.ArgumentParser(
        description="Utility for testing GreatFET One")
    parser.add_argument('-a', '--address', metavar='<n>', type=int,
                        help="starting address (default: 0)", default=0)
    parser.add_argument('-w', '--write', dest='write', metavar='<filename>', type=str,
                        help="Write data from file", default='/tmp/greatfet_usb.bin')
    parser.add_argument('--dfu-stub', dest='dfu_stub', metavar='<stub.dfu>', type=str,
                        help="The stub to use for DFU programming. If not provided, the utility will attempt to automtaically find one.",
                        default='/tmp/greatfet_usb.dfu')
    args = parser.parse_args()

    print("git-" + subprocess.getoutput('git log -n 1 --format=%hk'))

    eut = None

    tester = find_tester()

    u1, u2 = initialize_jig(tester)

    tester_pins[other_pins["RESET_J7_P11"]].set_direction(tester.gpio.DIRECTION_OUT)
    tester_pins[other_pins["RESET_J7_P11"]].write(0)

    tester_pins[other_pins["5V_EN"]].set_direction(tester.gpio.DIRECTION_OUT)
    tester_pins[other_pins["5V_EN"]].write(1)

    print('Connect Equipment Under Test (EUT) to spring pins on Narcissus. Do not connect EUT USB cables.')

    while True:
        for signal in analog_signals.keys():
            print(signal, read_analog_signal(tester, signal))
        print()
        time.sleep(1)

    eut_detected = False
    while True:
        if read_analog_signal(tester, "USB0_VBUS") > 100:
            eut_detected = True
        if read_analog_signal(tester, "USB1_VBUS") > 100:
            eut_detected = True
        time.sleep(0.5)
        if eut_detected:
            if read_analog_signal(tester, "USB0_VBUS") > 100:
                break
            if read_analog_signal(tester, "USB1_VBUS") > 100:
                break

    print('Detected EUT.')

    time.sleep(1)
    print("USB0_VBUS", read_analog_signal(tester, "USB0_VBUS"))
    print("USB1_VBUS", read_analog_signal(tester, "USB1_VBUS"))
    time.sleep(1)
    print("USB0_VBUS", read_analog_signal(tester, "USB0_VBUS"))
    print("USB1_VBUS", read_analog_signal(tester, "USB1_VBUS"))
    if read_analog_signal(tester, "USB0_VBUS") > 650:
        fail('FAIL 150: USB0 cable detected. Unplug USB cable from EUT J3/USB0.')
    if read_analog_signal(tester, "USB1_VBUS") > 650:
        fail('FAIL 160: USB1 cable detected. Unplug USB cable from EUT J4/USB1.')

    if read_analog_signal(tester, "USB0_VBUS") <= 100:
        fail('FAIL 170: USB0_VBUS not detected. Check connection of EUT to Narcissus. Check D5, FB1.')
    if read_analog_signal(tester, "USB1_VBUS") <= 100:
        fail('FAIL 180: USB1_VBUS not detected. Check connection of EUT to Narcissus. Check U5, C11.')

    # check that all GND pins are low
    if read_io_expander_pin(u2, 2, 4):
        fail('FAIL 200: GND_J2_P1 voltage detected. Check connection of EUT to Narcissus. Check J1 pin 1. Check J2 pin 1.')
    if read_io_expander_pin(u1, 0, 1):
        fail('FAIL 210: GND_J7_P1 voltage detected. Check connection of EUT to Narcissus. Check J7 pin 1.')
    if read_io_expander_pin(u2, 1, 6):
        fail('FAIL 220: GND_J7_P10 voltage detected. Check connection of EUT to Narcissus. Check J7 pin 10.')
    if read_io_expander_pin(u2, 0, 6):
        fail('FAIL 230: GND_J7_P19 voltage detected. Check connection of EUT to Narcissus. Check J7 pin 19.')

    # check that all signals with pull-down resistors are low
    if read_io_expander_pin(u1, 0, 5):
        fail('FAIL 250: USB1_SENSE voltage detected. Check connection of EUT to Narcissus. Check U5, R16, R21, R22.')
    if read_io_expander_pin(u1, 4, 2):
        fail('FAIL 260: J1_P12 voltage detected. Check connection of EUT to Narcissus. Check J1 pin 12, R8.')
    if read_io_expander_pin(u2, 2, 2):
        fail('FAIL 270: J7_P6 voltage detected. Check connection of EUT to Narcissus. Check J7 pin 6, R7.')
    if read_io_expander_pin(u2, 3, 5):
        fail('FAIL 280: J2_P13 voltage detected. Check connection of EUT to Narcissus. Check J2 pin 13, R6.')
    if check_gpio_pin(tester, 'J1_P35'):
        fail('FAIL 290: EUT not detected. Check connection of EUT to Narcissus. Check R16.')

    #FIXME replace VCC_J7_P20 check with analog check of EUT_VCC
    #if read_io_expander_pin(u2, 0, 5): #VCC_J7_P20
        #fail('FAIL 300: EUT target power detected. Disconnect USB cables from EUT.')

    #enable_5v.write(1)
    if not read_io_expander_pin(u2, 0, 5): #VCC_J7_P20
        fail('FAIL 310: EUT target power not detected. Check J1 pin 2, J7 pin 20, U3, C3, C4.')
    #FIXME check EUT_VCC, EUT_5V, USB0_VBUS, VBUS_BYPASS, USB1_VBUS with ADC

    # check that all signals with pull-up resistors are high
    if not check_gpio_pin(tester, 'J1_P31'):
        fail('FAIL 320: RESET voltage not detected. Check J7 pin 11, R5, SW2, U1 pin 128.')
    if not read_io_expander_pin(u1, 4, 3):
        fail('FAIL 330: P1_1 voltage not detected. Check J1 pin 10, R26.')
    if not read_io_expander_pin(u2, 4, 1):
        fail('FAIL 340: WAKEUP0 voltage not detected. Check J2 pin 17, R14.')
    if not read_io_expander_pin(u1, 0, 3):
        fail('FAIL 350: USB1_FAULT voltage not detected. Check R24.')
    if not read_io_expander_pin(u2, 3, 4):
        fail('FAIL 360: P2_7 voltage not detected. Check J2 pin 14, R15.')
    if not check_gpio_pin(tester, 'J2_P19'):
        fail('FAIL 370: I2C0_SCL voltage not detected. Check J2 pin 40, R18.')
    if not check_gpio_pin(tester, 'J2_P20'):
        fail('FAIL 380: I2C0_SDA voltage not detected. Check J2 pin 39, R17.')
    if not check_gpio_pin(tester, 'J1_P25'):
        fail('FAIL 390: TDO voltage not detected. Check R12.')
    if not check_gpio_pin(tester, 'J1_P27'):
        fail('FAIL 390: TCK voltage not detected. Check R11.')
    if not check_gpio_pin(tester, 'J1_P21'):
        fail('FAIL 400: DBGEN voltage not detected. Check R10.')
    if not read_io_expander_pin(u1, 0, 2):
        fail('FAIL 410: LED1 voltage not detected. Check D1, R1.')
    if not read_io_expander_pin(u1, 0, 4):
        fail('FAIL 420: LED2 voltage not detected. Check D2, R2.')
    if not read_io_expander_pin(u1, 1, 1):
        fail('FAIL 430: LED3 voltage not detected. Check D3, R3.')
    if not read_io_expander_pin(u1, 1, 2):
        fail('FAIL 440: LED4 voltage not detected. Check D4, R4.')

    # check that all signals with pull-down resistors are still low
    #FIXME there is enough reverse leakage through D5 to trigger USB0_SENSE
    #if read_io_expander_pin(u1, 1, 0):
        #fail('FAIL 450: USB0_SENSE voltage detected. Check D5, R25, R26.')
    if read_io_expander_pin(u1, 0, 5):
        fail('FAIL 460: USB1_SENSE voltage detected. Unplug cable from USB1. Check U5, R16, R21, R22.')
    if read_io_expander_pin(u1, 4, 2):
        fail('FAIL 470: P1_2 voltage detected. Check J1 pin 12, R8.')
    if read_io_expander_pin(u2, 2, 2):
        fail('FAIL 480: P2_9 voltage detected. Check J7 pin 6, R7.')
    if read_io_expander_pin(u2, 3, 5):
        fail('FAIL 490: P2_8 voltage detected. Check J2 pin 13, R6, SW1.')
    if check_gpio_pin(tester, 'J1_P35'):
        fail('FAIL 500: USB1_EN voltage detected. Check R16.')

    print('Press RESET button (SW2) on EUT.')
    timeout = time.time() + 30
    while(check_gpio_pin(tester, 'J1_P31')):
        if time.time() >= timeout:
            fail("FAIL 600: Timeout while waiting for RESET button. Check SW2.")
        pass
    print('Detected RESET button.')

    print('Press DFU button (SW1) on EUT.')
    timeout = time.time() + 30
    while(not read_io_expander_pin(u2, 3, 5)):
        if time.time() >= timeout:
            fail("FAIL 610: Timeout while waiting for DFU button. Check SW1.")
        pass
    print('Detected DFU button.')

    #enable_5v.write(0)
    tester_pins[other_pins["5V_EN"]].write(0)
    #FIXME verify power has gone away

    print('Connect EUT to this host with USB cable (J3/USB0) while pressing DFU button (SW1).')

    if not read_io_expander_pin(u2, 0, 5):
        fail('FAIL 1000: EUT target power not detected. Check J1 pin 2, J7 pin 20, U3, C3, C4.')
    #FIXME check EUT_VCC, EUT_5V, USB0_VBUS, VBUS_BYPASS, USB1_VBUS with ADC

    eut = flash_firmware(args, tester)
    setup_eut_pins(eut)
    test_gpio(eut, tester, u1, u2)
    test_leds(eut)
    test_usb1(tester, u1)
    print('PASS')

if __name__ == '__main__':
    main()