#
# Copyright (c) 2016 Kyle J. Temkin <kyle@ktemkin.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
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

    if error_number in GREATFET_ERRORS:
        error_class = GREATFET_ERRORS[error_number]
    else:
        error_class = GreatFETError

    message = "Error {}".format(error_number)

    return error_class(message)


