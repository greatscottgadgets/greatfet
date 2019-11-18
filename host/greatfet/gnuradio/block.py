#
# Base block for GreatFET GRC blocks.
#

import ast
import array

import numpy as np
from gnuradio import gr

from greatfet import GreatFETSingleton

class GreatFETStreamingSource(gr.sync_block):

    # The name for the block. Should be overridden by derived classes.
    BLOCK_NAME='GreatFET Source'

    # Default to having a float type for our output, as this is easiest
    # to work with in GNURadio.
    OUTPUT_TYPE=np.float32

    # If provided, the default sample processing will normalize a sample with a given
    # value to 1.0. If None, the sample will be left as is.
    OUTPUT_MAX_SCALE=None

    # The default timeout for the work function's USB transactions. Usually does not need
    # to be overridden.
    WORK_TIMEOUT=100

    # Default sample size and endianness.
    # Subclasses can either override this function or override the
    SAMPLE_SIZE_BYTES = 1
    SAMPLE_ENDIANNESS = 'little'

    # Default to an unsigned sample.
    SAMPLE_SIGNED = False


    def __init__(self, sample_rate, *args, **kwargs):
        """
            Basic block initialization for GreatFET blocks.
        """

        gr.sync_block.__init__(
            self,
            name=self.BLOCK_NAME,
            in_sig=None,
            out_sig=[self.OUTPUT_TYPE],
        )


        self.sample_rate = sample_rate

        # Create our core USB transaction buffer.
        # This will go away as soon as we switch to python-libusb1.
        self.buffer  = array.array('B', bytes(4096))

        # Create an array object that will store any pending samples received from the GreatFET.
        # This should probably be made a bytearray once we switch to python-libusb1.
        self.pending_samples = array.array('B')

        # Get a handle on the GreatFET object being used by this session.
        self.gf = GreatFETSingleton()

        # Store our arguments for later use.
        self.args = args
        self.kwargs = kwargs


    def start(self):
        # Start the streaming ourself by calling the set_up_streaming method.
        # Pass in any leftover arguments to our constructor.
        self.pipe_id = self.set_up_streaming(*self.args, **self.kwargs)
        return True



    def set_up_streaming(self):
        """
        The core unique definition for our GNURadio blocks. Accepts the initializer arguments
        after our sample rate; and should set up any streaming that needs to happen.

        Must be overridden by any descendant classes.
        """
        raise NotImplementedError()


    def tear_down_streaming(self):
        """
        Function called when the GreatFET source is being disposed of. Should be used to clean up
        any claimed hardware resources. Implementation is optional.
        """
        pass


    def get_sample_size(self):
        """
        Returns the size of a normal sample, in bytes. This default implementation returns SAMPLE_SIZE_BYTES,
        but blocks with runtime-configurable sample size should override this function.
        """
        return self.SAMPLE_SIZE_BYTES


    def get_sample_endianness(self):
        """
        Returns the endianness of a normal sample; either 'big' or 'little. This default implementation
        returns SAMPLE_ENDIANNESS, but blocks with runtime-configurable sample endianness should override this function.
        """
        return self.SAMPLE_ENDIANNESS


    def samples_are_signed(self):
        """
        Returns true iff samples should be interpreted as signed.
        Normally returns SAMPLED_SIGNED, but configurable blocks can override this method.
        """
        return self.SAMPLE_SIGNED


    def get_sample_max_scale(self):
        """
        Returns the maximum value we should expect from a register read. This default implementation
        returns OUTPUT_MAX_SCALE, but blocks with runtime-configurable maximum should override this.

        This can return None to avoid scaling altogether.
        """
        return self.OUTPUT_MAX_SCALE


    def process_samples(self, samples):
        """
        Function that processes incoming samples into a format acceptable to GNURadio.
        By default, processes samples into integers based on their size in bytes.
        """

        # Convert our samples into a mutable array of bytes.
        samples = bytearray(samples)

        sample_size = self.get_sample_size()
        endianness  = self.get_sample_endianness()

        # Figure out how many entries we'll have in our new array
        new_array_size = len(samples) // sample_size

        # Create a new array of samples...
        new_samples = np.array([0] * new_array_size, dtype=self.OUTPUT_TYPE)

        # ... and populate it.
        sample_index = 0
        while samples:
            raw_sample = samples[0:sample_size]
            del samples[0:sample_size]

            # Process our sample...
            sample = int.from_bytes(raw_sample, byteorder=endianness, signed=self.samples_are_signed())

            if self.get_sample_max_scale():
                sample = sample / self.get_sample_max_scale()

            # ... and move to the next sample.
            new_samples[sample_index] = sample
            sample_index += 1

        return new_samples


    def work(self, input_items, output_items):
        """ Core work function for our streaming blocks. """

        out = output_items[0]

        # Try to read the what data we can from the GreatFET's streaming pipe.
        # TODO: upgrade this to use python-libusb1 and the new comms API
        num_sample_bytes = self.gf.comms.device.read(self.pipe_id, self.buffer, self.WORK_TIMEOUT)
        samples = self.buffer[0:num_sample_bytes]

        # If we have samples left over from last time, use them.
        if self.pending_samples:
            samples = self.pending_samples + samples

        # If we don't have any samples, yet, return an empty length.
        if not samples:
            return 0

        # If we received more samples over USB than GNURadio is ready to handle, save the
        # additional samples for the next work iteration, ensuring we're delivering at a steady
        # rate.
        # TODO: cap this to make sure we don't have uncontrolled overrun?
        if len(samples) > len(out):
            self.pending_samples = samples[len(out):]
            samples = samples[:len(out)]

        # Perform any sample processing we need to do before sending these samples upstream.
        samples = self.process_samples(samples)

        # Finally, pass our samples to GNURadio.
        out[:len(samples)] = samples
        return len(samples)


    def stop(self, *args):
        """ Function called when we're halting execution of our flowgraph. """
        self.tear_down_streaming()
        return True


    def handle_preludes(self, prelude, prelude_script):
        """ Helper function that runs any prelude code specified by the given GRC block. """

        context = {'gf': self.gf}

        # If we have a direct prelude text, execute it.
        if prelude:
            exec(prelude, context)

        # ... and if we have a script, load it and execute it.
        if prelude_script:
            with open(prelude_script) as f:
                exec(f.read(), context)

