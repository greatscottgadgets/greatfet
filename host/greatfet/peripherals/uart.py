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

        self.active_uart[name] = UARTPin(self, name)

        return self.active_uart[name]

        # TODO:
        # If we couldn't create the UART pin, fail out.
        # raise ValueError("No available UART pin {}".format(name))


    def release_pin(self, uart_pin):
        """
        Releases a UART pin back to the system for re-use, potentially
        not as a UART.
        """

        if uart_pin.name != self.active_uart:
            raise ValueError("Trying to release a pin we don't own?")

        # TODO: Disable any pull-ups present on the pin.

        # Remove the UART pin from our active array, and add it back to the
        # available pool.
        del self.active_uart[uart_pin.name]


class UARTPin(object):
    """
    Class representing a single UART pin.
    """

    def __init__(self, uart_collection, name):
        """
        Creates a new object representing a UART Pin. Usually instantiated via
        a UART object.

        Args:
            uart_collection -- The UART object to which this pin belongs.
            name -- The name of the given pin. Should match a name registered
                in its UART collection.
        """

        self.uart = uart_collection
        self.name = name

        # TODO: handle when an invalid pin name is entered
        self.uart_num = -1
        self.port_and_pin = ((-1, -1), -1)
        self.scu_func = -1

        for uart_num, uart_dict in enumerate(self.uart.uart_mappings):
            if name in uart_dict.keys():
                self.uart_num = uart_num
                self.port_and_pin = uart_dict[name][0]
                self.scu_func = uart_dict[name][1]


    def read(self, rx_length=0, num_data_bits=8, num_stop_bits=1, parity_bit=0, baud=9600):
        """
            Reads data from the device connected to the UART pin.
        """

        self.num_data_bits = num_data_bits
        self.num_stop_bits = num_stop_bits
        self.parity_bit = parity_bit
        self.port = self.port_and_pin[0]
        self.pin = self.port_and_pin[1]
        self.baud_rate = int(baud)

        self.uart.api.initialize(self.uart_num, self.baud_rate, self.num_data_bits, self.num_stop_bits, self.parity_bit)
        data_read = self.uart.api.read(rx_length)

        return data_read

    def write(self, data, num_data_bits=8, num_stop_bits=1, parity_bit=0, baud=9600):
        """
            Sends data to the device connected to the UART pin.

            Args:
                data -- The byte(s) to be sent to the given device.
                num_data_bits -- The number of data bits to send per byte.
                num_stop_bits -- The number of stop bits to send per byte.
                parity_bit -- The parity type to use.
        """
        
        self.num_data_bits = num_data_bits
        self.num_stop_bits = num_stop_bits
        self.parity_bit = parity_bit
        self.port = self.port_and_pin[0]
        self.pin = self.port_and_pin[1]
        self.baud_rate = int(baud)

        self.uart.api.initialize(self.uart_num, self.baud_rate, self.num_data_bits, self.num_stop_bits, self.parity_bit)        
        
        for byte in data:
            self.uart.api.synchronous_transmit(self.uart_num, byte)

