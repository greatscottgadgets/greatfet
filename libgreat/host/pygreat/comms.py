#
# This file is part of libgreat
#

"""
Module containing the general definitions for communicating with
libgreat devices.
"""

from __future__ import unicode_literals

import re
import sys
import types
import struct
import inspect
import itertools
import collections

from future import utils as future_utils

class CommsBackend(object):
    """
    Class representing an abstract communications channel used to
    connect with a libgreat board.
    """

    """ The maximum input/output buffer size for libgreat commands. """
    LIBGREAT_MAX_COMMAND_SIZE = 4096

    """ Regular expression that identifies special fields for .pack and .unpack. """
    _SPECIAL_FIELD_REGEX = r"((?:[\d*]*[SX])|(?:\*\w)|(?:[\d*]*\([cbB?hHiIlLqQfdspPSX]+\)))"

    """ Regular expression that splits the parts of a .pack / .unpack format expression. """
    _FORMAT_FIELD_REGEX = r"([\d*]*)(?:([cbB?hHiIlLqQfdspPSX])|(?:\(([cbB?hHiIlLqQfdspPSX]+)\)))"

    """ Dictionary that describes nice names for each of the format fields. """
    _FORMAT_FIELD_ANNOTATION = {
        'x': 'padding',
        'X': 'bytes',
        'c': 'char',
        'b': 'int8',
        'B': 'uint8',
        '?': 'bool',
        'h': 'int16',
        'H': 'uint16',
        'i': 'int32',
        'I': 'uint32',
        'l': 'int32',
        'L': 'uint32',
        'q': 'int64',
        'Q': 'uint32',
        'f': 'float',
        'd': 'double',
        's': 'string',
        'S': 'string',
        'p': 'string',
        'P': 'int',
    }


    @classmethod
    def from_device_uri(cls, **device_uri):
        """ Creates a CommsBackend object apporpriate for the given device_uri.  """

        #FIXME: implment this properly

        from .comms_backends.usb import USBCommsBackend

        # TODO: handle providing board "URIs", like "usb://1234abcd/?param=value",
        # and automatic resolution to a backend?
        # TODO: support more than USB

        # XXX: for now, just create a USB backend, as that's all we support
        return USBCommsBackend(**device_uri)


    def __init__(self, **device_arguments):
        """ Sets up a new communication channel for a libgreat device.

        Args:
            device_arguments -- A dictionary of backend-specific arguments; as typically
                parsed from a device URI.
        """

        from pygreat.classes.core import CoreAPI

        # Create a dictionary that will store references to our low-level/raw board APIs.
        self.apis = collections.OrderedDict() 

        # Populate our core API, which is always present.
        self.apis['core'] = CoreAPI(self)


    def generate_api_object(self):
        """ Generates an object that gives us a view of all API methods
            accessible via communciations API. Identical to touching the .apis
            dictionary, but uses object semantics.
        """

        # Generate a class that wraps self.apis, so self.apis can be easily
        # accessed with object semantics.
        return CommsApiCollection(self.apis)


    def run_autoenumeration(self, overwrite=False):
        """ Uses the Core Introspection API to determine the accessible APIs
            and automatically genereate RPC stubs in the .api accessor.
        """

        # Fetch all of the available class numbers.
        class_numbers = self.apis['core'].get_available_classes()

        # And process each one.
        for class_number in class_numbers:
            self._generate_object_for_class(class_number)



    def _generate_object_for_class(self, class_number, overwrite=False):
        """ Uses the Core Introspection API to generate a python class containing
            RPCs for the provided libgreat class.

        Args:
            class_number -- The class number to generate RPCs for.
            overwrite -- If true, existing class definitions will be overwritten.
                Otherwise, any class that already has an object associated with it
                will be maintained.

        Returns:
            a dynamically-generated class object for the provieded class number, 
            filled with RPC methods
        """

        # Get the name / docstring for the given class.
        class_name = self.apis['core'].get_class_name(class_number)
        class_docs = self.apis['core'].get_class_docs(class_number)

        # Ensure that the class name is a string type that can be a class name.
        # This ensures python2 compatibility.
        class_name = future_utils.native_str(class_name)

        # If we already have an object for the given class,
        # and we're not in overwrite mode, skip enumerating it.
        if class_name in self.apis:
            if not overwrite:
                return

        # Get a set of RPC verbs for the given class, which will become our
        # class's methods.
        attrs = self._generate_rpc_verbs_for_class(class_number)

        # Each comms class needs a CLASS_NUMBER attribute, and should have a
        # CLASS_NAME attribute. We'll add ours.
        attrs['CLASS_NUMBER'] = class_number
        attrs['CLASS_NAME'] = class_name

        # Generat a documentation string for the given class.
        attrs['__doc__'] = \
                'Autogenerated class for the {} API class:\n{}'.format(class_name, class_docs)

        # Generate a class around the relevant verbs.
        cls = type(class_name, (GeneratedCommsClass,), attrs)

        # Finally, instantiate and store the relevant class.
        self.apis[class_name] = cls(self)

    @staticmethod
    def _parse_rpc_param_names_string(name_string):
        """ Parses a comma-separated command string into a list of names. """

        # Special case: if we got '*', the names are too complex for us.
        # Return None.
        if name_string == "*":
            return None

        # Break the name into its component parts.
        names = name_string.split(',')
        return [name.strip() for name in names]



    def _generate_rpc_verbs_for_class(self, class_number):
        """ Uses the Core Introspection API to generate RPCs for each of the verbs
            supported by a given class.
        Args:
            class_number -- The class number to generate RPCs for.

        Returns:
            a dictionary mapping verb names to yet-unbound method objects 
        """

        rpcs = {}
        core_api = self.apis['core']

        # Get the name for the given class.
        verb_numbers = core_api.get_available_verbs(class_number)

        # Iterate over each of the verbs and build an RPC for them.
        for verb_number in verb_numbers:
            
            # Fetch the information necessary to build the verb RPC.
            name            = core_api.get_verb_name(class_number, verb_number)
            in_signature    = core_api.get_verb_in_signature(class_number, verb_number)
            out_signature   = core_api.get_verb_out_signature(class_number, verb_number)
            documentation   = core_api.get_verb_documentation(class_number, verb_number)
            in_param_names  = core_api.get_verb_in_param_names(class_number, verb_number)
            out_param_names = core_api.get_verb_out_param_names(class_number, verb_number)

            # FIXME: automatically generate docs

            # If the arguments are too complex for our auto-generation, or no
            # signatures were provided, the device will respond with the special-case value '*'.
            # We'll skip these.
            if (in_signature == '*') or (out_signature == '*'):
                continue

            # Provide a nice disclaimer if the method is undocumented on the firmware side.
            if documentation == '*':
                documentation = "{undocumented on firmware side}"

            # Parse any parameter names the device knows about.
            in_param_names = self._parse_rpc_param_names_string(in_param_names)
            out_param_names = self._parse_rpc_param_names_string(out_param_names)

            # Build the relevnat RPCs.
            rpcs[name] = command_rpc(verb_number, in_signature, out_signature, name=name, 
                    doc=documentation, in_parameter_names=in_param_names, out_parameter_names=out_param_names)

        return rpcs


    @classmethod
    def _split_struct_string(cls, format_string):
        """ Splits a libgreat pack/unpack format string into groups
            that can be passed to a subordinate handler function.
        """

        # Split the pack string into a list of format string chunks to handle.
        strings_to_handle = re.split(cls._SPECIAL_FIELD_REGEX, format_string)

        # Return all of the non-empty sections.
        return [s for s in strings_to_handle if s]


    @classmethod
    def _is_special_format(cls, format_string):
        """ Returns true iff the given format string represents one of
            our extensions to struct.pack.
        """
        return re.match(cls._SPECIAL_FIELD_REGEX + "$", format_string)


    @classmethod
    def _format_string_argument_count(cls, format_string):
        """ Returns the number of arguments that will be consumed by
            the given format string. Format strings must not contain '*'.
        """

        total = 0

        # Get all of the non-padding struct arguments.
        elements = re.findall(cls._FORMAT_FIELD_REGEX, format_string)

        # Iterate over each of the matches, and update the count accordingly.
        for repeat_count, element_type, element_group in elements:

            # Special case: if this is an string or as-is specifier,
            # it should only consume a single argument no matter how many
            # are grabbed.
            will_conglomerate = element_type in "Xsp"

            # If we have a repeat specifier, parse it and include it.
            # Otherwise, this will consume only a single argument.
            if repeat_count and not will_conglomerate:
                total += int(count)
            else:
                total += 1

        return total


    @classmethod
    def _split_off_arguments_for_format(cls, format_string, arguments):
        """ Splits off the number of arguments necessary to handle the
            given format string.

            Returns:
                a 2-tuple of args_consumed, args_remaining
        """

        # Special case: if the format string starts with '*',
        # it consumes all remaining arguments.
        if format_string.startswith('*'):
            return arguments, []

        # Determine how many arguments we'll need to consume...
        args_to_consume = cls._format_string_argument_count(format_string)

        # ... and split them accordingly.
        args_consumed = arguments[0:args_to_consume]
        args_remaining = arguments[args_to_consume:]
        return args_consumed, args_remaining


    @classmethod
    def _get_bytes_consumed_by_group_format(cls, format_string, raw_bytes):
        """ Returns the number of bytes consumed by a given grouped
            format string. Accepts format strings that do not begin with '*'.

            Intended for use by _get_bytes_consumed_by_format.
        """

        total_bytes_consumed = 0

        # Break the group specifier into its components.
        element_matches = re.match(cls._FORMAT_FIELD_REGEX, format_string)
        repeat_count, element_type, element_group = element_matches.groups()

        # Figure out the number of times this subformat will repeat.
        if repeat_count is None:
            repeat_count = 1
        else:
            repeat_count = int(repeat_count)


        # Figure out how many bytes each subgroup will take.
        # Note that we can't simply request the length once and then
        # multiply out, as some format strings are dependent on the
        # data they're parsing.
        for _ in range(repeat_count):

            # Get the bytes consumed by this individual, non-group subformat.
            bytes_consumed = cls._get_bytes_consumed_by_format(element_group, raw_bytes)

            # And advance by the relevant number of bytes to repeat properly.
            raw_string = raw_string[bytes_consumed:]
            total_bytes_consumed += bytes_consumed

        return total_bytes_consumed





    @classmethod
    def _get_bytes_consumed_by_format(cls, format_string, raw_bytes):
        """ Returns the number of bytes consumed by a given
            format string.
        """

        # Special case: if the format string starts with a '*',
        # we take all of the remaining bytes.
        if format_string.startswith('*'):
            return len(raw_bytes)

        # Special case: if the format string represents a group,
        # we'll need to handle it recursively.
        if format_string.endswith(')'):
            return self._get_bytes_consumed_by_group_format(format_string, raw_bytes)

        # Special case: if the format string represents a string,
        # it'll take up to the first NULL.
        if format_string.endswith('S'):
            null_pos = raw_bytes.find(b"\0")

            # If we have no NULL, the whole string is to be consumed.
            if null_pos == -1:
                return len(raw_bytes)
            # Otherwise, it's the length with the NULL.
            else:
                return null_pos + 1


        # If this is a binary/as-is string, read the specifier
        # as its length.
        if format_string.endswith('X'):

            # Parse the format string, extracting the relevant fields.
            match = re.match(cls._FORMAT_STRING_REGEX, format_string)

            # If we didn't have a prefix, this is only a byte.
            if match[0] is None:
                return 1
            else:
                return int(match[0])


        # Sanity check: this string doesn't contain a string.
        if ('S' in format_string) or ('X' in format_string) or ('*' in format_string):
            raise ValueError("internal consistency error -- called with a " +
                             "type that shouldn't be possible!")

        # Otherwise, ask the format string module for how long this is,
        # as this is as plain section.
        return struct.calcsize(format_string)


    @classmethod
    def _split_off_bytes_for_format(cls, format_string, raw_bytes):
        """ Splits off the bytes necessary to handle the
            given format string.

            Returns:
                a 2-tuple of bytes_consumed, bytes_remaining
        """

        # Figure out how many bytes are consumed by the format...
        num_bytes_consumed = cls._get_bytes_consumed_by_format(format_string, raw_bytes)

        # ... and split the bytes off accordingly.
        bytes_consumed = raw_bytes[0:num_bytes_consumed]
        bytes_remaining = raw_bytes[num_bytes_consumed:]
        return bytes_consumed, bytes_remaining


    @classmethod
    def _pack_group(cls, format_string, args):
        """ Handles packing of a group subformat, which groups
            a pack string using parenthesis.
        """

        # Break the group specifier into its components.
        element_matches = re.match(cls._FORMAT_FIELD_REGEX, format_string)
        repeat_count, element_type, element_group = element_matches.groups()

        # If we have a repeat count, repeat this the appropriate number of times.
        if repeat_count:

            result = b""

            # If we're not repeating infinitely, validate our count.
            if repeat_count != '*':
                if int(repeat_count) != len(args):
                    raise ValueError("Unexpected number of repeated-groups provided!")

            # For each tuple of arguments in our arg-tuple,
            # repeat the parse.
            for arguments in args:
                result += cls.pack(element_group, *arguments)

            return result

        # Otherwise, this is just a single group; consume its arguments.
        else:
            return cls.pack(element_group, *args)


    @classmethod
    def _unpack_group(cls, format_string, raw_bytes):
        """ Handles packing of a group subformat, which groups
            a pack string using parenthesis.
        """

        import math

        # Break the group specifier into its components.
        element_matches = re.match(cls._FORMAT_FIELD_REGEX, format_string)
        repeat_count, element_type, element_group = element_matches.groups()

        # If we have a repeat count, repeat this the appropriate number of times.
        if repeat_count:

            result = []

            # If we're not repeating infinitely, parse is as an integer and
            # validate our count.
            if repeat_count != '*':
                repeat_count = int(repeat_count)
                if repeat_count != len(args):
                    raise ValueError("Unexpected number of repeated-groups provided!")
            # Otherwise, use a repeat_count of infinity.
            else:
                repeat_count = math.inf

            # While we have bytes left to parse, and we haven't consumed all
            # of our repeats, unpack each chunk of bytes.
            while raw_bytes and repeat_count:

                # Split off the bytes for the relevant subgroup.
                bytes_consumed, raw_bytes = \
                        cls._split_off_bytes_for_format(element_group, raw_bytes)

                # Parse the subgroup, and add it to our resultant list.
                result.append(cls.unpack(element_group, bytes_consumed))

                # And decrement our total repeat count.
                repeat_count -= 1


            return result

        # Otherwise, this is just a single group; consume its arguments.
        else:
            return cls.pack(element_group, *args)


    @classmethod
    def pack(cls, format_string, *args):
        """ Extended version of struct.pack() for libgreat communciations.

        Accepts mostly the same arguments as struct.pack, with the following
        additional supported format strings:

            S -- c-style, UTF-8, null-terminated string
            X -- as-is; returns a byte-string. If specified with a
                 repeat specifier, each of the bytes will be squished
                 together into a single string.

        Adds the following additional prefixes:

            * -- acts likes a repeat prefix, but consumes all remaining arguments
                 as though they were the given type; so the format "*B" would
                 pack all remaining arguments into uint8_ts

        Also accepts grouped arguments, which are grouped by parentheses,
        and accept indexable collections, like tuples. So, for example:

            (BH) -- would accept a tuple of a uint8 and a uint16

        Grouped arguments are especially useful with repeat count 'prefixes'.
        """

        # Break the format string into chunks we can handle.
        # These chunks can either be passed into struct.pack or into one
        # of our local packing functions.
        subformats = cls._split_struct_string(format_string)

        # Build a byte-string from each sub-format.
        result = b""
        for subformat in subformats:
            args_consumed, args = cls._split_off_arguments_for_format(subformat, args)

            # If this is one of our special formats, handle it.
            if cls._is_special_format(subformat):

                # If this is a group, recurse and handle the subformat.
                if subformat.endswith(')'):
                    result += cls._pack_group(subformat, args_consumed)
                # If this is a string subformat, handle it.
                elif subformat.endswith('S'):
                    result += c_string_arguments('UTF-8', *args_consumed)
                elif subformat.endswith('X'):
                    result += args_consumed[0]
                else:
                    result += int_array_arguments(subformat[-1], *args_consumed)


            # Otherwise, it's a standard pack format; pack it.
            else:
                result += struct.pack(subformat, *args_consumed)

        # Return the composed byte-string.
        return result


    @classmethod
    def unpack(cls, format_string, raw_bytes):
        """ Extended version of struct.unpack() for libgreat communciations.

        Accepts mostly the same arguments as struct.pack, with the following
        additional supported format strings:

            S -- c-style, UTF-8, null-terminated string
            X -- as-is; returns a byte-string. If specified with a
                 repeat specifier, each of the bytes will be squished
                 together into a single string.

        Adds the following additional prefixes:

            * -- acts likes a repeat prefix, but consumes all remaining arguments
                 as though they were the given type; so the format "*B" would
                 unpack all remaining bytes into uint8's

        Also accepts grouped arguments, which are grouped by parentheses,
        and unpack into tuples. So, for example:

            (BH) -- would parse three bytes into a tuple of a uint8 and a uint16

        Grouped arguments are especially useful with repeat count 'prefixes'.
        """

        # Break the format string into chunks we can handle.
        # These chunks can either be passed into struct.pack or into one
        # of our local packing functions.
        subformats = cls._split_struct_string(format_string)

        # Build a byte-string from each sub-format.
        results = []
        for subformat in subformats:
            bytes_consumed, raw_bytes = cls._split_off_bytes_for_format(subformat, raw_bytes)

            # If this is one of our special formats, handle it.
            if cls._is_special_format(subformat):

                # If this is a group, recurse and handle the subformat.
                if subformat.endswith(')'):
                    results.extend(cls._unpack_group(subformat, bytes_consumed))

                # If this is a string subformat, handle it.
                elif subformat.endswith('S'):
                    results.extend(c_string_return(bytes_consumed, 'UTF-8'))

                # If this is an as-is subformat, handle it.
                elif subformat.endswith('X'):
                    results.append(bytes_consumed)

                # Otherwise, it's an integer format.
                else:
                    results.extend(int_array_return(bytes_consumed, subformat[-1]))

            # Otherwise, it's a standard pack format; unpack it.
            else:
                results.extend(struct.unpack(subformat, bytes_consumed))

        # Return the generated tuple.
        return tuple(results)


    @classmethod
    def argument_annotations_for_format(cls, format_string):
        """ Returns a list of strings that describe each argument of a format.
        Intended to be used to generate argument annotations.

        Args:
            format_string -- The format to be described.

        Returns:
            a list of strings describing each argument
        """

        annotations = []

        # Get all of the non-padding struct arguments.
        elements = re.findall(cls._FORMAT_FIELD_REGEX, format_string)

        # Iterate over each of the matches.
        for repeat_count, element_type, element_group in elements:

            # Special case: if this is an string or as-is specifier,
            # it should only represent a single argument no matter how many
            # are grabbed.
            will_conglomerate = element_type in "Xsp"

            # If this is a group element, recurse.
            if element_group:
                subannotations = cls.argument_annotations_for_format(element_group)
                annotation = "(" + ', '.join(subannotations)  + ")"
            else:
                # Get the type of argument to add.
                annotation = cls._FORMAT_FIELD_ANNOTATION[element_type]

            # If we have a repeat specifier, tag this as an array.
            if repeat_count and not will_conglomerate:
                if repeat_count == "*":
                    annotations.append(annotation + "[]")
                else:
                    arguments_generated = int(repeat_count)
                    annotations.extend([annotation] * int(arguments_generated))
            else:
                annotations.append(annotation)

        return annotations



    def execute_command(self, class_number, verb, in_format, out_format,
            *arguments, **kwargs):
        """Executes a libgreat command.

        Args:
            class_number -- The class number for the given command.
                See the GreatFET wiki for a list of class numbers.
            verb -- The verb number for the given command.
                See the GreatFET wiki for the given class.

            in_format -- Specifies the input format of the arguments,
                in one of the formats specified below.,
            out_format -- Specifies the format in which the output argumnt
                will be parsed; in one of the formats specified below.
            *arguments -- The positional arguments to be passed to the libgreat
                device. These work as if passed to struct.pack.

            timeout -- Maximum command execution time, in ms.
            encoding -- If specified, any strings in response data will be
                decoded in the provided format.
            max_response_length -- If less than 4096, this parameter will
                cut off the provided response at the given length.

            name -- name of the command being executed; for error messages
                only

        The formats used by in_format and out_format can be as follows:
            - A format string in the format accepted by struct.pack;
            - A format string in the format accepted by CommsBackend.pack;
            - A callable object, which handles the conversion between tuple
              and byte payload itself.

        Returns any data recieved in response, parsed according to out_format.
        """

        # Emulate python3 keyword-only arguments.
        timeout  = kwargs.pop('timeout', 1000)
        encoding = kwargs.pop('encoding', None)
        max_response_length = kwargs.pop('max_response_length', 4096)
        comms_timeout = kwargs.pop('comms_timeout', 1000)
        name = kwargs.pop('name', "anonymous")

        if kwargs:
            raise TypeError("Unexpected keyword arguments: {}".format(kwargs.keys()))

        # Pack our input arguments into a payload.
        try:
            if callable(in_format):
                payload = in_format(*arguments)
            elif in_format:
                payload = self.pack(in_format, *arguments)
            else:
                payload = b""
        except Exception as e:
            # Wrap any exceptions that occur with a more specific method.
            message = "invalid arguments in call to RPC `{}`; innner message: {}; format: {}".format(name, e, in_format)
            outer_exception = type(e)(message)
            future_utils.raise_with_traceback(outer_exception, sys.exc_info()[2])

        # Execute the command.
        raw_result = self.execute_raw_command(class_number, verb, payload, timeout,
                None, max_response_length, comms_timeout).tostring()

        # Unpack our response into a tuple of result values.
        try:
            if callable(out_format):
                result = out_format(raw_result)
            elif out_format:
                result = self.unpack(out_format, raw_result)
            else:
                result = tuple()
        except Exception as e:
            # Wrap any exceptions that occur with a more specific method.
            message = "unexpected return RPC `{}`; innner message: {}; format: {}".format(name, e, out_format)
            outer_exception = type(e)(message)
            future_utils.raise_with_traceback(outer_exception, sys.exc_info()[2])

        # If we have an encoding, convert any byte arguments
        # into a string.
        if encoding:

            # Tuples are immutable, so we'll temporarily convert
            # the relevant result to an integer.
            result = list(result)

            for index, value in enumerate(result):
                if isinstance(value, bytes):
                    result[index] = value.decode(encoding)

            result = tuple(result)

        # Return the result in a format that makes it easily
        # intepreted as multiple return values. In most cases, a tuple is returned
        # when multiple values are possible; and a single value is returned if only
        # one value is possible.
        if self._command_results_should_collapse(result, out_format):
            return result[0]
        else:
            return result


    @classmethod
    def _command_results_should_collapse(cls, result, out_format):
        """ Returns true if the given command result should be collapsed
            into a single value.
        """

        non_padding_formats = 0

        # Never collapse non-strings, for now.
        if not isinstance(out_format, str):
            return False

        # Split the format string into its components.
        elements = re.findall(cls._FORMAT_FIELD_REGEX, out_format)
        for repeat_count, element_type, element_group in elements:

            # Certain formats conglomerate their outputs into a single
            # argument, and thus have repeat types that don't count.
            will_conglomerate = element_type in "Xsp"

            # If we have any repeat counts that aren't 1,
            # on a non-conglomerating element, never collapse this.
            if repeat_count and (repeat_count != "1") and not will_conglomerate:
                return False

            if element_type != "x":
                non_padding_formats += 1

        # If we have more than one format type (discounting padding),
        # don't collapse.
        if non_padding_formats != 1:
            return False

        # Otherwise, collapse if we have exactly a single result.
        return len(result) == 1


    def execute_raw_command(self, class_number, verb, data=None, timeout=1000,
            encoding=None, max_response_length=4096, comms_timeout=1000):
        """Executes a libgreat command.

        Args:
            class_number -- The class number for the given command.
                See the GreatFET wiki for a list of class numbers.
            verb -- The verb number for the given command.
                See the GreatFET wiki for the given class.
            data -- Data to be transmitted to the GreatFET.
            timeout -- Maximum command execution time, in ms.
            encoding -- If specified, the response data will attempt to be
                decoded in the provided format.
            max_response_length -- If less than 4096, this parameter will
                cut off the provided response at the given length.

        Returns any data recieved in response.
        """
        raise NotImplementedError()



class CommsApiCollection(object):
    """ Dynamically-allocated container object that is automatically
        populated with API objects. Provides a view of our dictionary
        of APIs that has object semantics.
    """

    def __init__(self, wrapped_dict):
        self.__dict__ = wrapped_dict


def _generate_command_in_signature(in_format, in_names):
    """ Generates in-signature documentation for a given RPC.  
    This acts as input to python's built-in documentation engine.
    """

    # Always start with an 'self' argument, as these methods are bound
    # to their RPC clas.
    params = [inspect.Parameter('self', inspect.Parameter.POSITIONAL_ONLY)]

    # If we don't have any parameters, use an empty signature.
    if not in_format:
        return params

    # If we don't have any in-names, represnet this as an empty
    # list, so we always generate default names.
    if not in_names:
        in_names = []

    # Generate the type annotations for each element of the signature.
    annotations = CommsBackend.argument_annotations_for_format(in_format)

    # Generate a signature item for each type annotation.
    for index, annotation in enumerate(annotations):

        # By default, assume all arguments are positional only.
        # We don't have proper names for them, so their names can't
        # be used as keyword arguments.
        kind = inspect.Parameter.POSITIONAL_ONLY

        # If we have a proper name for this annotation, use it.
        try:
            name = in_names[index]
        except IndexError:
            name = "arg{}".format(index + 1)

        # If this is a variadic argument, update its type,
        # and prefix its name with a "*".
        if annotation.endswith('[]'):
            kind = inspect.Parameter.VAR_POSITIONAL

        # Generate the given parameter object.
        params.append(inspect.Parameter(name, kind, annotation=annotation))

    return params


def _generate_command_out_signature(out_format, out_names):
    """ Generates out-signature documentation for a given RPC. 
    This is used to generate a helpful return annotation e.g. for use as 
    input to python's documentation engine.
    """

    return_values = []

    # If we don't have an out-format, generate an empty return annotation.
    if not out_format:
        return inspect.Parameter.empty

    # If we don't have any out-names, represent this as an empty
    # list, so we always generate default names.
    if not out_names:
        out_names = []

    # Generate the type annotations for each element of the signature.
    annotations = CommsBackend.argument_annotations_for_format(out_format)

    # Generate a signature item for each type annotation.
    for index, annotation in enumerate(annotations):

        # If we have a proper name for this annotation, use it.
        try:
            name = out_names[index]
        except IndexError:
            name = ""

        # Prefix variadic out-arguments with an asterisk, a la
        # python convention.
        prefix = "*" if annotation.endswith("[]") else ""

        # Generate the given parameter object.
        return_values.append("{}{}: {}".format(prefix, name, annotation))

    return tuple(return_values)


def _generate_command_rpc_signature(in_format, in_names, out_format, out_names):
    """ Geneates a signature object that describes a generated method call.
        This allows our generated method to have standard python docs, if we 
        want (we do).
    """


    in_signature = _generate_command_in_signature(in_format, in_names)
    out_signature = _generate_command_out_signature(out_format, out_names)

    return inspect.Signature(parameters=in_signature, return_annotation=out_signature)



def command_rpc(verb_number, in_format="", out_format="", 
        name="function", doc="undocumented, generated function", in_parameter_names=None, out_parameter_names=None):
    """ Convenience function that creates an RPC method for a given verb. 

    Args:
        verb_number -- The verb number for the RPC to be created.
        in_format -- The format of the arguments accepted.  Defined the same way as CommsBackend.pack.
        out_format -- The format of the return value accepted.  Defined the same way as struct.unpack.
    
    This function can be used to quickly define methods that issue libgreat
    commands, transparently packing and unpacking arguments to handle the RPC.

    For example, in the following code::

        class ExampleClass(CommsClass):
            read_a_thing = command_rpc(verb_number=1, in_format="<II", out_format="<I")
    
    ExampleClass would wind up with a method called ``read_a_thing``, which accepts 
    two arguments and returns a single integer. Arguments will be encoded as two uint32_ts
    in the command payload, and the response would be intepreted as containing 
    a uint32_t.

    Calling this method would look like this::

        x = ExampleClass(comms_backend)
        y = x.read_a_thing(1, 2)

        # If successful, y should contian an integer!
    """

    from builtins import str

    # Create a partially bound method that's closed over the variables we want to store.
    def method(self, *arguments, **kwargs):
        encoding = kwargs.pop('encoding', None)
        timeout  = kwargs.pop('timeout', 1000)
        max_response_length = kwargs.pop('max_response_length', 4096)

        return self.execute_command(verb_number, in_format, out_format, name=name, timeout=timeout,
                max_response_length=max_response_length, *arguments)

    # Apply our known documentation to the given command.
    method.__name__ = future_utils.native_str(name)
    method.__doc__ = doc

    # Generate a method signature object, so the python documentation will be correct.
    # (This only helps on modern python, but oh well.)
    try:
        method.__signature__ = _generate_command_rpc_signature(
                in_format, in_parameter_names, out_format, out_parameter_names)
    except AttributeError:
        pass

    return method


def c_string_arguments(encoding='UTF-8', *strings):
    """ Convenience function intended to be passed to in_format which allows
        easy arguments which are lists of null-terminated strings.
    """

    payload = b""

    # Add each string, followed by a null character.
    for string in strings:
        payload += string.encode(encoding)
        payload += b"\0"

    return payload


def c_string_return(raw_bytes, encoding='UTF-8'):
    """ Convenience function intended to be passed to out_format which allows
        easy parsing of return arguments that consist of one or more null-terminated
        string.
    """

    # Split the bytes at each NULL.
    raw_strings = raw_bytes.split(b"\0")

    # If the string was correctly null-terminated,
    # we should have a straggling final element.
    # If it's there, remove it.
    if raw_strings[-1] == b"":
        del raw_strings[-1]

    # If we have an encoding argument, decode it.
    if encoding:
        raw_strings = [string.decode(encoding) for string in raw_strings]

    return tuple(raw_strings)


def int_array_arguments(specifier='I', *integers):
    """ Convenience function that collapses an array of integers into a libgreat
        command payload.

    Args:
        specifier -- A single letter specifying the way the integer is to be
                     encoded. Should be one of the format letters that can be
                     provided to struct.pack()
    """

    payload = b""

    for integer in integers:
        payload += struct.pack('<' + specifier, integer)

    return payload


def int_array_return(raw_bytes, specifier='I'):
    """ Convenience function that splits a command return into a tuple of
        evenly-spaced arguments.

    Args:
        specifier -- A single letter specifying the way the integer is to be
                     encoded. Should be one of the format letters that can be
                     provided to struct.pack()
    """

    # from the python itertools recipes section
    def break_into_chunks(size, string):
        """ Iterates over a byte-string, chunking it into groups of length size"""

        chunks = []

        while string:
            chunks.append(string[0:size])
            string = string[size:]

        return chunks


    def parse_bytes(b):
        """ Parses each our collection of integers. """
        return struct.unpack('<' + specifier, b)[0]

    # Break the array into integers.
    size = struct.calcsize(specifier)
    return [parse_bytes(b) for b in break_into_chunks(size, raw_bytes)]


class CommsClass(object):
    """ Class representing an libgreat communications class, which exposes
    a set of functionality to the host. Classes group _verbs_, which act
    like remote procedure calls.

    TODO: also describe pipes here
    """

    """ The number for the relevant class. This must be overridden."""
    CLASS_NUMBER = None

    def __init__(self, comms_backend):
        """ Sets up a new communications class. """

        self.comms_backend = comms_backend

        # Ensure that the class is defined enough.
        if self.CLASS_NUMBER is None:
            raise ArgumentError("This class is implemented improperly -- it must define CLASS_NUMBER! Object: {}".format(self))


    def execute_command(self, verb, in_format, out_format, *arguments, **kwargs):
        """Executes a libgreat command.

        Args:
            verb -- The verb number for the given command.
                See the GreatFET wiki for the given class.

            in_format -- Specifies the input format of the arguments,
                in the same format as used by struct.pack.
            out_format -- Specifies the format in which the output argumnt
                will be parsed; in the same format used by struct.unpack.
            *arguments -- The positional arguments to be passed to the libgreat
                device. These work as if passed to struct.pack.

            timeout -- Maximum command execution time, in ms.
            encoding -- If specified, any strings in response data will be
                decoded in the provided format.
            max_response_length -- If less than 4096, this parameter will
                cut off the provided response at the given length.

        Returns any data recieved in response, parsed according to out_format.
        """
        return self.comms_backend.execute_command(self.CLASS_NUMBER, verb, in_format, 
                out_format, *arguments, **kwargs)


    def execute_raw_command(self, class_number, verb, data=None, timeout=1000,
            encoding=None, max_response_length=4096, comms_timeout=1000):
        """Executes a libgreat command.

        Args:
            class_number -- The class number for the given command.
                See the GreatFET wiki for a list of class numbers.
            verb -- The verb number for the given command.
                See the GreatFET wiki for the given class.
            data -- Data to be transmitted to the GreatFET.
            timeout -- Maximum command execution time, in ms.
            encoding -- If specified, the response data will attempt to be
                decoded in the provided format.
            max_response_length -- If less than 4096, this parameter will
                cut off the provided response at the given length.

        Returns any data recieved in response.
        """
        return self.execute_raw_command(self.CLASS_NUMBER, verb, data, timeout, 
                encoding, max_response_length, comms_timeout)


class GeneratedCommsClass(CommsClass):
    """ Special variant of CommsClass used as the parent of procedurally-
        genereated CommsClass. Can be used to differentiate automatically and
        manually generated classes.
    """
