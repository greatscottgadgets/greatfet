#
# This file is part of GreatFET
#

from ..interface import PirateCompatibleInterface


class I2CBus(PirateCompatibleInterface):
    """
        Class representing a GreatFET I2C bus.

        For now, supports only the primary I2C bus (I2C0), but will be
        expanded when the vendor commands are.
    """

    # Short name for this type of interface.
    INTERFACE_SHORT_NAME = "i2c"

    def __init__(self, board, name='i2c bus', buffer_size=255, duty_cycle_count=255):
        """
            Initialize a new I2C bus.

            Args:
                board -- The GreatFET board whose I2C bus we want to control.
                name -- The display name for the given I2C bus.
                buffer_size -- The size of the I2C receive buffer on the GreatFET.
        """

        # Store a reference to the parent board, and our API.
        self.board = board
        self.api = board.apis.i2c

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

        return self.api.read(address, receive_length)


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
        write_status = self.api.write(address, data)

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

        self.write(address, data)
        return self.read(address, receive_length)


    def scan(self):
        """
            TX/RX over the I2C bus, and receives ACK/NAK
            in response for valid/invalid addresses.
        """

        responses = self.api.scan()
        write_responses = responses[:16]
        read_responses = responses[16:]
        responses = []

        for write_response, read_response in zip(write_responses, read_responses):
            for x in range(8):
                responses.append(
                    (write_response & 1 << x != 0,
                     read_response & 1 << x != 0))

        return responses


    #
    # Low-level methods that allow us to be used in bus-pirate mode.
    #

    def _handle_pirate_read(self, length, ends_transaction=False):
        """ Performs a bus-pirate read of the given length, and returns a list of numeric values. """

        response = []

        # Send down data in 256-byte chuns.
        CHUNK_SIZE = 256

        # Read our data.
        to_receive = length
        while to_receive:
            size_to_read = CHUNK_SIZE if (to_receive > CHUNK_SIZE) else to_receive
            to_receive -= size_to_read

            # If this exhausts our data, we want to end with a NAK, as this serves as an I2C "end-of-packet".
            end_with_nak = ends_transaction and (not to_receive)
            data = self.api.read_bytes(size_to_read, end_with_nak)
            response.extend(data)


        return response



    def _handle_pirate_write(self, data, ends_transaction=False):
        """ Performs a bus-pirate send of the relevant list of data, and returns a list of any data received. """

        # Send down data in 256-byte chuns.
        CHUNK_SIZE = 256

        # Transmit the bytes.
        to_transmit = data[:]

        while to_transmit:
            chunk_to_transmit = to_transmit[0:CHUNK_SIZE]
            del to_transmit[0:CHUNK_SIZE]

            self.api.issue_bytes(bytes(chunk_to_transmit))

        # I2C is half-duplex, so we don't provide any data in return.
        return []


    def _handle_pirate_start(self):
        """ Starts a given communication by performing any start conditions present on the interface. """
        self.api.issue_start()


    def _handle_pirate_stop(self):
        """ Starts a given communication by performing any start conditions present on the interface. """
        self.api.issue_stop()
