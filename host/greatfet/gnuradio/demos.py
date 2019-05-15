#
# Simple example/demonsration blocks.
#

import numpy as np
from gnuradio import gr

from greatfet import GreatFETSingleton

class gpio_pin_sink(gr.sync_block):
    """ Demonstration sink that drives a single GPIO pin. """

    def __init__(self, pin="J1_P4", led=2):
        gr.sync_block.__init__(
            self,
            name='GPIO Sink',   # will show up in GRC
            in_sig=[np.uint8],
            out_sig=None
        )

        greatfet = GreatFETSingleton()
        self.previous_state = None

        self.set_up_gpio(pin)
        self.set_up_led(led)


    def set_up_gpio(self, pin):
        try:
            self.gpio = greatfet.gpio.get_pin(pin)
            self.gpio.set_direction(self.gpio.DIRECTION_OUT)
        except:
            self.gpio = None

    def set_up_led(self, led):
        try:
            self.led = greatfet.leds[led]
        except:
            self.led = None

    def toggle_gpio(self):
        if self.gpio:
            self.gpio.write(not self.gpio.read())
        if self.led:
            GreatFETSingleton().leds[1].toggle()


    def process_sample(self, state):

        # If this is a rising edge, toggle the relevant GPIO pin.
        if (self.previous_state == 0) and state:
            self.toggle_gpio()

        # Store the GPIO state.
        self.previous_state = state


    def work(self, input_items, output_items):
        inputs = input_items[0]

        for i in inputs:
            self.process_sample(i)

        return len(input_items)

