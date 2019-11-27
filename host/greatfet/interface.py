#
# This file is part of GreatFET
#

import time

class GreatFETInterface(object):
    """
    Generic base class for GreatFET peripherals.
    """


    def __init__(self, device):
        """ Default peripheral initializer -- just stores a reference to the relevant GreatFET. """

        self.device = device



class PirateCompatibleInterface(GreatFETInterface):
    """ Mix-in for interfaces that support bus-pirate style commands. """


    def run_pirate_commands(self, command_string):
        """ Runs a bus-pirate style command on the current interface. """

        # Command characters for Bus Pirate commands.
        _START_CHARS = "[{"
        _STOP_CHARS  = "]}"
        _READ_CHARS = "Rr"
        _DELAY_CHARS = "&"
        _DELIMITERS = " ,"

        # Numeric definitions for buspirate.
        _CHARS_VALID_TO_START_NUMBER="0123456789"
        _CHARS_VALID_IN_NUMBER="0123456789abcdefxh"


        self._result = []
        self._commands = list(command_string)

        # Simple performance enhancement: we'll gather any consecutive reads/writes and issue
        # them as single commands to the GreatFET.
        self._pending_write_data = []
        self._pending_read_length = 0


        def issue_pending_writes(ends_transaction=False):
            """ Issues any writes pending; used when performing a non-write operation."""

            if not self._pending_write_data:
                return

            # Perform all of our pending writes.
            self._result.extend(self._handle_pirate_write(self._pending_write_data, ends_transaction=ends_transaction))
            self._pending_write_data = []


        def perform_pending_reads(ends_transaction=False):
            """ Issues any writes pending; used when performing a non-write operation."""

            # If we don't have any pending reads, don't do anything.
            if not self._pending_read_length:
                return

            # Perform all of our pending reads.
            self._result.extend(self._handle_pirate_read(self._pending_read_length, ends_transaction=ends_transaction))
            self._pending_read_length = 0


        def handle_pending_io(ends_transaction=False):
            """ Convenience method that handles any pending I/O."""
            issue_pending_writes(ends_transaction=ends_transaction)
            perform_pending_reads(ends_transaction=ends_transaction)



        def extract_number(char=None):
            """
            Extracts a number from the current command stream. Should only be called when the command stream
            starts with a number.
            """

            # Start building our number.
            number = []

            try:

                # If we don't have a starting character, read one.
                if char is None:
                    char = self._commands.pop(0)

                # Grab all characters from the string until we run out of numbers.
                while char in _CHARS_VALID_IN_NUMBER:

                    # Quirk: the bus pirate accepts 'h' as a synonym for 'x' in number prefixes.
                    # We'll convert it to 'x' to match python's prefix format.
                    char = 'x' if (char == 'h') else char

                    number.append(char)
                    char = self._commands.pop(0)



            except IndexError:
                # If we've run out of characters to parse, this is a de-facto delimiter.
                # Convert it to one so we can handle the pending number below.
                char = ' '


            # If we don't have a number, return None.
            if len(number) == 0:
                return None


            # Once we no longer have a valid numeric character, parse the number we've built.
            number = ''.join(number)
            return int(number, 0)


        def get_repeat_count():
            """
            Checks to see if the given stream has a bus-pirate repeat operator (:<number>), and if so, returns it.
            If it doesn't, returns a default value of 1.

            """

            if len(self._commands) and self._commands[0] == ':':

                # Discard our colon...
                del self._commands[0]

                # ... and extract the relevant number.
                return extract_number()

            else:
                return 1


        # Handle each byte in the command string.
        while self._commands:

            # Start off with no repeat modifier, and no pending operation.
            length = None

            # Grab the next command-character in the queue.
            char = self._commands.pop(0)

            # If this character starts a number, we have a write operation.
            if char in _CHARS_VALID_TO_START_NUMBER:
                byte = extract_number(char)

                if byte > 255:
                    raise ValueError("Bus pirate commands must provide only byte values to write!")

                # Schedule the write.
                perform_pending_reads()
                self._pending_write_data.append(byte)

            # Handle our core commands.
            elif char in _START_CHARS:
                handle_pending_io()
                self._handle_pirate_start()

            elif char in _STOP_CHARS:
                handle_pending_io(ends_transaction=True)
                self._handle_pirate_stop()

            elif char in _READ_CHARS:
                issue_pending_writes()

                length = get_repeat_count()
                self._pending_read_length += length

            elif char in _DELAY_CHARS:
                handle_pending_io()

                # Compute the total length of time we want to delay...
                duration_us = get_repeat_count()
                time.sleep(duration_us / 1000000)

            elif char in _DELIMITERS:
                pass
            else:
                raise ValueError("Unsupported command character {}".format(char))

            # TODO: support 'A/a/@'?
            # TODO: support 'D/d'?
            # TODO: message on 'Ww'?

        return self._result


    #
    # Default (do-nothing) implementations of our support functions.
    # Subclasses typically should implement these.
    #


    def _handle_pirate_read(self, length, ends_transaction=False):
        """ Performs a bus-pirate read of the given length, and returns a list of numeric values. """
        return []


    def _handle_pirate_write(self, data, ends_transaction=False):
        """ Performs a bus-pirate send of the relevant list of data, and returns a list of any data received. """
        return []


    def _handle_pirate_start(self):
        """ Starts a given communication by performing any start conditions present on the interface. """
        pass


    def _handle_pirate_stop(self):
        """ Starts a given communication by performing any start conditions present on the interface. """
        pass
