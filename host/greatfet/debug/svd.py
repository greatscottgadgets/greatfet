#
# GreatFET debug wrappers for SVD files.
# TODO: decide if this should be in libgreat?
#

import tabulate
tabulate.PRESERVE_WHITESPACE = True


class SVDGenerated(object):
    """ Generic base class for objects generated from SVDs. """

    _children           = None
    parent             = None
    _name              = None
    _read_before_write = True
    write_only         = False
    read_only          = False
    _short_type        = "SVD object"

    def __init__(self, parent):
        """ Generic constructor for SVD-generated objects. Usually not called directly. """

        self.__dict__['parent'] = parent


    @staticmethod
    def _normalize_name(name):
        """ Attempts to massage a given name into being a valid python identifier."""

        append_to_numeric = "VALUE_{}".format(name) if name[0].isnumeric() else name
        return append_to_numeric.lower()

    @classmethod
    def _attach_standard_fields(cls, unique_type, svd_element):
        """ Attach our standard fields a given unique type. Used during subclass construction. """

        unique_type._name = cls._normalize_name(svd_element.name)
        unique_type.__doc__ = svd_element.description


    def __setattr__(self, name, value):

        if not hasattr(self, name):
            raise AttributeError("cannot add fields to the debug address space")
        else:
            super(SVDGenerated, self).__setattr__(name, value)


    def __dir__(self):
        return self._children.keys()


    def _get_long_name(self):
        """ Retrieves a long name for the given class, with a "fully-qualified"-style path in the SVD domain. """

        # Build a list of path components...
        name = [self._name]

        # ... prepending each parent name until we run out of parents.
        parent = self.parent
        while parent:
            name[:0] = (parent._name,)
            parent = parent.parent

        # Squish all our path components together into a nice, long name.
        return ".".join(name)


    def __getattr__(self, name):
        """ General 'missed attribute' handler that makes objects case-insensitive. """

        # Special case: translate __name__ requests to SVD names.
        if name == "__name__":
            return self._name
        elif name.lower() in self._children:
            return self._children[name.lower()]
        else:
            raise KeyError("invalid key {} in {}".format(name, self._get_long_name()))


    def __repr__(self):
        """ Fancy printer for most SVD-derived classes. """
        return "<{} {}: {}._children>".format(self._short_type, self._get_long_name(), len(self._children))


    def methodname(self, typename, level):
        return "bees"

    @classmethod
    def _unique_type_from_svd_attribute(cls, svd_object, attribute, name_prefix="GeneratedObject", translate_writes=False):
        """ Creates a new type that represents a given section of SVD memory. """

        # Create normalized, hopefully-unique type name.
        type_name = "{}_{}".format(name_prefix, cls._normalize_name(svd_object.name))

        # Create the new, unique type for the generated debug register. Generating a new type
        # is necessary to have python treat the properties on the new object as 'first class' --
        # for example, to have our generated documentation automatically show in help().
        unique_type = type(type_name, (cls,), {})
        cls._attach_standard_fields(unique_type, svd_object)

        # Capture the name of the SVD attribute we represent.
        unique_type._attribute = attribute

        # Generate properties for each of our relevant fields. These will create the attributes
        # on our relevant type that serve as our documentation.
        unique_type._children = {}
        for child in getattr(svd_object, attribute):

            # We can't work with._children that don't have names, so skip them.
            if child.name is None:
                continue

            name = cls._normalize_name(child.name)

            # If 'translate_writes' is set, automatically translate any assignment
            # to the given object into a call to its poke() method.
            if translate_writes:
                setter = lambda self, value, name=name : self._children[name].poke(value)

            # Otherwise, don't allow the value to be overwritten.
            else:
                setter = None

            # Always allow the property to be read.
            getter = lambda self, name=name : self._children[name]

            # Build the property that defines accessors for the given field, and attach it to our type.
            field_property = property(getter, setter, doc=child.description)
            setattr(unique_type, name, field_property)

        return unique_type


    @classmethod
    def _instantiate_unique_type(cls, unique_type, child_type, svd_object, *arguments):

        # Finally, create our relevant instance...
        instance = unique_type(*arguments)

        # ... and populate each of the fields, complete with a parent reference.
        for value in getattr(svd_object, instance._attribute):

            if value.name is None:
                continue

            name = cls._normalize_name(value.name)
            instance._children[name] = child_type.from_svd(value, instance)

        return instance


class SVDMemoryAccessible(SVDGenerated):
    """ Subclass of SVDGenerated for objects that are accessible in memory; e.g. that implement
        peek(self) and poke(self, value).
    """

    @staticmethod
    def _attach_standard_fields(unique_type, svd_element):

        # Attach our basic standar fields.
        SVDGenerated._attach_standard_fields(unique_type, svd_element)

        # Mark whether this property is read-only / write-only.
        unique_type.read_only  = svd_element.access == "read-only"
        unique_type.write_only = svd_element.access == "write-only"

    def peek(self):
        raise NotImplementedError()

    def poke(self, value):
        raise NotImplementedError()


    def __int__(self):
        """ Allow this type to be referenced as an integer. """
        return self.peek()


    def __repr__(self):
        """ Print a nice, eloquent description of the current type, including value. """

        name  = self._get_long_name()

        if self.write_only:
            return "{}: write-only register".format(self._get_long_name)

        if hasattr(self, '_width') and self._width == 1:
            return "{} = {}".format(name, int(self))
        else:
            value = int(self)
            return "{} = {} / 0x{:x} / 0b{:b}".format(name, value, value, value)


class MemoryWindow(SVDGenerated):
    """ Class that implements a view into the target's memory.

    This object can be used to read and write memory using the indexing operator:

        print(gf.lowlevel.memory[0x00000000]) # prints the contents of address 0 (the start of the vector table)

        gf.lowlevel.memory[0x10000000] = 0    # writes the value 0 to address 0x10000000

    """

    _name = "memory"
    _description = "Full memory address space"

    def __init__(self, peek_function, poke_function):
        """ Sets up our view into memory. """

        # Store our peek and poke functions,
        self.__dict__['peek'] = peek_function
        self.__dict__['poke'] = poke_function


    def __getitem__(self, address):
        """ Shortcut that allows us to read memory using the index operator. """

        if isinstance(address, slice):
            result = []

            for i in range(address.start, address.stop, address.step or 1):
                result.append(self.peek(i))

            return result
        else:
            return self.peek(address)

    def __setitem__(self, address, value):
        """ Shortcut that allows us to write to memory using the index operator. """

        if isinstance(address, slice):
            for i in range(address.start, address.stop, address.step or 1):
                self.poke(i, value)
        else:
            self.poke(address, value)


    def __repr__(self):
        return object.__repr__(self)



class DebugTarget(SVDGenerated):
    """ Class representing a target board, and its associated address space.. """

    _short_type = "target device"

    @classmethod
    def from_svd(cls, svd_device, *arguments):
        """ Creates a new DebugPeripheral-derived object for a given SVD peripheral. """

        # Create a unique object type that represents the given SVD peripheral.
        unique_type = cls._unique_type_from_svd_attribute(svd_device, 'peripherals',
            "GeneratedDebugTarget", translate_writes=False)

        # Add a window into the target's memory.
        unique_type.memory = property(lambda self : self._children['memory'], doc="Provides an indexable view into target memory.")

        # Finally, instantiate the unique type to get a register object.
        instance = cls._instantiate_unique_type(unique_type, DebugPeripheral, svd_device, *arguments)
        instance._children['memory'] = MemoryWindow(instance.peek, instance.poke)

        return instance


    def __init__(self, peek_function, poke_function):
        """ Create the address space object by wrapping a local SVD.

        Params:
            peek_function: A function that takes an address, and returns its value. Specified in the system word size.
            poke_function: A function that takes (address, value), and sets the given address to
                contain the relevant value. Always writes the system's word size.
        """

        self.__dict__['_peek'] = peek_function
        self.__dict__['_poke'] = poke_function

    def peek(self, address):
        """ Returns the contents of the target memory address. """
        return self._peek(address)


    def poke(self, address, value):
        """ Writes the provided value into the target memory address. """
        return self._poke(address, value)


    def peripherals(self):
        return self._children.keys()


    def __repr__(self):

        headers = ['peripheral', 'description']
        rows = []

        for peripheral in self._children.values():
            rows.append([peripheral._name, peripheral._description or peripheral.__doc__])

        return tabulate.tabulate(rows, tablefmt='simple', headers=headers)



class DebugPeripheral(SVDGenerated):
    """ Class representing a peripheral on a target board. """

    _base = None
    _short_type = "peripheral"
    _description = "anonymous peripheral"

    @classmethod
    def from_svd(cls, svd_peripheral, parent):
        """ Creates a new DebugPeripheral-derived object for a given SVD peripheral. """

        # Create a unique object type that represents the given SVD peripheral.
        unique_type = cls._unique_type_from_svd_attribute(svd_peripheral, 'registers',
            "GeneratedDebugPeripheral", translate_writes=True)

        # ... and store its base address.
        unique_type._base = svd_peripheral.base_address
        unique_type._description = svd_peripheral._description

        # Finally, instantiate the unique type to get a register object.
        return cls._instantiate_unique_type(unique_type, DebugRegister, svd_peripheral, parent)


    def peek_at_offset(self, offset):
        """ Read the given register, and return its value. """

        # Read the given register from the parent's address range.
        return self.parent.peek(self._base + offset)


    def poke_at_offset(self, offset, value):
        """ Write a provided value to this register. """

        if self.read_only:
            raise AttributeError("tried to write into write-only register {}".format(self._name))

        return self.parent.poke(self._base + offset, value)


    def registers(self):
        return self._children.keys()


    def __repr__(self, include_fields=False):

        headers = ['register', 'dec', 'hex', 'bin', 'note']
        table_entries = []

        # Add each of the registers to this representation.
        for register in self._children.values():

                value = None if register.write_only else int(register)

                table_entries.append(register._table_row(value))

                # And if we're including fields, add them, too.
                if (value is not None) and include_fields:

                    # If we don't have any fields to represent, then continue without trying.
                    if (not register._children) or list(register._children.values())[0]._represents_whole_register():
                        continue

                    for field in register._children.values():
                        table_entries.append(field._table_row(value))

                    # Add an empty row to help space things out.
                    table_entries.append(['', '', '', '', ''])

        return tabulate.tabulate(table_entries, tablefmt='simple', headers=headers)


    def print_all(self, include_fields=False):
        """ Convenience method that prints a representation of every child of the given object. """

        print(self.__repr__(include_fields))




class DebugRegister(SVDMemoryAccessible):
    """ Class representing a register on a target board. """

    _offset = None
    _fields = {}

    @classmethod
    def from_svd(cls, svd_register, parent):
        """ Creates a new DebugRegister-derived object for a given SVD register. """

        # Create a unique object type that represents the given SVD register's topology..
        unique_type = cls._unique_type_from_svd_attribute(svd_register, 'fields',
                "GeneratedDebugRegister", translate_writes=True)

        # Store our address offset into the relevant peripheral.
        unique_type._offset = svd_register.address_offset

        # Finally, instantiate the unique type to get a register object.
        return cls._instantiate_unique_type(unique_type, DebugField, svd_register, parent)


    def peek(self):
        """ Read the given register, and return its value. """

        if self.write_only:
            raise AttributeError("tried to read from write-only register {}".format(self._name))

        # Read the given register from the parent's address range.
        return self.parent.peek_at_offset(self._offset)


    def poke(self, value):
        """ Write a provided value to this register. """

        if self.read_only:
            raise AttributeError("tried to write into write-only register {}".format(self._name))

        return self.parent.poke_at_offset(self._offset, value)


    def fields(self):
        return self._children.keys()


    def _get_unimplemented_bits(self):

        # Start off with a belief that all bits are implemented.
        values = set(range(0, 32))

        # Iterate over each field in the register...
        for field in self._children.values():

            # ... and remove any bits that are represented by a field.
            for i in range(field._width):
                values.remove(field._shift + i)

        return values


    def _table_row(self, value=None):
        """ Generates a table row for the active register. """

        if self.write_only:
            return [self._name, 'w', "w" * 8, "w" * 32, 'write-only']
        else:
            if value is None:
                value = int(self)

            bit_value_string = "{:032b}".format(value)

            # Modify the string, so any unimplemented bits are not included.
            bit_value_list = list(bit_value_string)
            unimplemented_bits = self._get_unimplemented_bits()
            for bit in unimplemented_bits:
                string_position = 31 - bit
                bit_value_list[string_position] = '-'
            bit_value_string = "".join(bit_value_list)

            return [self._name, value, "{:08x}".format(value), bit_value_string, self.__doc__]


    def __repr__(self):
        """ Print a nice, eloquent description of the current type, including value and fields. """

        if self.write_only:
            return "{}: write-only register".format(self._get_long_name())

        headers = ['register', 'dec', 'hex', 'bin', 'note']

        # Generate our first table row.
        value = None if self.read_only else int(self)
        table_entries = [self._table_row(value)]

        # Add each of the fields to this representation.
        if value is not None:
            for field in self._children.values():
                table_entries.append(field._table_row(value))

        return tabulate.tabulate(table_entries, tablefmt='simple', headers=headers)








class DebugField(SVDMemoryAccessible):
    """ Base class for representing a set of bits in a device register. """

    # Fields overridden in generated classes.
    # Defining these here makes linters happy.
    _shift             = None
    _mask              = None
    _mask_inverse      = None
    value_names        = {}

    @classmethod
    def from_svd(cls, svd_field, parent):
        """ Creates a new DebugField-derived object for a given SVD field definition. """

        type_name = cls._normalize_name(svd_field.name)

        # Create the new, unique type for the generated debug field. See comments for `from_svd` above.
        unique_type = type("GeneratedDebugField_{}".format(type_name), (cls,), {})
        cls._attach_standard_fields(unique_type, svd_field)

        # Attach each of the enumerated values to our class.
        enum_names = {}

        if svd_field.enumerated_values:
            for value in svd_field.enumerated_values:

                if not value.name:
                    continue

                name = cls._normalize_name(value.name)

                # Create the relevant property, and attach it to our new class.
                prop = property(lambda _ : value.value, doc=value.description)
                setattr(unique_type, name, prop)

                # And generate a reverse-lookup dictionary for
                enum_names[value.value] = name

        # Attach the enum names to the object.
        unique_type.value_names = property(lambda _ : enum_names, doc="dictionary that maps integer values to enum names")

        # Set the properties that allow us to extract this field from its parent register.
        unique_type._shift        = svd_field.bit_offset
        unique_type._width        = svd_field.bit_width
        unique_type._mask         = ((1 << svd_field.bit_width) - 1) << unique_type._shift
        unique_type._mask_inverse = 0xFFFFFFFF - unique_type._mask

        # Determine whether we should read the value before writing back certian bits.
        # Some registers are write only; or are e.g. write-to-set; so we don't need to
        # read them first.
        # TODO: optimize by looking at the modified_write_values flag and not reading if
        # this is a register that e.g. sets bits
        unique_type._read_before_write = not unique_type.write_only

        # Finally, create an instance of the generated class and return.
        return unique_type(parent)


    def extract_value(self, raw_register):
        """ Extracts the value of this field given a raw register value. """

        return (raw_register & self._mask) >> self._shift


    def peek(self):
        """ Returns the value of the relevant debug field. """

        if self.write_only:
            raise AttributeError("tried to read from write-only field {}".format(self._name))


        # Read the raw value of the parent register, and then extract this field.
        raw = self.parent.peek()
        return self.extract_value(raw)


    def value_name(self, default=None):
        """ Returns the name of the given field value, if it has one; by default, the raw value if not. """
        value = self.peek()

        # If we have a name for the value, return it.
        if value in self.value_names:
            return self.value_names[value]

        # Otherwise, return the default value (or the value if not provided).
        else:
            return default if (default is not None) else value


    def poke(self, value):
        """ Sets the value of the relevant debug field. """

        if self.read_only:
            raise AttributeError("tried to write into read-only field {}".format(self._name))

        # If we need to read the previous value before updating, do so.
        old_value = self.parent.peek() if self._read_before_write else 0

        # Mask away the section of the old value that we'll be updating.
        old_value = old_value & self._mask_inverse

        # Shift the our new value up into the place where it should be.
        value = value << self._shift

        # Finally, merge the old and new values, and write them back.
        self.parent.poke(old_value | value)


    def _represents_whole_register(self):
        """ Returns true iff the given field covers the whole register. """

        return self._width == 32


    def _table_row(self, parent_register_value):
        """ Generates a table row for __repr__ with multiple display formats. """

        value = self.extract_value(parent_register_value)

        # Generate a nice binary padding that indicates where the fields are in their register.
        after_padding = "." * self._shift
        before_padding = "." * (32 - self._width - self._shift)
        formatted_binary = "{}{:0{}b}{}".format(before_padding, value, self._width, after_padding)

        # And do the same for hex.
        hex_width = (self._width + 3) // 4
        formatted_hex = "{:0{}x}".format(value, hex_width)

        # Finally, if have an enumerated value for the current
        note = self.value_name(default='')

        # Finally, generate the table row for this field.
        return ["    ." + self._name, value, formatted_hex, formatted_binary, note]


    def __dir__(self):
        return self.enum_names.values()
