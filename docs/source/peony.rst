================================================
Peony
================================================

Peony is a Software-Defined Radio (SDR) neighbor featuring the AT86RF215IQ transceiver.

    - Operating Frequency Range: 1 MHz to 2.5 GHz
    - Analog Bandwidth: 2 MHz
    - Sample Rate: 4 Msps
    - Dynamic Range: 13 bits I and 13 bits Q

Peony is half-duplex, although we may be able to support full-duplex operation in the specific frequency bands supported by the AT86RF215 (389.5 MHz to 510 MHz, 779 MHz to 1020 MHz, 2400 MHz to 2483.5 MHz).

A front-end mixer (probably RF2052) in an arrangement similar to that found on HackRF One provides coverage of frequencies outside the bands supported by the AT86RF215. We might decide to omit the mixer, simplifying Peony and limiting its operating frequency range to the bands supported by the AT86RF215.

A small FPGA (probably an ECP5) serves as an interface between the serial LVDS I/Q interface of the AT86RF215 (up to two simultaneous lanes of DDR LVDS at 128 Mbit/s) and a parallel interface on the LPC4330 on Azalea. The interface with the LPC4330 will be either SGPIO or EMC (TBD).

    - Clocking TBD
    - Limited full-duplex mode TBD
    - Stacking of multiple Peonies TBD
    - Maximum TX power TBD but probably between 10 dBm and 14 dBm
    - Maximum RX power TBD but probably between 0 dBm and 10 dBm

