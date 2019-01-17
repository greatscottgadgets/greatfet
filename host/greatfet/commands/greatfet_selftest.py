#!/usr/bin/env python3
#
# This file is part of GreatFET.
#
"""
Self-testing framework for GreatFET hardware.
"""

import math
import unittest

from greatfet import GreatFET
from greatfet.utils import GreatFETArgumentParser


class ValidateBasicComms(unittest.TestCase):

    def test_comms(self):
        """ Test of basic communications with host """
        gf.serial_number()


class ValidateSystemClocks(unittest.TestCase):
    """
    Ensures each of the GreatFET's clocks are up and running.
    """

    # Specifies the allowed error before a clock is considered noconforming.
    DEFAULT_TOLERANCE = 0.01

    # The clock source we'll test.
    SYSTEM_CLOCK_SOURCES = {
        0x6: { 'name': 'crystal oscillator',   'frequency':  12e6 },
        0x7: { 'name': 'USB PLL',              'frequency': 480e6 },
        0x9: { 'name': 'main PLL',             'frequency': 204e6 },
        0xd: { 'name': 'USB1 clock divider',   'frequency':  60e6 },
    }
    def test_system_clocks(self):
        """ Test of each of the system's clocks """
        global gf

        # Get our full list of clocks to check.
        clocks_to_check = self.SYSTEM_CLOCK_SOURCES.keys()

        # Ask the GreatFET for the frequency of each of the clocks.
        frequencies = gf.apis.selftest.measure_clock_frequencies(*clocks_to_check)

        # Validate that each of the frequencies make sense.
        for clock_number, measured_frequency in zip(clocks_to_check, frequencies):
            parameters = self.SYSTEM_CLOCK_SOURCES[clock_number]
            with self.subTest(parameters['name']):

                # Compute the amount the given clock deviates from its expected value.
                expected_frequency = parameters['frequency']
                error = abs(expected_frequency - measured_frequency) / float(expected_frequency)

                # Figure out how much tolerance we should allow.
                tolerance = parameters['tolerance'] if ('tolerance' in parameters) else self.DEFAULT_TOLERANCE

                # Validate that the relevant clock is within our tolerance.
                if error > tolerance:
                    measured_frequency_mhz = measured_frequency / 1e6
                    expected_frequency_mhz = expected_frequency / 1e6

                    # Validate the clock error is within acceptable bounds.
                    failure_message = \
                        "Clock {} did not provide expected frequency; {} MHz is off by {}% of expected {} MHz.".format(
                            parameters['name'], measured_frequency_mhz, error * 100, expected_frequency_mhz)
                    self.fail(failure_message)


def main():
    global gf

    # Set up a simple argument parser.
    parser = GreatFETArgumentParser( description="Runner for the GreatFET board self-tests.")

    # Parse the arguments, and connect to the relevant GreatFET.
    gf = parser.find_specified_device()

    # TODO: possibly reset the GreatFET _first_, to ensure it's in a known state?

    # Run our self-tests.
    unittest.main(verbosity=2)


if __name__ == '__main__':
    main()
