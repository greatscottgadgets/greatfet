#
# This file is part of GreatFET
#

import os

from ..interface import GreatFETInterface


class M0Coprocessor(GreatFETInterface):
    """
    Class that represents and controls the GreatFET's m0 coprocessor.
    """

    # Upload in 2048-byte chunks. This fits nicely in a libgreat command, as is a nice, logical
    # unit for uploading.
    UPLOAD_CHUNK_SIZE = 2048


    def __init__(self, device):
        """ Creates a new M0 coprocessor control object. """

        self.device = device
        self.api = device.apis.loadables


    def start(self):
        """ Starts execution of the m0 processor. """
        self.api.start_m0()


    def halt(self):
        """ Halts execution of the m0 processor. """
        self.api.halt_m0()


    def load_loadable(self, data_or_filename):
        """
        Loads (but does not start) an M0 loadable.

        Arguments:
            data_or_filename -- The loadable binary file, or the filename where one can be found.
        """
        data = data_or_filename

        # If we have a string that seems to be a valid filename, load the file as our data.
        if isinstance(data_or_filename, str) and os.path.isfile(data_or_filename):
            with open(data_or_filename, "rb") as f:
                data = f.read()

        # FIXME: validate the binary size

        # Halt the m0 processor before the upload.
        self.halt()

        # Iterate over each chunk in the relevant data, and upload it.
        for offset in range(0, len(data), self.UPLOAD_CHUNK_SIZE):

            # Grab the data chunk to upload...
            to_upload = data[offset:offset + self.UPLOAD_CHUNK_SIZE]

            # ... and upload it.
            self.api.load_m0_page(offset, to_upload)



    def run_loadable(self, data_or_filename):
        """
        Runs a "loadable" binary on the GreatFET's Cortex-M0.
        Equivalent to calling .load_loadable() and then .start().

        Arguments:
            data_or_filename -- The loadable binary file, or the filename where one can be found.
        """

        # Upload the loadable code...
        self.load_loadable(data_or_filename)

        # ... and run it.
        self.start()







