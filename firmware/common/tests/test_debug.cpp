/*
 * This file is part of GreatFET
 */

#include "catch.hpp"

extern "C" {
    #include <debug.h>
}


SCENARIO("lines are being added and removed to a ringbuffer", "[ringbuffer]") {

    GIVEN("an empty ringbuffer") {
        debug_init();
        char test_data[2040];
        char test_string[] = "hi\n";

        WHEN("a single piece of data is added") {

            // Set up some initial data.
            for (unsigned i = 0; i < sizeof(test_data); i++)
                test_data[i] = (i & 0x7f);

            debug_ring_write(test_data, sizeof(test_data));

            THEN("the size increses to match") {
                REQUIRE(debug_ring_used_space() == sizeof(test_data));
            }

            THEN("reading from the ringbuffer returns the data added") {
                debug_ring_read(test_data, sizeof(test_data), false);
                for (int i = 0; i < sizeof(test_data); i++) {
                    if ((int)test_data[i] != (i & 0x7f))
                        FAIL("ringbuffer starts to mismatch at index " << i);
                }
            }
        }

        WHEN("a string is added") {
            debug_ring_write_string(test_string);

            THEN("the size increses to match") {
                REQUIRE(debug_ring_used_space() == strlen(test_string));
            }
        }

    }
    GIVEN("a ringbuffer with some data") {
        debug_init();
        char test_data[2040];
        debug_ring_write(test_data, sizeof(test_data));

        WHEN("data is read, but not removed") {
            debug_ring_read(test_data, 1000, false);

            THEN("the size doesn't change") {
                REQUIRE(debug_ring_used_space() == sizeof(test_data));
            }
        }

        WHEN("a single piece of data is removed") {
            debug_ring_read(test_data, 1000, true);

            THEN("the size decreases to match") {
                REQUIRE(debug_ring_used_space() == (sizeof(test_data) - 1000));
            } 
        }
    }

    GIVEN("a ringbuffer that's almost full") {
        debug_init();
        char test_string[] = "hello, I'm a string of a small size!\n";
        char test_data[2048 - sizeof(test_string) - 10];

        // populate some test data
        memset(test_data, 'A', sizeof(test_data));
        debug_ring_write_string(test_string);
        debug_ring_write(test_data, sizeof(test_data));

        WHEN("more data than free space is added") {
            char new_string[] = "I'm long enough to trigger reclamation!";
            debug_ring_write_string(new_string);

            THEN("the first line is reclaimed") {
                REQUIRE(debug_ring_used_space() == sizeof(test_data) + strlen(new_string));
            }

            THEN("the ringbuffer now starts with the data after reclamation") {
                debug_ring_read(test_data, sizeof(test_data), false);
                for (int i = 0; i < sizeof(test_data); ++i) {
                    if (test_data[i] != 'A') {
                        FAIL("data mismatch at index " << i << " (expected 'A' but got '" << test_data[i] << "')" );
                    }
                }
            }

            THEN("the new data can be read from the ringbuffer") {
                debug_ring_read(test_data, sizeof(test_data), true);
                debug_ring_read(test_data, strlen(new_string), false);

                for (int i = 0; i < strlen(new_string); ++i) {
                    if (test_data[i] != new_string[i]) {
                        FAIL("data mismatch at index " << i << " (expected '" << new_string [i] << "' but got '" << test_data[i] << "')" );
                    }
                }
            }


        }
    }
}
