#
# This file is part of GreatFET.
#

from nmigen.build import *
from nmigen.vendor.lattice_ecp5 import *

from .... import GreatFET

__all__ = ["FoxglovePlatformR01"]



def PortAResource(name, number, io_site, oe_site, sf_connection_sites):
    """ Adds a pin resource for a Foxglove Port-A pin with connected level-shifter and switched connections. """

    return Resource(name, number,
        Subsignal("io", Pins(io_site, dir="io")),
        Subsignal("direction", Pins(oe_site, dir="o")),
        Subsignal("connections", Pins(sf_connection_sites, dir="o", assert_width=3), Attrs(PULLMODE="down")),
        Attrs(IO_TYPE="LVCMOS33")
    )


class FoxglovePlatformR01(LatticeECP5Platform):
    """ Board description for the pre-release r0.1 (29-July-2019) revision of Foxglove. """

    device      = "LFE5U-12F"
    package     = "BG256"
    speed       = "6"

    default_clk = "clk2"

    # Default our Port B voltage to 3V3.
    port_b_voltage = 3.3

    resources   = [

        # Clock pins that accept clock signals from the GreatFET.
        Resource("clk0", 0, Pins("P1", dir="i"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("clk2", 0, Pins("E9", dir="i"), Attrs(IO_TYPE="LVCMOS33")),

        # Primary SPI bus; as previously used for configuration. Used for simple
        # communications with the GreatFET.
        Resource("spi", 0,

            # Note: the SCK signal previously used for configuration is tied to the dedicated
            # CCLK pin, so the GreatFET will have to re-map this to J2_P21 in the SCU.

            Subsignal("sck",   Pins("B7", dir="i")),
            Subsignal("miso",  Pins("T7", dir="o")),
            Subsignal("mosi",  Pins("T8", dir="i")),
            Subsignal("cs_n",  Pins("R8", dir="i")),
            Attrs(IO_TYPE="LVCMOS33")
        ),


        # Provide a convenience mapping to our standard SGPIO pins.
        Resource("sgpio",  0, Pins("L1",  dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio",  1, Pins("M2",  dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio",  2, Pins("R13", dir="io"), Attrs(IO_TYPE="LVCMOS33")), #<-- not correctly routed; currently (our only SGPIO2 pin is tied to INIT_N)
        Resource("sgpio",  3, Pins("T13", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio",  4, Pins("C14", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio",  5, Pins("C13", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio",  6, Pins("B13", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio",  7, Pins("M1",  dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio",  8, Pins("T15", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio",  9, Pins("D8",  dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio", 10, Pins("D4",  dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio", 11, Pins("T14", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio", 12, Pins("E8",  dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio", 13, Pins("E6",  dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio", 14, Pins("C5",  dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("sgpio", 15, Pins("B3",  dir="io"), Attrs(IO_TYPE="LVCMOS33")), #<-- not correctly routed; currently a stand-in


        # Note that an awkward design decision by another engineer led to the ports
        # being numbered 1-8, matching their connector pins, rather than 0-7, which
        # matches the usual vector conventions. I'm choosing to enter these as 0-7,
        # as I plan on changing that in future silkscreen revisions. ~KT

        # Main connector for Port A; before the level shifter. The logic here is
        # always 3V3; the other side's voltage is set by the Vtarget regulator.
        PortAResource("port_a", 0, io_site="K16", oe_site="L14", sf_connection_sites="K14  K15  K16"),
        PortAResource("port_a", 1, io_site="J14", oe_site="J13", sf_connection_sites="G16  H15  J15"),
        PortAResource("port_a", 2, io_site="K13", oe_site="L15", sf_connection_sites="L16  M14  L13"),
        PortAResource("port_a", 3, io_site="G13", oe_site="M14", sf_connection_sites="G14  G15  F16"),
        PortAResource("port_a", 4, io_site="F15", oe_site="F14", sf_connection_sites="C15  D12  E12"),
        PortAResource("port_a", 5, io_site="E13", oe_site="E14", sf_connection_sites="D16  E15  E16"),
        PortAResource("port_a", 6, io_site="P13", oe_site="P12", sf_connection_sites="P14  P16  N11"),
        PortAResource("port_a", 7, io_site="N16", oe_site="N14", sf_connection_sites="M13  M15  M16"),


        # Main connector for Port B. The VCCIO for this logic can be set to a variety
        # of levels by software; so these attrs may need to be modified by software.
        Resource("port_b",     0, Pins("F1", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("port_b",     1, Pins("G2", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("port_b",     2, Pins("G1", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("port_b",     3, Pins("H2", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("port_b",     4, Pins("H4", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("port_b",     5, Pins("H5", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("port_b",     6, Pins("J1", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("port_b",     7, Pins("J2", dir="io"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("port_b_bus", 0, Pins("F1 G2 G1 H2 H4 H5 J1 J2", dir="io"), Attrs(IO_TYPE="LVCMOS33")),


        # The pins in Port B are routed to also support being used differentially.
        # Accordingly, we provide a duplicate with differential naming.
        Resource("port_b_diff", 0, DiffPairs("F1", "G2", dir="io"), Attrs(IO_TYPE="LVCMOS33D")),
        Resource("port_b_diff", 1, DiffPairs("G1", "H2", dir="io"), Attrs(IO_TYPE="LVCMOS33D")),
        Resource("port_b_diff", 2, DiffPairs("H4", "H5", dir="io"), Attrs(IO_TYPE="LVCMOS33D")),
        Resource("port_b_diff", 3, DiffPairs("J1", "J2", dir="io"), Attrs(IO_TYPE="LVCMOS33D")),

        # ADC scaling control.
        Resource("adc_pull_en",     0, PinsN("P3", dir="o"), Attrs(IO_TYPE="LVCMOS33")),
        Resource("adc_pull_up",     0,  Pins("P4", dir="o"), Attrs(IO_TYPE="LVCMOS33"))

        # TODO: add aliases for CLK0/1, I2C, SSP, SPI, UART pins
    ]

    # Neighbor headers.
    connectors = [

        # Top header (J2)
        Connector("J", 2, """
        -   D7  D6  E5  E8  E9  E10  C4  C5  C6  C7  C8  C9  C10  C11  C12  C13  C14  M3   C16
        -   E6  -   D4  D8  D9  B3   B4  B5  B6  B7  B8  B9  B10  B11  B12  B13  B14  B15  B16
        """),

        # Bonus row (J7)
        Connector("J", 7, """
        -  E7  E4  D5  A2  A3  A4  A5  A6  A7  A8  A9  A10  A11  A12  A13  A14  A15  -  -
        """),

        # Bottom header (J1)
        Connector("J", 1, """
        -  L1  M2  N3  P2  -   R1  T2  T3  T4  T6  N8  R12  -  T13  T14  T15  P15  -   T7
        -  K4  -   M1  N1  P1  -   R2  R3  R4  R5  R6  -    -  R13  R14  R15  R16  R8  T8
        """),

    ]

    def toolchain_prepare(self, fragment, name, **kwargs):
        overrides = {
            'ecppack_opts': '--idcode {}'.format(0x21111043)
        }
        return super().toolchain_prepare(fragment, name, **overrides, **kwargs)



    def toolchain_program(self, products, name):
        """ Programs the relevant Foxglove board with the generated build product. """

        # Create our connection to our Foxglove target.
        gf = GreatFET()
        foxglove = gf.attach_neighbor('foxglove')

        # Turn on VCCB, and set it to the provided voltage.
        if self.port_b_voltage is not None:
            foxglove.provide_vccb(self.port_b_voltage)

        # Grab our generated bitstream, and upload it to the FPGA.
        bitstream =  products.get("{}.bit".format(name))
        foxglove.configure_fpga(bitstream)


