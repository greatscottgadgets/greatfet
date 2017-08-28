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


def from_greatfet_error(error_number):
    """
    Returns the error class appropriate for the given GreatFET error.
    """
    error_class = GREATFET_ERRORS.get(error_number, GreatFETError)
    message = "Error {}".format(error_number)
    return error_class(message)
