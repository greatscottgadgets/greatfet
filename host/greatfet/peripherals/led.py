#
# This file is part of GreatFET
#

from ..protocol import vendor_requests
from ..peripheral import GreatFETPeripheral

class LED(GreatFETPeripheral):
    """ Simple periheral that allows control of an LED through the GreatFET HAL."""

    # The LED request codes supported by the GreatFET.
    LED_REQUEST_TOGGLE = 0
    LED_REQUEST_CLEAR  = 1
    LED_REQUEST_SET    = 2

    def __init__(self, board, led_number):
        """Create a new object representing a GreatFET LED.

        board -- The GreatFET board object that owns the given LED.
        led_number -- The one-indexed LED number. On GreatFET boards, this
                matches the number printed on the silkscreen.
        """

        # Store a reference to the parent board.
        self.board = board

        # Store which of the four(?) LEDs we refer to.
        # TODO: Validate this?
        self.led_number = led_number


    def _led_request(self, request_type):
        """ Internal function that issues a request to the given board for this LED.

        request_type -- The type of request to be issued. Should be one of the 
                LED_REQUEST_xxx properties of this class.
        """
        self.board.vendor_request_out(vendor_requests.SET_LEDS, index=request_type, value=self.led_number)


    def set(self, on=True):
        """ Sets the value of the LED.

        on -- True-like to turn the LED on, or false-like to turn the LED off.
            Optional, so you can use this function as .set() to set the LED to on.
        """

        if on:
            self._led_request(self.LED_REQUEST_SET)
        else:
            self._led_request(self.LED_REQUEST_CLEAR)


    def clear(self):
        """ Convenience function that turns the relevant LED off. """
        self.set(False)


    def toggle(self):
        """ Function that toggles the relevant LED value. """
        self._led_request(self.LED_REQUEST_TOGGLE)




