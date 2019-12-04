#
# This file is part of GreatFET
#

from __future__ import print_function

import sys

from warnings import warn
from ..interface import GreatFETInterface

from ..support.bits import bits
from ..protocol.jtag_svf import SVFParser, SVFEventHandler

class JTAGPatternError(IOError):
    """ Class for errors that come from a JTAG read not matching the expected response. """

    def __init__(self, message, result):
        self.result = result
        super(JTAGPatternError, self).__init__(message)


# FIXME: should this be an instance of a 'target' class?
class JTAGDevice(GreatFETInterface):
    """ Class representing a single device on a JTAG scan chain. """

    DESCRIPTION = "no description available"

    # A list of supported IDCODEs for the relevant class.
    # Used unless the supports_idcode() method is overridden.
    SUPPORTED_IDCODES = []

    # A list of any GreatFET subcommands that are useful for driving this target;
    # for informational use.
    SUPPORTED_CONSOLE_COMMANDS = []


    @classmethod
    def from_idcode(cls, idcode, position_in_chain=0):
        """ Attempts to create a JTAGDevice object that fits the provided IDCODE. """

        # Assume the generic device class is the most appropriate class for the device, initially.
        most_appropriate_class = cls

        # Search each imported subclass for the
        for subclass in cls.__subclasses__():
            if subclass.supports_idcode(idcode):
                most_appropriate_class = subclass
                break


        # Finally, create an instance of the most appropriate class for this object.
        instance = object.__new__(most_appropriate_class)
        most_appropriate_class.__init__(instance, idcode, position_in_chain)

        return instance


    @classmethod
    def supports_idcode(cls, idcode):
        """
        Returns true iff this class supports the given IDCODE.

        This default implementation uses SUPPORTED_IDCODES, but subclasses can override this
        for more nuanced behavior.
        """
        return idcode in cls.SUPPORTED_IDCODES


    @classmethod
    def supported_console_commands(cls):
        """ Returns a list of GreatFET subcommands that provide access to the given class. """
        return cls.SUPPORTED_CONSOLE_COMMANDS


    def idcode(self):
        """ Returns this device's IDCODE. """
        return self._idcode


    def description(self):
        """ Returns a short description of the device. """
        return self.DESCRIPTION


    def __init__(self, idcode, position_in_chain):
        self._idcode = idcode



class JTAGChain(GreatFETInterface):
    """ Class representing a JTAG scan-chain interface. """

    # Short name for this type of interface.
    INTERFACE_SHORT_NAME = "jtag"

    #
    # Simple mapping that captures the various TAP FSM states.
    # Names from the JTAG SVF specification are used directly, so we can easily parse SVF files.
    #
    STATE_PROGRESSIONS = {
        'RESET':     {0: 'IDLE',  1: 'RESET'        },
        'IDLE':      {0: 'IDLE',  1: 'DRSELECT'     },

        # Data register path.
        'DRSELECT':  {0: 'DRCAPTURE', 1: 'IRSELECT' },
        'DRCAPTURE': {0: 'DRSHIFT',   1: 'DREXIT1'  },
        'DRSHIFT':   {0: 'SHIFT_DR',  1: 'DREXIT1'  },
        'DREXIT1':   {0: 'DRPAUSE',   1: 'DRUPDATE' },
        'DRPAUSE':   {0: 'DRPAUSE',   1: 'DREXIT2'  },
        'DREXIT2':   {0: 'DRSHIFT',   1: 'DRUPDATE' },
        'DRUPDATE':  {0: 'IDLE',      1: 'DRSELECT' },

        # Instruction register path.
        'IRSELECT':  {0: 'IRCAPTURE', 1: 'RESET'    },
        'IRCAPTURE': {0: 'IRSHIFT',   1: 'IREXIT1'  },
        'IRSHIFT':   {0: 'IRSHIFT',   1: 'IREXIT1'  },
        'IREXIT1':   {0: 'IRPAUSE',   1: 'IRUPDATE' },
        'IRPAUSE':   {0: 'IRPAUSE',   1: 'IREXIT2'  },
        'IREXIT2':   {0: 'IRSHIFT',   1: 'IRUPDATE' },
        'IRUPDATE':  {0: 'IDLE',      1: 'DRSELECT' },
    }


    def __init__(self, board, max_frequency=405e3):
        """ Creates a new JTAG scan-chain interface.

        Paramters:
            board         -- the GreatFET board we're working with.
            max_frequency -- the maximum frequency we should attempt scan out data with
        """

        # Grab our JTAG API object.
        self.api = board.apis.jtag

        # Assume we're starting our chain in 'IDLE'.
        self.state = 'IDLE'

        # Configure our chain to run at the relevant frequency.
        self.frequency = int(max_frequency)
        self.max_bits_per_scan = self.api.configure(self.frequency)


    def set_frequency(self, max_frequency):
        """ Sets the operating frequency of future transactions on this JTAG chain. """

        self.frequency = int(max_frequency)
        self.api.configure(self.frequency)



    def _progress_state(self, tms_value):
        """ Adjusts our internal model of the TAP FSM to account for an applied TMS value. """

        # Normalize our state to always be 1 or 0.
        tms_value = 1 if tms_value else 0

        # Move our state to the next state per our TAP FSM.
        self.state = self.STATE_PROGRESSIONS[self.state][tms_value]


    def pulse_tms(self, cycles=1, asserted=True):
        """ Asserts or de-asserts TMS for the given number of cycles; used for navigating the TAP FSM. """

        # Run the clock for a single cycle, with TMS asserted each time.
        for _ in range(cycles):
            self.api.run_clock(1, asserted)
            self._progress_state(asserted)


    def initialize_chain(self):
        """ Put the scan chain into its initial state, allowing fresh JTAG communications. """

        # Pulse the TMS line five times -- this brings us into the TEST_RESET state, which resets the test logic.
        self.pulse_tms(5)

        # We now should know that we're in the RESET state.
        assert(self.state == 'RESET')


    def _receive_data(self, bits_to_scan, advance_state=False):
        """ Performs a raw scan-in of data, and returns the result. """

        # Perform our actual data scan-in.
        # TODO: break larger-than-maximum transactions into smaller ones.
        result = self.api.scan_in(bits_to_scan, advance_state)

        # Once we're complete, advance our state, if necessary.
        if advance_state:
            self._progress_state(True)

        return result


    def _pad_data_to_length(self, length_in_bits, data=None):
        """ Pads a given data set to a given length, in bits. """

        # Compute how many bytes we need the data to be.
        target_length_bytes = (length_in_bits + 7) // 8

        # If our data doesn't need padding, return it directly.
        if data and (len(data) >= target_length_bytes):
            return data

        # Create a mutable array of data; and add any data we have.
        padded = bytearray()
        if data:
            padded.extend(data)

        # Figure out how much padding we need.
        padding_necessary = target_length_bytes - len(padded)
        padded.extend("b\0" * padding_necessary)

        # Return our padded data.
        return padded


    def _transmit_data(self, bits_to_scan, data=None, advance_state=False):
        """ Performs a raw scan-out of data, discarding any result. """

        # Pad our data to the relevant length.
        data = self._pad_data_to_length(bits_to_scan)

        # Perform our actual data scan-in.
        # TODO: break larger-than-maximum transactions into smaller ones.
        self.api.scan_out(bits_to_scan, advance_state, data)

        # Once we're complete, advance our state, if necessary.
        if advance_state:
            self._progress_state(True)


    def _scan_data(self, bits_to_scan, byte_data, advance_state=False):
        """ Performs a raw scan-in of data, and returns the result. """

        # Perform our actual data scan-in.
        # TODO: break larger-than-maximum transactions into smaller ones.
        result = self.api.scan(bits_to_scan, advance_state, byte_data)

        # Once we're complete, advance our state, if necessary.
        if advance_state:
            self._progress_state(True)

        return result


    def _next_hop_towards(self, state):
        """ Identify the next TMS value we should apply to move towards the given state. """

        # Special case: if we're headed to RESET, then our next hop is always 1.
        if state == 'RESET':
            return 1

        # Special case: if we're in the Select-DR state, we'll steer either towards the instruction column ('1')
        # or data column ('0') based on the target state.
        if self.state == 'DRSELECT':
            return 1 if 'IR' in state else 0


        # Grab the next states for TMS values of one and zero.
        next_states = self.STATE_PROGRESSIONS[self.state]

        # We'll apply a simple heuristic to advance through the TAP FSM.
        # First, we'll identify it providing a '1' would cause us to loop back towards the current state,
        # which will occur if we'd stay in the same state with a '1', or if we'd move out of the core FSM.
        towards_one_would_loop = (next_states[1] == self.state) or (next_states[1] == 'RESET')

        # Next, we'll apply the following simple heuristics:
        #  - If pulsing clock with TMS=0 would land us in the right state, do so.
        #  - If pulsing clock with TMS=1 would cause us to self, loop, pulse clock with TMS=0.
        #  - Otherwise, pulse clock with TMS=1, as TMS=1 generally moves us through the TAP FSM.
        target_state_is_towards_zero = (next_states[0] == state)

        return 0 if (target_state_is_towards_zero or towards_one_would_loop) else 1


    def _ensure_in_state(self, state):
        """
        Ensures the JTAG TAP FSM is in the given state.
        If we're not; progresses the TAP FSM by pulsing TMS until we reach the relevant state.
        """

        # Progress through the TAP FSM until we're in the right state.
        while self.state != state:

            # Identify the direction we'll need to move in order to move closer to our target state...
            next_hop = self._next_hop_towards(state)

            # ... and apply it.
            self.pulse_tms(asserted=next_hop)


    def move_to_state(self, state_name):
        """ Moves the JTAG scan chain to the relevant state.

        Parameters:
            state_name: The target state to wind up in, as a string. States are accepted in the format
            defined in the JTAG SVF standard, and thus should be one of:
                "RESET", "IDLE", "DRSELECT", "DRCAPTURE", "DRSHIFT", "DREXIT1", "DRPAUSE",
                "DREXIT2", "DRUPDATE", "IRSELECT", "IRCAPTURE", "IRSHIFT", "IREXIT1", "IRPAUSE",
                "IREXIT2", "IRUPDATE"
        """
        self._ensure_in_state(state_name.strip())


    def _shift_while_in_state(self, state, tdi=None, length=None, ignore_response=False, advance_state=False, byteorder='big'):
        """ Shifts data through the chain while in the given state. """

        # Normalize our data into a bitstring type that we can easily work with.
        # This both ensures we have a known format; and implicitly handles things like padding.
        if tdi:
            data_bits = bits(tdi, length, byteorder=byteorder)

            # Convert from our raw data to the format we'll need to send down to the device.
            bit_length = len(data_bits)
            data_bytes = data_bits.to_bytes(byteorder='big')
        else:
            if length is None:
                raise ValueError("either TDI or length must be provided!")

            bit_length = length

        # Move into our shift-DR state.
        self._ensure_in_state(state)

        # Finally, issue the transaction itself.
        if tdi and ignore_response:
            self._transmit_data(bit_length, data_bytes, advance_state)
            return None
        elif tdi:
            result = self._scan_data(bit_length, data_bytes, advance_state)
        else:
            result = self._receive_data(bit_length, advance_state)

        # Return our data, converted back up to bits.
        return bits(result, bit_length)


    def _validate_response(self, response_bits, tdo=None, mask=None):
        """ Validates the response provided by a _shift_while_in_state call, in the traditional JTAG SVF form. """

        # If we don't have any data to validate against, vacuously succeed.
        if (not tdo) or (not response_bits):
            return

        # If we have a mask, mask both the TDO value and response, and then compare.
        masked_response = mask & response_bits if mask else response_bits
        masked_tdo      = mask & tdo if mask else tdo

        if masked_response != masked_tdo:
            raise JTAGPatternError("Scan result did not match expected pattern: {} != {} (expected)!".format(
                    masked_response, masked_tdo), response_bits)


    def shift_data(self, tdi=None, length=None, tdo=None, mask=None,
            ignore_response=False, advance_state=False, byteorder='big'):
        """ Shifts data through the scan-chain's data register.

        Parameters:
            tdi    -- The bits to be scanned out via TDI. Can be a support.bits() object, a string of 1's and 0's,
                      an integer, or bytes. If this is an integer or bytes object, the length argument must be provided.
                      If omitted or None, a string of all zeroes will be used,
            length -- The length of the transaction to be performed, in bits. This can be longer than the TDI data;
                      in which case the transmission will be padded with zeroes.
            tdo    -- The expected data to be received from the scan operation. If this is provided, the read result
                      will be compared to this data (optionally masked by mask), and an exception will be thrown if
                      the data doesn't match this value. Designed to behave like the SVF TDO field.
            mask   -- If provided, the given tdo argument will be masked, such that only bits corresponding to a '1'
                      in this mask argument are considered when checking against 'tdo'. This is the behavior defiend
                      in the SVF standard; see it for more information.
            ignore_response -- If provided; the returned response will always be empty, and tdo and mask will be ignored.
                               This allows for slight a performance optimization, as we don't have to shuttle data back.
            byteorder       -- The byteorder to consider the tdi value in; if bytes are provided.

        Returns the bits read, or None if the response is ignored.
        """

        # Perform the core shift, and gather the response.
        response = self._shift_while_in_state('DRSHIFT', tdi=tdi, length=length, ignore_response=ignore_response,
                advance_state=advance_state, byteorder=byteorder)

        # Validate our response against any provided constraints.
        self._validate_response(response, tdo=tdo, mask=mask)
        return response


    def shift_instruction(self, tdi=None, length=None, tdo=None, mask=None,
            ignore_response=False, advance_state=False, byteorder='big'):
        """ Shifts data through the chain's instruction register.

        Parameters:
            tdi    -- The bits to be scanned out via TDI. Can be a support.bits() object, a string of 1's and 0's,
                      an integer, or bytes. If this is an integer or bytes object, the length argument must be provided.
                      If omitted or None, a string of all zeroes will be used,
            length -- The length of the transaction to be performed, in bits. This can be longer than the TDI data;
                      in which case the transmission will be padded with zeroes.
            tdo    -- The expected data to be received from the scan operation. If this is provided, the read result
                      will be compared to this data (optionally masked by mask), and an exception will be thrown if
                      the data doesn't match this value. Designed to behave like the SVF TDO field.
            mask   -- If provided, the given tdo argument will be masked, such that only bits corresponding to a '1'
                      in this mask argument are considered when checking against 'tdo'. This is the behavior defiend
                      in the SVF standard; see it for more information.
            ignore_response -- If provided; the returned response will always be empty, and tdo and mask will be ignored.
                               This allows for slight a performance optimization, as we don't have to shuttle data back.
            byteorder       -- The byteorder to consider the tdi value in; if bytes are provided.

        Returns the bits read, or None if the response is ignored.
        """

        # Perform the core shift, and gather the response.
        response = self._shift_while_in_state('IRSHIFT', tdi=tdi, length=length, ignore_response=ignore_response,
                advance_state=advance_state, byteorder=byteorder)

        # Validate our response against any provided constraints.
        self._validate_response(response, tdo=tdo, mask=mask)
        return response


    def run_test(self, cycles, from_state='IDLE', end_state=None):
        """ Places the device into the RUNTEST/IDLE (or provided) state, and pulses the JTAG clock.

        Paraameters:
            cycles -- The number of cycles for which the device should remain in the given state.
            from_state -- The state in which the cycles should be spent; defaults to IDLE.
            end_state -- The state in which the device should be placed after the test is complete.
        """

        if from_state:
            self.move_to_state(from_state)

        self.api.run_clock(cycles, False, timeout=0)

        if from_state:
            self.move_to_state(end_state)


    def _create_device_for_idcode(self, idcode, position_in_chain):
        """ Creates a JTAGDevice object for the relevant idcode. """

        return JTAGDevice.from_idcode(idcode, position_in_chain)


    def enumerate(self, return_idcodes=False):
        """ Initializes the JTAG TAP FSM, and attempts to identify all connected devices.

        Parameters:
            return_idcodes -- If true, this method will return a list of IDCodes rather than JTAGDevice objects.

        Returns a list of JTAGDevices (return_idcodes=False) or JTAG IDCODES (return_idcodes=True).
        """

        devices = []

        # Place the JTAG TAP FSM into its initial state, so we can perform enumeration.
        self.initialize_chain()

        # Resetting the TAP FSM also automatically loaded the instruction register with the IDCODE
        # instruction, and accordingly filled the chain of data registers with each device's IDCODE.
        # We can accordingly just scan out the data using shift_data.

        # Once we (re-)initialize the chain, each device automatically loads the IDCODE instruction
        # for execution. This means that if we just scan in data, we'll receive each device's IDCODE,
        # followed by a null terminator (32 bits of zeroes).
        position_in_chain = 0
        while True:

            # Attempt to read a 32-bit IDCODE from the device.
            raw_idcode = self.shift_data(length=32)
            idcode = int.from_bytes(raw_idcode, byteorder='little')

            # If our IDCODE is all 1's, and we have no devices, we seem to be stuck at one.
            # Warn the user.
            if idcode == 0xFFFFFFFF and not devices:
                warn("TDI appears to be stuck at '1'. Check your wiring?")

            # If we've received our null IDCODE, we've finished enumerating the chain.
            # We'll also treat an all-1's IDCODE as a terminator, as this invalid IDCODE occurs
            # if TDI is stuck-at-one.
            if idcode in (0x00000000, 0xFFFFFFFF):
                self.pulse_tms(asserted=True)
                break

            if return_idcodes:
                devices.append(idcode)
            else:
                devices.append(self._create_device_for_idcode(idcode, position_in_chain))

            position_in_chain += 1

        return devices


    def play_svf_instructions(self, svf_string, log_function=None, error_log_function=print):
        """ Executes a string of JTAG SVF instructions, strumming the relevant scan chain.

        svf_string   -- A string containing valid JTAG SVF instructions to be executed.
        log_function -- If provided, this function will be called with verbose operation information.
        log_error    -- This function will be used to print information about errors that occur.
        """

        # Create the parser that will run our SVF file, and run our SVF.
        parser = SVFParser(svf_string, GreatfetSVFEventHandler(self, log_function, error_log_function))
        parser.parse_file()


    def play_svf_file(self, svf_file, log_function=None, error_log_function=print):
        """ Executes the JTAG SVF instructions from the given file.

        svf_file     -- A filename or file object pointing to a JTAG SVF file.
        log_function -- If provided, this function will be called with verbose operation information.
        log_error    -- This function will be used to print information about errors that occur.
        """

        close_after = False

        if isinstance(svf_file, str):
            svf_file = open(svf_file, 'r')
            close_after = True

        self.play_svf_instructions(svf_file.read(), log_function=log_function, error_log_function=error_log_function)

        if close_after:
            svf_file.close()



class GreatfetSVFEventHandler(SVFEventHandler):
    """ SVF event handler that delegates handling of SVF instructions to a GreatFET JTAG interface. """


    def __init__(self, interface, verbose_log_function=None, error_log_function=print):
        """ Creates a new SVF event handler.

        Parameters:
            interface: The GreatFET JTAG interface that will execute our JTAG commands.
        """

        if verbose_log_function is None:
            verbose_log_function = lambda string : None
        if error_log_function is None:
            error_log_function = print

        self.interface = interface
        self.log = verbose_log_function
        self.log_error = error_log_function

        # Assume that after a data / instruction shift operation that we'll
        # wind up in the IDLE state, per the SVF standard. The SVF file can
        # override these defaults
        self.end_dr_state = 'IDLE'
        self.end_ir_state = 'IDLE'

        # By default, don't have any headers or trailers for IR or DR shifts.
        # The SVF can override these using the HDR/TDR/HIR/TIR instructions.
        nullary_padding = {'tdi': bits(), 'tdo': bits(), 'mask': bits(), }
        self.dr_header  = nullary_padding.copy()
        self.dr_trailer = nullary_padding.copy()
        self.ir_header  = nullary_padding.copy()
        self.ir_trailer = nullary_padding.copy()

        # Store default masks for our ShiftIR and ShiftDR instructions.
        self.last_dr_mask  = None
        self.last_dr_smask = None
        self.ir_mask  = None
        self.ir_smask = None


    def svf_frequency(self, frequency):
        """Called when the ``FREQUENCY`` command is encountered."""
        self.log (" -- FREQUENCY set to {}".format(frequency))
        self.interface.set_frequency(frequency)


    def svf_trst(self, mode):
        """Called when the ``TRST`` command is encountered."""
        warn('SVF provided TRST command; but this implementation does not yet support driving the TRST line')


    def svf_state(self, state, path):
        """Called when the ``STATE`` command is encountered."""

        # Visit each state in any intermediate paths provided...
        if path:
            for intermediate in path:
                self.log("STATE; Moving through {}.".format(intermediate))
                self.interface.move_to_state(intermediate)

        # ... ensuring we end up in the relevant state.
        self.log("Moving to {} STATE.".format(state))
        self.interface.move_to_state(state)


    def svf_endir(self, state):
        """Called when the ``ENDIR`` command is encountered."""
        self.log("Moving to {} after each Shift-IR.".format(state))
        self.end_dr_state = state


    def svf_enddr(self, state):
        """Called when the ``ENDDR`` command is encountered."""
        self.log("Moving to {} after each Shift-DR.".format(state))
        self.end_ir_state = state


    def svf_hir(self, **header):
        """Called when the ``HIR`` command is encountered."""
        self.log("Applying Shift-IR prefix. ")
        self.ir_header = header


    def svf_tir(self, **trailer):
        self.log("Applying Shift-IR suffix. ")
        self.ir_trailer = trailer


    def svf_hdr(self, **header):
        """Called when the ``HDR`` command is encountered."""
        self.log("Applying Shift-DR header. ")
        self.dr_header = header


    def svf_tdr(self, **trailer):
        """Called when the ``TDR`` command is encountered."""
        self.log("Applying Shift-DR suffix. ")
        self.dr_trailer = trailer


    def svf_sir(self, **data):
        """Called when the ``SIR`` command is encountered."""

        # Append our header and trailer to each of our arguments.
        arguments = {}
        for arg, value in data.items():
            header  = self.ir_header[arg] if (arg in self.ir_header) else bits()
            trailer = self.ir_trailer[arg] if (arg in self.ir_trailer) else bits()
            arguments[arg] = (header + value + trailer) if value else None

        if data['mask']:
            self.ir_mask = data['mask']
        if data['smask']:
            self.ir_smask = data['mask']

        self.log("Performing SHIFT-IR:")
        self.log(   "out:      {}".format(arguments['tdi']))
        self.log(   "expected: {}".format(arguments['tdo']))
        self.log(   "mask:     {}".format(arguments['tdo']))
        try:
            result = self.interface.shift_instruction(tdi=arguments['tdi'], tdo=arguments['tdo'], mask=arguments['mask'])
        except JTAGPatternError as e:
            self.log(   "in:       {} [FAIL]\n".format(e.result))
            self.log_error("\n\n<!> Failure while performing SHIFT-IR: \n    " + str(e))
            raise

        self.log(   "in:       {} [OK]\n".format(result))


    def svf_sdr(self, **data):
        """Called when the ``SDR`` command is encountered."""

        # Append our header and trailer to each of our arguments.
        arguments = {}
        for arg, value in data.items():
            header  = self.dr_header[arg] if (arg in self.dr_header) else bits()
            trailer = self.dr_trailer[arg] if (arg in self.dr_trailer) else bits()
            arguments[arg] = (header + value + trailer) if value else None

        if data['mask']:
            self.dr_mask = data['mask']
        if data['smask']:
            self.dr_smask = data['mask']

        self.log("Performing SHIFT-DR:")
        self.log(   "out:      {}".format(arguments['tdi']))
        self.log(   "expected: {}".format(arguments['tdo']))
        self.log(   "mask:     {}".format(arguments['tdo']))
        try:
            result = self.interface.shift_data(tdi=arguments['tdi'], tdo=arguments['tdo'], mask=arguments['mask'])
        except JTAGPatternError as e:
            self.log(   "in:       {} [FAIL]\n".format(e.result))
            self.log_error("\n\n<!> Failure while performing SHIFT-DR: \n    " + str(e))
            raise
        self.log(   "in:       {} [OK]\n".format(result))


    def svf_runtest(self, run_state, run_count, run_clock, min_time, max_time, end_state):
        """Called when the ``RUNTEST`` command is encountered."""
        self.log("Running test for {} cycles.".format(run_count))
        self.interface.run_test(run_count, from_state=run_state, end_state=end_state)


    def svf_piomap(self, mapping):
        """Called when the ``PIOMAP`` command is encountered."""
        raise NotImplementedError("This implementation does not yet support PIOMAP.")

    def svf_pio(self, vector):
        """Called when the ``PIO`` command is encountered."""
        raise NotImplementedError("This implementation does not yet support PIO.")
