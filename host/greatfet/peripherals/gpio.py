#
# This file is part of GreatFET
#

from ..peripheral import GreatFETPeripheral
from ..protocol import vendor_requests


class GPIO(GreatFETPeripheral):
    """
    Interact with the GPIO lines on a GreatFET board
    """

    def __init__(self, board):
        """
        Args:
            board -- GreatFET board whose GPIO lines are to be controlled
        """
        self.board = board
        # XXX it's necessary to reset the state of all GPIO on the GreatFET
        # here because the firmware currently provides no way to read its
        # existing configuration; this should be fixed so GPIO state is never
        # changed unless the user requests to change it.
        self.reset()

    def reset(self):
        """
        Reset the state of all GPIO lines.  This changes all lines back
        to the power-on defaults.
        """
        self.board.vendor_request_out(vendor_requests.GPIO_RESET)

        # clear mappings of lines (port, pin) to the indexes the firmware uses
        # in its gpio_in[] and gpio_out[] arrays
        # {(port, pin): index, (port, pin): index, ...}
        self._inputs = {}
        self._outputs = {}

    def setup(self, line, direction):
        """
        Configure a GPIO line for use as an input or output.  This must be
        called before the line can be used by other functions.

        Args:
            line -- (port, pin); typically a tuple from J1, J2, J7 below
            direction -- Directions.IN (input) or Directions.OUT (output)

        TODO: allow pull-up/pull-down resistors to be configured for inputs
        """
        if direction == Directions.IN:
            this_dict, other_dict = self._inputs, self._outputs
            num_inputs = 1
        else:
            this_dict, other_dict = self._outputs, self._inputs
            num_inputs = 0

        if this_dict.get(line) is None:
            # register this line if it isn't already registered
            self.board.vendor_request_out(vendor_requests.GPIO_REGISTER,
                value=num_inputs, data=line)
            # save the index that the firmware should have assigned it
            index = len(this_dict)
            this_dict[line] = index

        if line in other_dict:
            # if the line was previously in the other direction, mark that old
            # registration as unusable.  firmware doesn't support unregistering
            # so the entry must be kept in the dict to preserve the count.
            other_dict[line] = None

    def output(self, line, state):
        """
        Set the state of an output line.  The line must have previously been
        configured as an output using setup().

        Args:
            line -- (port, pin); typically a tuple from J1, J2, J7 below
            state -- True sets line high, False sets line low
        """
        gpio_out_index = self._outputs.get(line)
        if gpio_out_index is None:
            raise ValueError("GPIO line %s not set up as output" % repr(line))

        self.board.vendor_request_out(vendor_requests.GPIO_WRITE,
            data=[gpio_out_index, int(state)])

    def input(self, line):
        """
        Get the state of an input line.  The line must have previously been
        configured as an output using setup().

        Args:
            line -- (port, pin); typically a tuple from J1, J2, J7 below

        Return:
            bool -- True if line is high, False if line is low
        """
        gpio_in_index = self._inputs.get(line)
        if gpio_in_index is None:
            raise ValueError("GPIO line %s not set up as input" % repr(line))

        data = self.board.vendor_request_in(vendor_requests.GPIO_READ,
            length=255)

        byte = gpio_in_index // 8
        bit = 7 - (gpio_in_index % 8)
        return data[byte] & (2**bit) != 0


class Directions(object):
    """"Use for configuring a GPIO line as input or output"""
    IN = 0
    OUT = 1


class J1(object):
    """GreatFET One header J1 pins and their LPC GPIO (port, pin) equivalents"""
    # P1 = GND
    # P2 = VCC
    P3 = (5, 13)
    P4 = (0, 0)
    P5 = (5, 14)
    P6 = (0, 1)
    P7 = (0, 4)
    P8 = (2, 9)
    P9 = (2, 10)
    P10 = (0, 8)
    # P11 = CLK0
    P12 = (0, 9)
    P13 = (1, 8)
    P14 = (2, 11)
    P15 = (1, 0)
    P16 = (1, 9)
    P17 = (1, 2)
    P18 = (1, 1)
    P19 = (2, 12)
    P20 = (1, 3)
    P21 = (1, 5)
    P22 = (1, 4)
    P23 = (2, 14)
    P24 = (2, 13)
    P25 = (1, 7)
    P26 = (1, 6)
    P27 = (2, 15)
    P28 = (0, 2)
    P29 = (2, 7)
    P30 = (0, 3)
    P31 = (0, 13)
    P32 = (0, 12)
    P33 = (5, 18)
    P34 = (4, 11)
    P35 = (5, 0)
    # P36 = P6_0
    P37 = (0, 15)
    # P38 = P1_19
    P39 = (0, 11)
    P40 = (0, 10)

class J2(object):
    """GreatFET One header J2 pins and their LPC GPIO (port, pin) equivalents"""
    # P1 = GND
    # P2 = VBUS
    P3 = (5, 12)
    P4 = (2, 0)
    # P5 = ADC0_0
    P6 = (2, 5)
    P7 = (2, 4)
    P8 = (2, 2)
    P9 = (2, 3)
    P10 = (2, 6)
    # P11 = P4_7
    # P12 = CLK2
    P13 = (5, 7)
    P14 = (0, 7)
    P15 = (5, 6)
    P16 = (3, 15)
    # P17 = WAKEUP0
    P18 = (5, 5)
    P19 = (5, 4)
    P20 = (5, 3)
    # P21 = PF_4
    P22 = (5, 9)
    P23 = (3, 10)
    P24 = (5, 8)
    P25 = (3, 9)
    # P26 = P3_0
    P27 = (3, 8)
    P28 = (1, 14)
    P29 = (5, 16)
    P30 = (5, 10)
    P31 = (5, 15)
    # P32 = P3_3
    P33 = (5, 2)
    P34 = (0, 5)
    P35 = (5, 1)
    P36 = (3, 2)
    P37 = (1, 15)
    P38 = (0, 6)
    # P39 = I2C0_SDA
    # P40 = I2C0_SDL

class J7(object):
    """GreatFET One header J7 pins and their LPC GPIO (port, pin) equivalents"""
    # P1 = GND
    P2 = (3, 3)
    P3 = (3, 4)
    # P4 = ADC0_5
    # P5 = ADC0_2
    P6 = (1, 10)
    P7 = (1, 12)
    P8 = (1, 13)
    # P9 = RTC_ALARM
    # P10 = GND
    # P11 = RESET
    # P12 = VBAT
    P13 = (1, 11)
    P14 = (0, 14)
    P15 = (3, 6)
    P16 = (3, 5)
    P17 = (3, 1)
    P18 = (3, 0)
    # P19 = GND
    # P20 = VCC
