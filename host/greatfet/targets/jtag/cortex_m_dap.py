#
# This file is part of GreatFET
#

from ...interfaces.jtag import JTAGDevice


class CortexMDebugConnection(JTAGDevice):
    """ Class representing an ARM Cortex-M debug connection. """

    # Accessed from: https://developer.arm.com/docs/103490233/10/the-jtag-idcode-for-a-cortex-processor
    SUPPORTED_IDCODES = [
        0x0BA01477, # Cortex-M0 / Cortex-M0+
        0x4BA00477, # Cortex-M3 / Cortex-M4
        0x0BA02477, # Cortex-M7
        0x0BA05477, # Cortex-M23
        0x0BA04477, # Cortex-M33
    ]

    DESCRIPTION = "ARM Cortex-M debug/programming access port"
