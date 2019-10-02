#
# This file is part of GreatFET.
#


from nmigen import Signal, Module, Elaboratable, ClockDomain, ClockSignal
from greatfet.neighbors.foxgen.platform.foxglove_r0_1 import FoxglovePlatformR01

class Blinky(Elaboratable):
    """ Hardware module that validates basic Foxglove functionality. """


    def elaborate(self, platform):
        """ Generate the SF tester. """

        m = Module()

        # Grab our I/O connectors.
        clk    = platform.request("clk2")
        port_b = platform.request("port_b_bus", dir="o")

        # TODO: also blink r0.2+'s LEDs

        # Grab our clock signal, and attach it to the main clock domain.
        m.domains.sync = ClockDomain()
        m.d.comb += ClockSignal().eq(clk.i)

        # Increment port_b every clock cycle, for now.
        m.d.sync += port_b.o.eq(port_b.o + 1)

        # Return our elaborated module.
        return m


if __name__ == "__main__":
    platform = FoxglovePlatformR01()
    platform.build(Blinky(), do_program=True)



