from greatfet.interfaces.i2c_device import I2CDevice
from greatfet.programmer import GreatFETProgrammer
import time
from math import log2, ceil, floor

"""
This programmer can configure itself using Microchip part numbers to identify the device we wish to address.
In most cases, we will want to create the programmer like this:
    p = gf.create_programmer('microchipEEPROM', device='24LC128')

Then read some bytes:
     read_bytes = p.read_bytes(0x000, 0x3FFF)

or maybe write some bytes:
    p.write_bytes(0x000, b'\xde\xad\xbe\xef')

Occasionally, we might need to pass an argument to specify the address bits that have been set on the EEPROM:
    p = gf.create_programmer('microchipEEPROM', device='24LC128', slave_address=0b010)

Or to configure the EEPROM ourselves, by specifying the capacity and the page size, with a few other options:
    p = gf.create_programmer(capacity, page_size, bitmask="AAA", slave_address=0, write_cycle_length=0.005)

By default, the first I2C bus will be used. The bus keyword argument can be used in to pass in a different I2C bus object.

"""


BASE_DEVICE_ADDRESS = 0x50

EEPROM_MODELS = \
{'24AA00':      {'page_size':  1,    'max_clock':  400000,   'write_cycle':  0.004,  'capacity':  16,      'bitmask':  'xxx'},
 '24AA01':      {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'xxx'},
 '24AA014':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 '24AA014H':    {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 '24AA01H':     {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'xxx'},
 '24AA02':      {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'xxx'},
 '24AA024':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24AA024H':    {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24AA025':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24AA025E48':  {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24AA025E64':  {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24AA025UID':  {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24AA02E48':   {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'xxx'},
 '24AA02E64':   {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'xxx'},
 '24AA02H':     {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'xxx'},
 '24AA02UID':   {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'xxx'},
 '24AA04':      {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'xxB'},
 '24AA044':     {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'AAB'},
 '24AA04H':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'xxB'},
 '24AA08':      {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  1024,    'bitmask':  'xBB'},
 '24AA08H':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  1024,    'bitmask':  'xBB'},
 '24AA1025':    {'page_size':  128,  'max_clock':  400000,   'write_cycle':  0.003,  'capacity':  131072,  'bitmask':  'BAA'},
 '24AA1026':    {'page_size':  128,  'max_clock':  400000,   'write_cycle':  0.003,  'capacity':  131072,  'bitmask':  'AAB'},
 '24AA128':     {'page_size':  64,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  16384,   'bitmask':  'AAA'},
 '24AA16':      {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'BBB'},
 '24AA16H':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'BBB'},
 '24AA256':     {'page_size':  64,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  32768,   'bitmask':  'AAA'},
 '24AA256UID':  {'page_size':  64,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  32768,   'bitmask':  'AAA'},
 '24AA32A':     {'page_size':  32,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  4096,    'bitmask':  'AAA'},
 '24AA32AF':    {'page_size':  32,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  4096,    'bitmask':  'AAA'},
 '24AA512':     {'page_size':  128,  'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  65536,   'bitmask':  'AAA'},
 '24AA64':      {'page_size':  32,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24AA64F':     {'page_size':  32,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24AA65':      {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24C00':       {'page_size':  1,    'max_clock':  400000,   'write_cycle':  0.004,  'capacity':  16,      'bitmask':  'xxx'},
 '24C01C':      {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.001,  'capacity':  128,     'bitmask':  'AAA'},
 '24C02C':      {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.001,  'capacity':  256,     'bitmask':  'AAA'},
 '24C65':       {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24CW1280':    {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  16384,   'bitmask':  'AAA'},
 '24CW16':      {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'AAA'},
 '24CW160':     {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'AAA'},
 '24CW32':      {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  4096,    'bitmask':  'AAA'},
 '24CW320':     {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  4096,    'bitmask':  'AAA'},
 '24CW64':      {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24CW640':     {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24FC01':      {'page_size':  8,    'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'xxx'},
 '24FC02':      {'page_size':  8,    'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'xxx'},
 '24FC04':      {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'xxB'},
 '24FC04H':     {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'xxB'},
 '24FC08':      {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  1024,    'bitmask':  'xBB'},
 '24FC1025':    {'page_size':  128,  'max_clock':  1000000,  'write_cycle':  0.003,  'capacity':  131072,  'bitmask':  'BAA'},
 '24FC1026':    {'page_size':  128,  'max_clock':  1000000,  'write_cycle':  0.003,  'capacity':  131072,  'bitmask':  'AAB'},
 '24FC128':     {'page_size':  64,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  16384,   'bitmask':  'AAA'},
 '24FC16':      {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'BBB'},
 '24FC16H':     {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'BBB'},
 '24FC256':     {'page_size':  64,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  32768,   'bitmask':  'AAA'},
 '24FC512':     {'page_size':  128,  'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  65536,   'bitmask':  'AAA'},
 '24FC64':      {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24FC64F':     {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24LC00':      {'page_size':  1,    'max_clock':  400000,   'write_cycle':  0.004,  'capacity':  16,      'bitmask':  'xxx'},
 '24LC014':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 '24LC014H':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 '24LC01B':     {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'xxx'},
 '24LC01BH':    {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'xxx'},
 '24LC024':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24LC024H':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24LC025':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24LC02B':     {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'xxx'},
 '24LC02BH':    {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'xxx'},
 '24LC04B':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'xxB'},
 '24LC04BH':    {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'xxB'},
 '24LC08B':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  1024,    'bitmask':  'xBB'},
 '24LC08BH':    {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  1024,    'bitmask':  'xBB'},
 '24LC1025':    {'page_size':  128,  'max_clock':  400000,   'write_cycle':  0.003,  'capacity':  131072,  'bitmask':  'BAA'},
 '24LC1026':    {'page_size':  128,  'max_clock':  400000,   'write_cycle':  0.003,  'capacity':  131072,  'bitmask':  'AAB'},
 '24LC128':     {'page_size':  64,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  16384,   'bitmask':  'AAA'},
 '24LC16B':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'BBB'},
 '24LC16BH':    {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'BBB'},
 '24LC21A':     {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.01,   'capacity':  128,     'bitmask':  '000'},
 '24LC22A':     {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.01,   'capacity':  256,     'bitmask':  '000'},
 '24LC256':     {'page_size':  64,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  32768,   'bitmask':  'AAA'},
 '24LC32A':     {'page_size':  32,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  4096,    'bitmask':  'AAA'},
 '24LC32AF':    {'page_size':  32,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  4096,    'bitmask':  'AAA'},
 '24LC512':     {'page_size':  128,  'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  65536,   'bitmask':  'AAA'},
 '24LC64':      {'page_size':  32,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24LC64F':     {'page_size':  32,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24LC65':      {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 '24LCS21A':    {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.01,   'capacity':  128,     'bitmask':  '000'},
 '24LCS22A':    {'page_size':  8,    'max_clock':  400000,   'write_cycle':  0.01,   'capacity':  256,     'bitmask':  '000'},
 '24VL014':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 '24VL014H':    {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 '24VL024':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24VL024H':    {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 '24VL025':     {'page_size':  16,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 'AT24C01C':    {'page_size':  8,    'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 'AT24C01D':    {'page_size':  8,    'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 'AT24C02C':    {'page_size':  8,    'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 'AT24C02D':    {'page_size':  8,    'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 'AT24C04C':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'AAB'},
 'AT24C04D':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'AAB'},
 'AT24C08C':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  1024,    'bitmask':  'AAA'},
 'AT24C08D':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  1024,    'bitmask':  'ABB'},
 'AT24C128C':   {'page_size':  64,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  16384,   'bitmask':  'AAA'},
 'AT24C16C':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'BBB'},
 'AT24C16D':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'BBB'},
 'AT24C256C':   {'page_size':  64,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  32768,   'bitmask':  'AAA'},
 'AT24C32D':    {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  4096,    'bitmask':  'AAA'},
 'AT24C32E':    {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  4096,    'bitmask':  'AAA'},
 'AT24C512C':   {'page_size':  128,  'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  65536,   'bitmask':  'AAA'},
 'AT24C64B':    {'page_size':  32,   'max_clock':  400000,   'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 'AT24C64D':    {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 'AT24CM01':    {'page_size':  256,  'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAB'},
 'AT24CM02':    {'page_size':  256,  'max_clock':  1000000,  'write_cycle':  0.01,   'capacity':  256,     'bitmask':  'ABB'},
 'AT24CS01':    {'page_size':  8,    'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 'AT24CS02':    {'page_size':  8,    'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 'AT24CS04':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'AAB'},
 'AT24CS08':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  1024,    'bitmask':  'ABB'},
 'AT24CS16':    {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  2048,    'bitmask':  'BBB'},
 'AT24CS32':    {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  4096,    'bitmask':  'AAA'},
 'AT24CS64':    {'page_size':  32,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  8192,    'bitmask':  'AAA'},
 'AT24CSW010':  {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  128,     'bitmask':  'AAA'},
 'AT24CSW020':  {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 'AT24CSW040':  {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'AAA'},
 'AT24CSW080':  {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  1024,    'bitmask':  'AAA'},
 'AT24HC02C':   {'page_size':  8,    'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 'AT24HC04B':   {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  512,     'bitmask':  'AAB'},
 'AT24MAC402':  {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'},
 'AT24MAC602':  {'page_size':  16,   'max_clock':  1000000,  'write_cycle':  0.005,  'capacity':  256,     'bitmask':  'AAA'}}

def create_programmer(board, *args, **kwargs):
    """ Creates a representative programmer for the given module. """

    # Use supplied bus argument, or use the first bus for the board as default
    bus = kwargs.pop('bus', board.i2c_busses[0])

    # If 'device' argument is supplied, intialize for that part number
    if "device" in kwargs:
        device = kwargs.pop('device')
        return EEPROM(bus, device, **kwargs)

    # Otherwise pass all the arguments along to the initializer
    return EEPROMDevice(bus, *args, **kwargs)



def EEPROM(bus, part_number, slave_address=0):
    """
        Create and configure an EEPROM device given a Microchip part number.

        Parameters:
            bus                      -- The I2C bus to connect the device on
            part_number              -- The Microchip part number
    """
    if part_number not in EEPROM_MODELS:
        raise ValueError("Unknown model of EEPROM.")
    config = EEPROM_MODELS[part_number]
    eeprom = EEPROMDevice(
        bus, 
        config['capacity'],
        config['page_size'],
        bitmask = config['bitmask'],
        slave_address = slave_address, 
        write_cycle_length = config['write_cycle'])
    return eeprom

def setbits(word, bits, value):
    """Map each bit of a value over a list of bit locations in a word"""
    # Make sure that value is expressible with bits given
    assert(1<<len(bits) > value)
    for i in range(0, len(bits)): 
        if value & 1<<i: 
            word |= 1<<bits[i] 
        else: 
            word &= ~(1<<bits[i]) 
    return word 

class EEPROMDevice(GreatFETProgrammer):
    """
        Class representing a Microchip I2C serial EEPROM connected to a GreatFET I2C bus.
    """

    def __init__(self, i2c_bus, capacity, page_size, bitmask="AAA", slave_address=0, write_cycle_length=0.005):
        """
        Creates a new Microchip I2C EEPROM Device.

        EEPROMDevice is directly instantiable, but typically you would instantiate it via the EEPROM() helper
        function which will configure an object appropriately based on its part number.

        Parameters:
            i2c_bus             -- The I2C bus to connect the device on.
            capacity            -- The size of the ROM in bytes.
            page_size           -- The size of a page. All writes are page aligned.
            bitmask             -- String describing the meanings of the lowest three bits of the device address
            slave_address       -- Slave address set on pins A0-A3, if present on package. Defaults to 0b000.
            write_cycle_length  -- Time to wait for a write cycle, in seconds. Defaults to 5ms.
        """

        self.capacity           = capacity
        self.page_size          = page_size
        self.write_cycle_length = write_cycle_length
        self.bus                = i2c_bus

        base_address            = BASE_DEVICE_ADDRESS

        # Which bits are actually used to addess the chip?
        # Some devices will listen on a block of devices.
        mask = 0b1111000

        # Work out what the three LSBs of the device slave address mean from the bitmask
        block_address_bits  = []
        slave_address_bits  = []
        for i in range(0,3):
            bit = bitmask[2-i]
            if bit == '0' or bit == '1':                # Bit is of fixed value
                mask |= 1<<i
                base_address |= int(bit) * 1<<i
            elif bit == 'A':                            # Bit is part of pin selectable slave address
                mask |= 1<<i
                slave_address_bits.append(i)
            elif bit == 'B':                            # Bit is part of block select address
                mask |= 1<<i
                block_address_bits.append(i)
            else:                                       # Bit is a "don't care"
                pass

        # Set the slave address
        base_address = setbits(base_address, slave_address_bits, slave_address)

        # Some EEPROMs are arranged in blocks, with each block having a different 
        # Create an I2C device for each block
        self.blocks = []
        for i in range(0, 1<<len(block_address_bits)):
            address = setbits(base_address, block_address_bits, i)
            block   = I2CDevice(i2c_bus, address, name="Microchip EEPROM")
            self.blocks.append(block)

        # Work out how big a block is
        # and how many bits would be required to address it
        self.block_size = int(capacity/len(self.blocks))
        assert(self.block_size % self.page_size == 0)
        if self.block_size<=256:
            self.address_bits = 8
        elif self.block_size <= 65536:
            self.address_bits = 16
        else:
            raise(ValueError("Specified capacity and number of block address bits would result in a block size that would take more than 16 bits to address."))


    def encode_address(self, address):
        """Encode an address"""
        if self.address_bits == 8:
            l = address & 0xFF
            return bytes([l])
        else:
            h = (address >> 8) & 0xFF
            l = address & 0xFF
            return bytes([h, l])
    
    def device_for_address(self,address):
        """Returns the appropriate device for handling writes to this address"""
        block = address>>self.address_bits
        return self.blocks[block]
    
    def read_bytes(self, start_address, end_address):
        """
            Read bytes sequentially from a specified memory address as a bytestring.
            Will handle buffering and reads across blocks transparently
            
            Parameters:
                start_address -- Address of the start of the block to read.
                end_address   -- Address of the end of the block, inclusive.

            Returns:
                A bytestring
        """
        if start_address<0 or start_address>=self.capacity:
            raise ValueError("Invalid start address.")
        if end_address<0 or end_address>=self.capacity:
            raise ValueError("Invalid end address.")
        if end_address<start_address:
            raise ValueError("End address must come after start.")

        # What size chunks can we read from the chip?
        buff_size = self.bus.buffer_size

        buff = b''
        addr = start_address

        # Our read might span multiple blocks, and block boundaries cannot be read through
        while addr<=end_address:
            # Aim to read until the end address or the end of the block, which ever comes first
            end_of_block = (floor(addr/self.block_size) + 1) * self.block_size - 1
            max_addr = min(end_of_block, end_address)

            # Select appropriate device and write address bytes to it
            device = self.device_for_address(addr)
            device.write(self.encode_address(addr))

            # Read bytes in chunks matched to the read buffer size
            # Either to the end of the block, or to the end address
            while addr<=max_addr:
                bytes_to_read = (max_addr - addr) + 1
                read_data = device.read(min(bytes_to_read, buff_size))
                buff = buff + bytes(read_data)
                addr = addr + len(read_data)
        
        return buff

    def write_bytes(self, word_address, data, write_cycle_length=0.005, attempts=2):
        """
            Write bytes sequentially starting at a specified memory address. Will read data back to assure write was successful.
            Handles 
            Parameters:
                word_address       -- Address of the start of the block to write.
                data               -- Data to write, as a bytestring
                attempts           -- The number of attempts made to write to the ROM
                                      and verify the contents before giving up. Defaults to 2.
                                      A value of 0 forgoes verification step and will just
                                      assume it worked.
        """

        bytes_to_write = len(data)
        data           = bytes(data)              # What if it's something weird and not a bytestring? What then?

        # We need to break the bytes up so that each write fits within page boundaries
        # Crossing a page boundary will cause a write cycle to begin, resulting in
        # subsequent bytes being ignored.
        i       = 0
        retries = attempts

        while i<bytes_to_write:

            # Calculate how many bytes we can write out before hitting a page/block boundary
            addr = word_address + i
            writeable_bytes = addr % self.page_size
            writeable_bytes = writeable_bytes if writeable_bytes != 0 else self.page_size
            l = min(bytes_to_write - i, writeable_bytes)

            # Find appropriate device for this address
            device = self.device_for_address(addr)

            # Write out 1 or 2 address bytes, then the current chunk of data
            device.write(self.encode_address(addr) + data[i:i + l])

            # Wait for write cycle to complete
            time.sleep(self.write_cycle_length)

            # If attempts is 0, skip verification step
            if attempts == 0:
                i += l
                continue

            # Read back data to verify the write, retrying if necessary
            written_bytes = self.read_bytes(addr, addr+(l-1))
            if data[i:i + l] == written_bytes:
                i += l
                retries = attempts
            else:
                retries = retries - 1
                if retries == 0:
                    raise RuntimeError("Could not write to EEPROM.")

