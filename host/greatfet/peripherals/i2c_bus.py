#
# This file is part of GreatFET
#

from ..peripheral import GreatFETPeripheral


class I2CBus(GreatFETPeripheral):
    """
        Class representing a GreatFET I2C bus.

        For now, supports only the primary I2C bus (I2C0), but will be
        expanded when the vendor commands are.
    """

    def __init__(self, board, name='i2c bus', buffer_size=255, duty_cycle_count=255):
        """
            Initialize a new I2C bus.

            Args:
                board -- The GreatFET board whose I2C bus we want to control.
                name -- The display name for the given I2C bus.
                buffer_size -- The size of the I2C receive buffer on the GreatFET.
        """

        # Store a reference to the parent board.
        self.board = board

        # Store our limitations.
        self.buffer_size = buffer_size

        # Create a list that will store all connected devices.
        self.devices = []

        # Store our duty cycle count
        self.duty_cycle_count = duty_cycle_count

        # Set up the I2C bus for communications.
        board.apis.i2c.start(duty_cycle_count)

    def attach_device(self, device):
        """
            Attaches a given I2C device to this bus. Typically called
            by the I2C device as it is constructed.

            Arguments:
                device -- The device object to attach to the given bus.
        """

        # TODO: Check for address conflicts!

        self.devices.append(device)

    def read(self, address, receive_length=0):
        """
            Reads data from the I2C bus.

            Args:
                address -- The 7-bit I2C address for the target device.
                    Should not contain read/write bits. Can be used to address
                    special addresses, for now; but this behavior may change.
                receive_length -- The I2C controller will attempt
                        to read the provided amount of data, in bytes.
        """

        if (not isinstance(receive_length, int)) or receive_length < 0:
            raise ValueError("invalid receive length!")

        if receive_length > self.buffer_size:
            raise ValueError("Tried to receive more than the size of the receive buffer.")

        if address > 127 or address < 0:
            raise ValueError("Tried to transmit to an invalid I2C address!")

        read_response = self.board.apis.i2c.read(address, receive_length)
        read_status = read_response[-1]
        read_data = read_response[:-1]

        return read_data, read_status

    def write(self, address, data):
        """
            Sends data over the I2C bus.

            Args:
                address -- The 7-bit I2C address for the target device.
                    Should not contain read/write bits. Can be used to address
                    special addresses, for now; but this behavior may change.
                data -- The data to be sent to the given device.
        """

        if address > 127 or address < 0:
            raise ValueError("Tried to transmit to an invalid I2C address!")
        data = bytes(data)
        write_status = self.board.apis.i2c.write(address, data)
        
        return write_status

    def transmit(self, address, data, receive_length):
        """
            Wrapper function for back to back TX/RX.

            Args:
                address -- The 7-bit I2C address for the target device.
                    Should not contain read/write bits. Can be used to address
                    special addresses, for now; but this behavior may change.
                data -- The data to be sent to the given device.
                receive_length -- The I2C controller will attempt
                        to read the provided amount of data, in bytes.
        """

        write_status = self.write(address, data)
        read_data, read_status = self.read(address, receive_length)

        return read_data, write_status, read_status

    def scan(self):
        """
            TX/RX over the I2C bus, and recieves ACK/NAK
            in response for valid/invalid addresses.
        """

        responses = self.board.apis.i2c.scan()
        write_responses = responses[:16]
        read_responses = responses[16:]
        responses = []

        for write_response, read_response in zip(write_responses, read_responses):
            for x in range(8):
                responses.append(
                    (write_response & 1 << x != 0,
                     read_response & 1 << x != 0))

        return responses

