#
# This file is part of GreatFET
#

class DeviceNotFoundError(IOError):
    """ Error indicating no GreatFET device was found. """
    pass


class DeviceBusyError(IOError):
    """ Error indicating the GreatFET is too busy to service the given request. """
    pass


class DeviceMemoryError(MemoryError):
    """ Error indicating that the GreatFET has run out of memory. """
    pass


class NotFoundError(IOError):
    """ Error indicating that a resource was not found. """
    pass

class GreatFETError(RuntimeError):
    """ Runtime error used when no better description is available. """
    pass


class ExternalDeviceError(IOError):
    """
    Error used when a external device (e.g. not on the GreatFET)
    experiences an issue. This typically means that the error is not wit
    the GreatFET hardware or software, but may be with e.g. connections.
    """


GREATFET_ERRORS = {
    -2: ValueError,
    -5: NotFoundError,
    -6: DeviceBusyError,
    -7: MemoryError,
}

# Libgreat's errno.h provides the following values, which we use to be standard across both sides of the RPC.
LIBGREAT_ERROR_NAMES =  {
    1: "EPERM",
    2: "ENOENT",
    3: "ESRCH",
    4: "EINTR",
    5: "EIO",
    6: "ENXIO",
    7: "E2BIG",
    8: "ENOEXEC",
    9: "EBADF",
    10: "ECHILD",
    11: "EAGAIN",
    12: "ENOMEM",
    13: "EACCES",
    14: "EFAULT",
    15: "ENOTBLK",
    16: "EBUSY",
    17: "EEXIST",
    18: "EXDEV",
    19: "ENODEV",
    20: "ENOTDIR",
    21: "EISDIR",
    22: "EINVAL",
    23: "ENFILE",
    24: "EMFILE",
    25: "ENOTTY",
    26: "ETXTBSY",
    27: "EFBIG",
    28: "ENOSPC",
    29: "ESPIPE",
    30: "EROFS",
    31: "EMLINK",
    32: "EPIPE",
    33: "EDOM",
    34: "ERANGE",
    35: "ENOMSG",
    36: "EIDRM",
    37: "ECHRNG",
    38: "EL2NSYNC",
    39: "EL3HLT",
    40: "EL3RST",
    41: "ELNRNG",
    42: "EUNATCH",
    43: "ENOCSI",
    44: "EL2HLT",
    45: "EDEADLK",
    46: "ENOLCK",
    50: "EBADE",
    51: "EBADR",
    52: "EXFULL",
    53: "ENOANO",
    54: "EBADRQC",
    55: "EBADSLT",
    56: "EDEADLOCK",
    57: "EBFONT",
    60: "ENOSTR",
    61: "ENODATA",
    62: "ETIME",
    63: "ENOSR",
    64: "ENONET",
    65: "ENOPKG",
    66: "EREMOTE",
    67: "ENOLINK",
    68: "EADV",
    69: "ESRMNT",
    70: "ECOMM",
    71: "EPROTO",
    74: "EMULTIHOP",
    75: "ELBIN",
    76: "EDOTDOT",
    77: "EBADMSG",
    79: "EFTYPE",
    80: "ENOTUNIQ",
    81: "EBADFD",
    82: "EREMCHG",
    83: "ELIBACC",
    84: "ELIBBAD",
    85: "ELIBSCN",
    86: "ELIBMAX",
    87: "ELIBEXEC",
    88: "ENOSYS",
    89: "ENMFILE",
    90: "ENOTEMPTY",
    91: "ENAMETOOLONG",
    92: "ELOOP",
    95: "EOPNOTSUPP",
    96: "EPFNOSUPPORT",
    104: "ECONNRESET",
    105: "ENOBUFS",
    106: "EAFNOSUPPORT",
    107: "EPROTOTYPE",
    108: "ENOTSOCK",
    109: "ENOPROTOOPT",
    110: "ESHUTDOWN",
    111: "ECONNREFUSED",
    112: "EADDRINUSE",
    113: "ECONNABORTED",
    114: "ENETUNREACH",
    115: "ENETDOWN",
    116: "ETIMEDOUT",
    117: "EHOSTDOWN",
    118: "EHOSTUNREACH",
    119: "EINPROGRESS",
    120: "EALREADY",
    121: "EDESTADDRREQ",
    122: "EMSGSIZE",
    123: "EPROTONOSUPPORT",
    124: "ESOCKTNOSUPPORT",
    125: "EADDRNOTAVAIL",
    126: "ENETRESET",
    127: "EISCONN",
    128: "ENOTCONN",
    129: "ETOOMANYREFS",
    130: "EPROCLIM",
    131: "EUSERS",
    132: "EDQUOT",
    133: "ESTALE",
    134: "ENOTSUP",
    135: "ENOMEDIUM",
    136: "ENOSHARE",
    137: "ECASECLASH",
    138: "EILSEQ",
    139: "EOVERFLOW",
    140: "ECANCELED",
    141: "ENOTRECOVERABLE",
    142: "EOWNERDEAD",
    143: "ESTRPIPE",
}

def get_error_name(error_number):
    """ Returns the error string for the given libgreat name, or None if none exists """

    if error_number in LIBGREAT_ERROR_NAMES:
        return LIBGREAT_ERROR_NAMES[error_number]
    else:
        return None


def from_greatfet_error(error_number):
    """
    Returns the error class appropriate for the given GreatFET error.
    """
    error_class = GREATFET_ERRORS.get(error_number, GreatFETError)
    message = "Error {}".format(error_number)
    return error_class(message)
