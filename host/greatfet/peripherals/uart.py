#
# This file is part of GreatFET
#

from ..peripheral import GreatFETPeripheral

from warnings import warn


class UART(GreatFETPeripheral):
    """
    TODO: description
    """

    def __init__(self, board):
        """
        Args:
            board -- GreatFET board whose UART lines are to be controlled
        """
        self.board = board
        self.api = self.board.apis.uart
        self.pin_mappings = {}

        self.uart_mappings = self.board.UART_MAPPINGS
        # print("uart mappings: ", self.uart_mappings)

        self.available_pins = []
        self.active_uart = {}


    def get_available_pins(self, include_active=True):
        """ Returns a list of available UART names. """
        available = self.available_pins[:]
        available.extend(self.active_uart.keys())

        return available

    # TODO: modify/remove pin tracking function remnants
    def get_pin(self, name, unique=False):
        """
        Returns a UARTPin object by which a given pin can be controlled.

        Args:
            name -- The UART name to be used.
            unique -- True if this should fail if a UART object for this pin
                already exists.
        """

        # TODO: modify/remove these checks

        # If we already have an active UART pin for the relevant name, return it.
        # if name in self.active_uart and not unique:
        #     return self.active_uart[name]

        # If the pin's available for UART use, grab it.
        # if name in self.available_pins:
        #     port = self.pin_mappings[name]
        #     print("port:", port)

        self.active_uart[name] = UARTPin(self, name)
        # self.mark_pin_as_used(name)

        return self.active_uart[name]

        # If we couldn't create the UART pin, fail out.
        # raise ValueError("No available UART pin {}".format(name))


    def release_pin(self, uart_pin):
        """
        Releases a UART pin back to the system for re-use, potentially
        not as a UART.
        """

        if uart_pin.name != self.active_uart:
            raise ValueError("Trying to release a pin we don't own?")

        # Mark the pin as an input, placing it into High-Z mode.
        # TODO: Disable any pull-ups present on the pin.
        # gpio_pin.set_direction(DIRECTION_IN)

        # Remove the UART pin from our active array, and add it back to the
        # available pool.
        del self.active_uart[uart_pin.name]
        # self.mark_pin_as_unused(uart_pin.name)


class UARTPin(object):
    """
    Class representing a single UART pin.
    """

    def __init__(self, uart_collection, name):#, port_and_pin):
        """
        Creates a new object representing a UART Pin. Usually instantiated via
        a UART object.

        Args:
            gpio_collection -- The UART object to which this pin belongs.
            name -- The name of the given pin. Should match a name registered
                in its UART collection.
            port_and_pin -- A 2-tuple containing LPC4330 (port, pin) numbers.

        """

        self.uart = uart_collection
        self.name = name

        for uart_num, uart_dict in enumerate(self.uart.uart_mappings):
            if name in uart_dict.keys():
                self.uart_num = uart_num
                self.port_and_pin = uart_dict[name][0]
                self.scu_func = uart_dict[name][1]

        # TODO: handle when an invalid pin name is entered

    def read(self):
        """
            Reads data from the device connected to the UART pin.
        """

        read_data = self.uart.api.read(self.uart_num)
        return read_data

    def write(self, data, num_data_bits=8, num_stop_bits=1, parity_bit=0, divisor=1328, divaddval=0, mulval=1):
        """
            Sends data to the device connected to the UART pin.

            Args:
                data -- The data to be sent to the given device.
                num_data_bits -- The number of data bits to send.
                num_stop_bits -- The number of stop bits to send.
                parity_bit -- The parity type to use.
                TODO:
                    calculate divisor/divaddval/mulval from a given baud rate
        """
        
        self.num_data_bits = num_data_bits
        self.num_stop_bits = num_stop_bits
        self.parity_bit = parity_bit
        self.divisor = divisor
        self.divaddval = divaddval
        self.mulval = mulval
        self.port = self.port_and_pin[0]
        self.pin = self.port_and_pin[1]

        print("pin: ", self.port_and_pin)

        self.uart.api.init(self.uart_num, self.num_data_bits, self.num_stop_bits, self.parity_bit, self.divisor, self.divaddval, self.mulval)
        self.uart.api.write(self.uart_num, self.scu_func, self.port, self.pin, data)

