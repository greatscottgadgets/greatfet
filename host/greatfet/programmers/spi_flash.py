#
# This file is part of GreatFET
#

from ..programmer import GreatFETProgrammer
from .firmware import DeviceFirmwareManager

from pygreat.comms import CommandFailureError

def create_programmer(board, *args, **kwargs):
    """ Creates a representative programmer for the given module. """

    # For now, always create an SSPI interface.
    # We can take an 'interface' argument later to differentiate.
    return SPIFlash(board, *args, **kwargs)


class SPIFlash(DeviceFirmwareManager, GreatFETProgrammer):
    """ Class representing an SPI flash connected to the GreatFET. """

    #
    # Common JEDEC manufacturer IDs for SPI flash chips.
    #
    JEDEC_MANUFACTURERS = {
        0xFF: "unknown",
        0x01: "AMD/Spansion/Cypress",
        0x04: "Fujitsu",
        0x1C: "eON",
        0x1F: "Atmel/Microchip",
        0x20: "Micron/Numonyx/ST",
        0x37: "AMIC",
        0x62: "SANYO",
        0x89: "Intel",
        0x8C: "ESMT",
        0xA1: "Fudan",
        0xAD: "Hyundai",
        0xBF: "SST",
        0xC2: "Micronix",
        0xC8: "Gigadevice",
        0xD5: "ISSI",
        0xEF: "Winbond",
        0xE0: 'Paragon',
    }

    #
    # Common JEDEC device IDs. Prefixed with their manufacturer for easy / unique lookup.
    #
    JEDEC_PARTS = {
        0xFFFFFF: "unknown part",
        0xEF3015: "W25X16L",
        0xEF3014: "W25X80L",
        0xEF3013: "W25X40L",
        0xEF3012: "W25X20L",
        0xEF3011: "W25X10L",
        0xEF4015: "W25Q16DV",
        0xEF4018: "W25Q16DV",
        0xC22515: "MX25L1635E",
        0xC22017: "MX25L6405D",
        0xC22016: "MX25L3205D",
        0xC22015: "MX25L1605D",
        0xC22014: "MX25L8005",
        0xC22013: "MX25L4005",
        0xC22010: "MX25L512E",
        0x204011: "M45PE10",
        0x202014: "M25P80",
        0x1f4501: "AT24DF081",
        0x1C3114: "EN25F80",
        0xE04014: "PN25F08",
    }


    #
    # Common JEDEC capacity values.
    #
    JEDEC_CAPACITIES = {
        0x18: 0x1000000,
        0x17: 0x800000,
        0x16: 0x400000,
        0x15: 0x200000,
        0x14: 0x100000,
        0x13: 0x080000,
        0x12: 0x040000,
        0x11: 0x020000,
        0x10: 0x010000,
    }



    def _stuck_signal_check(self, fail_on_null_jedec_id=True):
        """ Attempts to detect a stuck MISO value; raises an IOError if one is detected. """

        # Attempt to read the raw values for the device's ID...
        info = self.api.query_device_id()

        # ... and check to see if it's stuck at 1/0
        if fail_on_null_jedec_id and (info == (0xff, 0xffff, 0xff)):
            raise IOError("spiflash: this target lacks a JEDEC ID, or DO/MISO appears to be stuck at '1'; check your connections?")
        if info == (0x00, 0x0000, 0x00):
            raise IOError("spiflash: DO/MISO appears to be stuck at '0'; check your connections")



    def _parse_jedec_id(self):
        """ Attempts to extract manufacturer name and string from JEDEC information. """

        manufacturer, part, capacity = self.api.query_device_id()

        # Correct the endianness of the part number.
        part = (part & 0xff) << 8 | (part >> 8)

        # Build a unique value that identifies the part and manufacturer -- this allows us to
        full_id = manufacturer << 16 | part

        self.manufacturer_id = manufacturer
        self.part_id = part
        self.jedec_capacity = capacity

        # Populate our information strings.
        self.manufacturer = self.JEDEC_MANUFACTURERS[manufacturer]  \
            if (manufacturer in self.JEDEC_MANUFACTURERS) else "unrecognized manufacturer"

        # If we don't currently have a capacity, and
        if (not self.maximum_address) and (capacity in self.JEDEC_CAPACITIES):
            self.maximum_address = self.JEDEC_CAPACITIES[capacity]

        self.part = self.JEDEC_PARTS[full_id] if (full_id in self.JEDEC_PARTS) else "unrecognized part"



    def __init__(self, board, autodetect=True, allow_fallback=False,
            page_size=256, pages=8192, maximum_address=None, allow_null_jedec=False,
            device_id=0, chip_select_port=0, chip_select_pin=15, force_page_size=None):
        """Set up a new SPI flash connection.

        Args:
            board -- The GreatFETBoard that will be programming our flash chip.
            autodetect -- If True, the API will attempt to automatically detect the flash's parameters.
            allow_fallback -- If False, we'll fail if autodetect is set and we can't autodetect the flash's paramters.
                If true, we'll fall-back to the keyword arguments provided
        """

        # Store a reference to the parent board, via which we'll program the
        # the actual SPI flash.
        self.board = board
        self.api = board.apis.spi_flash
        self.comms = board.comms
        self.capacity = 0

        # If autodetect is set to True, we'll try to automatically detect the device's topology.
        if autodetect:

            # Override the device ID to accept anything, as we're not tied to a specific device.
            device_id = 0

            self.api.initialize(0, 0, 0, chip_select_port, chip_select_pin, device_id)
            try:
                page_size, pages, size_in_bytes = self.api.query_topology()
                maximum_address = size_in_bytes - 1
            except CommandFailureError:

                # If we weren't able to read the SPDF, we either want to fall back to our
                # keyword arguments, or to throw an exception.
                if not allow_fallback:
                    raise
                else:
                    # If we do fall back, then check to make sure none of our signals are stuck at 1/0.
                    self._stuck_signal_check(fail_on_null_jedec_id=not allow_null_jedec)


        # If we have an argument forcing a given page size, override everything
        # we've worked out above with the forced page size.
        if force_page_size:
            total_size = page_size * pages
            page_size = force_page_size

            # Adjust the number of pages accordingly.
            pages = total_size // page_size


        # Store the limitations for this SPI flash.
        self.page_size = page_size
        self.maximum_address = maximum_address if maximum_address else (page_size * pages) - 1
        self.api.initialize(page_size, pages, page_size * pages, chip_select_port, chip_select_pin, device_id)

        # Extract manufacturer and device info, where possible.
        self._parse_jedec_id()

        # TODO: re-initialize the flash if the JEDEC identification updates our capacity?
        # TODO: decide if we want to do ^


