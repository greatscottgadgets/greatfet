
#
# This file is part of libgreat
#

from ..comms import CommsClass, command_rpc


class CoreAPI(CommsClass):
    """
    Class representing the libgreat core API, which all devices must support.
    """

    CLASS_NUMBER = 0
    CLASS_NAME = "core"

    VERB_DESCRIPTOR_OUT_SIGNATURE = 0
    VERB_DESCRIPTOR_IN_SIGNATURE = 1
    VERB_DESCRIPTOR_DOC = 2
    VERB_DESCRIPTOR_OUT_PARAM_NAMES = 3
    VERB_DESCRIPTOR_IN_PARAM_NAMES = 4

    # RPC that reads the board ID
    read_board_id = command_rpc(verb_number=0x0, out_format="<I", name="read_board_id", out_parameter_names=["id"])
    read_board_id.__doc__ = \
        """Fetches the board's type identifier.
           A type identifiers uniquely identifies what model of board this is. 
        """

    # RPC that reads the version
    read_version_string = command_rpc(verb_number = 0x1, out_format="<S", 
            name="read_version_string", out_parameter_names=["version"], doc="Fetches the board's version.")

    # RPC that reads the part ID
    read_part_id = command_rpc(verb_number=0x2, out_format="<2I", name="read_part_id", out_parameter_names=["part_id"])
    read_part_id.__doc__ = \
        """Fetches the part ID used on the board.
        
        Returns:
                A byte-string containing a code that describes which part is used on the board.
        """

    # RPC that fetches the serial number
    read_serial_number = command_rpc(verb_number=0x3, out_format="<4I",
            name="read_serial_number", out_parameter_names=["serial_number"])
    read_serial_number.__doc__ = \
        """Fetches the board's serial number.
        
        Returns:
            A 4-tuple of uint32's that compose the board's unique serial number.
        """

    # Introspection API.
    get_available_classes = command_rpc(verb_number=0x4, out_format="<*I",
            name= "get_available_classes", out_parameter_names=["numbers"], doc="Fetches the available class numbers.")
    get_available_verbs = command_rpc(verb_number=0x5, in_format="<I", out_format="<*I", name= "get_available_verbs", 
            in_parameter_names=["class_number"], out_parameter_names=["numbers"], doc="Fetches the available verb numbers for a given class.")
    get_verb_name = command_rpc(verb_number=0x6, in_format="<II", out_format="<S",
            name= "get_verb_name", out_parameter_names=["name"], doc="Fetches the string name for the given verb.")
    get_verb_descriptor = command_rpc(verb_number=0x7, in_format="<IIB", out_format="<S",
            name= "get_verb_descriptor", out_parameter_names=["descriptor"], doc="Fetches the information about the given verb.") #FIXME: expand docstring
    get_class_name = command_rpc(verb_number=0x8, in_format="<I", out_format="<S",
            name= "get_class_name", out_parameter_names=["name"], doc="Fetches the string name for the given class.")
    get_class_docs = command_rpc(verb_number=0x9, in_format="<I", out_format="<S",
            name= "get_class_docs", out_parameter_names=["docstring"], doc="Fetches for documentation the given class.")

    def get_verb_in_signature(self, class_number, verb_number):
        """ Fetches the given verb's in-signature. """
        return self.get_verb_descriptor(class_number, verb_number, self.VERB_DESCRIPTOR_IN_SIGNATURE)

    def get_verb_out_signature(self, class_number, verb_number):
        """ Fetches the given verb's out-signature. """
        return self.get_verb_descriptor(class_number, verb_number, self.VERB_DESCRIPTOR_OUT_SIGNATURE)

    def get_verb_documentation(self, class_number, verb_number):
        """ Fetches the given verb's documentation. """
        return self.get_verb_descriptor(class_number, verb_number, self.VERB_DESCRIPTOR_DOC)

    def get_verb_in_param_names(self, class_number, verb_number):
        """ Fetches the given verb's in-param names. """
        return self.get_verb_descriptor(class_number, verb_number, self.VERB_DESCRIPTOR_IN_PARAM_NAMES)

    def get_verb_out_param_names(self, class_number, verb_number):
        """ Fetches the given verb's out-param names. """
        return self.get_verb_descriptor(class_number, verb_number, self.VERB_DESCRIPTOR_OUT_PARAM_NAMES)

    # TODO : move debug into this

    # FIXME: re-assign verb number or move out of core?
    request_reset = command_rpc(verb_number=0x20, in_format="<I", 
            name="request_reset", doc="Resets the relevant board.")
