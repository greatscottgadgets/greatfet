================================================
LibGreat Verb Signatures
================================================

``libgreat`` verbs are designed to be self-describing: each verb provided by a libgreat device includes a small body of metadata that can be queried by the host:

.. code-block :: sh

  {    
      // The name of the verb. These are typically named like C function names.
      .name = "sum_and_difference", 
      
      // The handler function for the given verb. This is the verb definition we
      // provided above.
      .handler = example_verb_sum_and_difference,

      // A short piece of documentation for the verb.
      .doc = "Computes the sum and difference of two ints.",

      // The signature for the verb's arguments. This roughly matches python's
      // struct.pack format; see the wiki documentation for more information.
      .in_signature = "<II",

      // The signature for the verb's return values. This roughly matches python's
      // struct.pack format; see the wiki documentation for more information.
      .out_signature = "<II",

      // The names of the arguments to the verb.
      .in_param_names = "a, b",

      // The names of the return values for the verb.
      .out_param_names = "sum, difference"
  },

This meta-data can include machine-parseable descriptions of each verb's signatures in the form of a short, machine-readable string. These strings include a description of the arguments to the function (the in-signature) and of the function's multiple return values (the out-signature); and effectively describe the data formats sent to and from the device during execution of a verb.

Providing descriptions of these signatures is optional, but it's highly recommended that you do so whenever possible: the ``libgreat`` host library can use these signatures to generate code for you -- making device communications transparent to your code!



Describing Verb Signatures
~~~~~~~~~~~~~~~~~~~~~~~~~~

Verb signatures are provided in a format that's heavily inspired by `Python's struct module <https://docs.python.org/2/library/struct.html>`__ -- in fact, the formats are mostly identical. To provide some additional flexibility, we support a few additional format characters. The standard and added format characters are described below:

.. list-table:: Verb Signatures
   :widths: 10 7 11 10 33 29
   :header-rows: 1

   * - Format
     - Bytes
     - C-Type
     - Python Type
     - libgreat parse function
     - libgreat response function
   * - x *(1)*
     - 1
     - none
     - none
     - comms_argument_read_buffer( trans, 1, NULL )
     - comms_argument_read_buffer( trans, 1, NULL )
   * - c
     - 1
     - char
     - string of length 1
     - comms_argument_parse_uint8_t
     - comms_response_add_uint8_t
   * - b
     - 1
     - int8_t
     - integer
     - comms_argument_parse_int8_t
     - comms_response_add_int8_t
   * - B
     - 1
     - uint8_t
     - integer
     - comms_argument_parse_uint8_t
     - comms_response_add_uint8_t
   * - ?
     - 1
     - bool / _Bool
     - integer
     - comms_argument_parse_bool
     - comms_response_add_bool
   * - h
     - 2
     - int16_t
     - integer
     - comms_argument_parse_int16_t
     - comms_response_add_int16_t
   * - H
     - 2
     - uint16_t
     - integer
     - comms_argument_parse_uint16_t
     - comms_response_add_uint16_t
   * - i
     - 4
     - int32_t
     - integer
     - comms_argument_parse_int32_t
     - comms_response_add_int32_t
   * - I
     - 4
     - uint32_t
     - integer
     - comms_argument_parse_uint32_t
     - comms_response_add_uint32_t
   * - l
     - 4
     - int32_t
     - integer
     - comms_argument_parse_int32_t
     - comms_response_add_int32_t
   * - L
     - 4
     - uint32_t
     - integer
     - comms_argument_parse_uint32_t
     - comms_response_add_uint32_t
   * - q
     - 8
     - int64_t
     - integer
     - comms_argument_parse_int64_t
     - comms_response_add_int64_t
   * - Q
     - 8
     - uint64_t
     - integer
     - comms_argument_parse_uint64_t
     - comms_response_add_uint64_t
   * - f
     - 4
     - float
     - float
     - comms_argument_parse_float
     - comms_response_add_float
   * - d
     - 8
     - double
     - float
     - comms_argument_parse_double
     - comms_response_add_double
   * - s *(2)(3)(6)*
     - 
     - char[]
     - string
     - comms_argument_read_buffer
     - comms_response_add_raw
   * - p *(2)(6)*
     - 
     - char[]
     - string
     - comms_argument_parse_uint8_t / comms_argument_read_buffer
     - comms_response_add_uint8_t / comms_response_add_raw
   * - S *(4)*
     - 
     - char[]
     - string
     - comms_argument_read_string
     - comms_response_add_string
   * - X *(3)(5)(6)*
     - 
     - uint8_t
     - bytes of length 1
     - comms_argument_read_string
     - comms_response_add_string


.. list-table:: 
  :header-rows: 1
  :widths: 1 2

  * - note number
    - description
  * - *(1)*
    - null padding byte; rarely used
  * - *(2)*
    - see python docs
  * - *(3)*
    - typically used with a numeric prefix
  * - *(4)*
    - encodes a null-terminated string
  * - *(5)*
    - encodes raw bytes; repeated elements are merged into a single entry
  * - *(6)*
    - numeric prefixes behave differently; see below

``libgreat`` data is always of standard size, and *always* little-endian. **Accordingly, every non-empty method signature must begin with a '<'.** Exceptions are made for methods that expect no arguments or return no values, which can provide an empty string.




Repeat Specifiers
~~~~~~~~~~~~~~~~~

Most types can be modified with a numeric *repeat specifier*; this acts the same as if the element were repeated multiple times. For example:

.. code-block:: sh

    4I

is exactly equivalent to:

.. code-block:: sh

    IIII

This matches the behavior of Python's pack and unpack. Unless denoted with note ``(6)`` in the table above, each type supports a repeat specifier.

``libgreat`` adds one additional repeat specifier: a repeat specifier of ``*`` specifies that all a remaining data or arguments should be interpereted as instances of the provided type. Accordingly, a verb with an in-signature of ``<*I`` accepts any number of ``uint32_t`` arguments (including zero); a verb with an out-signature of ``<II*B`` would always return two 32-bit integers, followed by any number of single bytes.




Length Specifiers
~~~~~~~~~~~~~~~~~

A handful of format specifiers interpret numeric prefixes as *element lengths*, rather than repeat counts. These types interpret these specifiers as documented below:

.. list-table :: 
  :header-rows: 1
  :widths: 1 3

  * - type
    - interpretation
  * - s
    - the specified element represents a string of N characters, where N is the length specifier
  * - p
    - the specified element represents a pascal string of maximum length N, where N is the length specifier
  * - X
    - the specified element represents a string of N bytes, where N is the length specifier

For the ``s`` and ``X`` specifiers, a length specifier of ``*`` indicates that the relevant string can be expected to take up all of the remaining data. Note that the format ``S`` does accepts a *repeat specifier* and **not** a *length specifier*, so the string ``32S`` denotes 32 null-terminated strings.



Element Groups
~~~~~~~~~~~~~~

``libgreat``'s format strings add one additional feature: *format groups*. Format groups use parenthesis to create *groups of elements*, which are handled slightly differently:

    - On the python side, each format group accepts a single tuple (or list) that should contain each of the parenthesized types. So, the group ``<(IIB)`` would expect a single tuple contianing three integers, which would be packed as two consecutive ``uint32_ts`` followed by a ``uint8_t``.
    - Each format group can accept a *repeat specifier*; so the string ``<8(IB)`` would denote eight pairs of one ``uint32_t`` and one ``uint8_t``. A repeat specifier of ``*`` is also acceptable, which implies that the entire remainder of the arguments accepted or data parsed will consist of pairs of ``uint32_t`` and ``uint8_t``.



Examples
~~~~~~~~

It may help to consider an example RPC with the following meta-data:

.. code-block:: sh

  { .name = "sum_polar", .handler = example_verb_sum_polar, .in_signature = "<*(II)",
      .out_signature = "<II", .in_param_names = "magnitudes_and_angles", .out_param_names = "sum_magnitude, sum_angle",
      .doc = "Sums together a collection of polar coordinates." },

The method's in-signature, ``<*(II)``, demonstrates that the method expects any number of *two-element pairs*, which each contain a pair of integers. Accordingly, we might call it as follows:

.. code-block:: sh

  # Assuming the RPC is available as gf.apis.example.sum_polar:
  magnitude, angle = gf.apis.example.sum_polar((1, 2,), (3, 4),)

Each argument will be intepreted as a pair of 32-bit integers; so the resultant data stream will wind up looking like:

.. code-block:: sh

  <uint32_t '1'><uint32_t '2'><uint32_t '3'><uint32_t '4'>

On the device side, we might read the data as follows:

.. code-block:: sh

  static int example_verb_sum_polar(struct command_transaction *trans)
  {
      uint32_t sum_magnitude = 0, sum_angle = 0;

      // While there's still data available in the string, grab vectors the data-stream.
      while (comms_argument_data_remaining(trans)) {

          // Read the next pair of vector components from the data stream...
          uint32_t magnitude = comms_argument_parse_uint32_t(trans);
          uint32_t angle = comms_argument_parse_uint32_t(trans);
        
          // ... do your math here.
          // <left as an exercise to the reader>
      } 

      // Check to make sure we actually got all the pairs we tried to read.
      // If we didn't, this function will fail out!
      if (!comms_transaction_okay(trans)) {
          return EBADMSG;
      }

      // And respond with the relevant data.
      comms_response_add_uint32_t(trans, sum_magnitude);
      comms_response_add_uint32_t(trans, sum_angle);
      return 0;
  }

In this case, we repeatedly call ``comms_argument_parse_uint32_t`` to capture each piece of the input stream; using ``comms_argument_data_remaining`` to check how much data is left.



Omitting Verb Signatures
~~~~~~~~~~~~~~~~~~~~~~~~

In some cases, we may not exactly be able to describe our data format using the strings above; or we may not know the data format until run-time. In these cases, the verb signature can be replaced with the string ``"*"``, which indicates that the signature is too complex to be handled automatically.

Using this signature allows us to be flexible, but comes at a significant cost: the host code can no longer automatically generate RPC methods for us. It becomes our responsibility to provide code on the host side for to interface with these verbs. Typically, this is accomplished using the ``execute_raw_command`` method of the ``CommsBackend`` class. See the on-line help for more documentation:

.. code-block:: sh

  from pygreat.comms import CommsBackend
  help(CommsBackend.execute_raw_command)
