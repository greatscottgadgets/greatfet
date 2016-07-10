#
# Copyright 2016 Kyle J. Temkin <kyle@ktemkin.com>
#
# This file is part of GreatFET.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)gt
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
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

    if error_number in GREATFET_ERRORS:
        error_class = GREATFET_ERRORS[error_number]
    else:
        error_class = GreatFETError

    message = "Error {}".format(error_number)

    return error_class(message)


