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

    # Default to having a uint8_t type for
    OUT_SIGNATURE=[np.uint8]

    # The default timeout for the work function's USB transactions. Usually does not need
    # to be overridden.
    WORK_TIMEOUT=100

    def __init__(self, sample_rate, *args, **kwargs):
        """
            Basic block initialization for GreatFET blocks.
        """

        gr.sync_block.__init__(
            self,
            name=self.BLOCK_NAME,
            in_sig=None,
            out_sig=self.OUT_SIGNATURE,
        )


        self.sample_rate = sample_rate

        # Create our core USB transaction buffer.
        # This will go away as soon as we switch to python-libusb1.
        self.buffer   = array.array('B', bytes(4096))

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


    def work(self, input_items, output_items):
        """ Core work function for our streaming blocks. """

        out = output_items[0]

        # Try to read the what data we can from the GreatFET's streaming pipe.
        # TODO: upgrade this to use python-libusb1 and the new comms API
        num_samples = self.gf.comms.device.read(self.pipe_id, self.buffer, self.WORK_TIMEOUT)
        samples = self.buffer[0:num_samples]

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

        # Finally, pass our samples to GNURadio.
        out[:len(samples)] = samples
        return len(samples)


    def stop(self, *args):
        """ Function called when we're halting execution of our flowgraph. """
        self.tear_down_streaming()
        return True
