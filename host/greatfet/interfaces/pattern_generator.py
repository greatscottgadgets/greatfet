#
# This file is part of GreatFET
#

from ..interface import GreatFETInterface


class PatternGenerator(GreatFETInterface):
    """
        Class that supports using the GreatFET as a simple pattern generator.
    """

    def __init__(self, board, sample_rate=1e6, bus_width=8):
        """ Set up a GreatFET pattern generator object. """

        # Grab a reference to the board and its pattern-gen API.
        self.board = board
        self.api   = board.apis.pattern_generator

        # Grab a reference to the user's bus parameters.
        self.sample_rate = int(sample_rate)
        self.bus_width   = bus_width

        # FIXME: These should be read from the board, rather than hardcoded!
        self.upload_chunk_size = 2048
        self.samples_max       = 32 * 1024


    def set_sample_rate(self, sample_rate):
        """ Updates the generator's sample rates. """
        self.sample_rate = int(sample_rate)


    def _upload_samples(self, samples):
        """ Uploads a collection of samples into the board's sample memory; precedes scan-out of those samples. """
        # Iterate over the full set of provided samples, uploading them in chunks.
        for offset in range(0, len(samples), self.upload_chunk_size):
            chunk = samples[offset:offset + self.upload_chunk_size]
            self.api.upload_samples(offset, chunk)


    def scan_out_pattern(self, samples, repeat=True):
        """ Sends a collection of fixed samples to the board, and then instructs it to repeatedly """

        samples = bytes(samples)

        # Upload the samples to be scanned out...
        self._upload_samples(samples)

        # ... and then trigger the scan-out itself.
        self.api.generate_pattern(self.sample_rate, self.bus_width, len(samples), repeat)


    def stop(self):
        """ Stops the board from scanning out any further samples. """
        self.api.stop()


    def dump_sgpio_config(self, include_unused=False):
        """ Debug function; returns the board's dumped SGPIO configuration. """

        self.api.dump_sgpio_configuration(include_unused)
        return self.board.read_debug_ring()

