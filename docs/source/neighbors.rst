.. _neighbors:

================================================
Neighbors
================================================

already designed
~~~~~~~~~~~~~~~~

    - `Begonia <https://github.com/greatfet-hardware/begonia>`__: a GIMME for GreatFET
    - `Crocus <https://github.com/greatfet-hardware/crocus>`__: nRF24L01+
    - `Daffodil <https://github.com/greatfet-hardware/daffodil>`__: through-hole prototyping
    - `Scorzonera <https://github.com/miek/scorzonera>`__: interface for FLIR PM-series thermal cameras (AM7969 & RS232)
    - `Spot <https://github.com/straithe/NeighbourSpotHardware>`__: a chonky indicator LED

some progress made
~~~~~~~~~~~~~~~~~~

    - `Canna <https://github.com/tarikku/canna-greatfet-neighbor>`__: basic 2-channel CAN neighbor
    - Edelweiss: CC1310 (depending on progress, could consider newer cc1352 that supports more)
    - `Foxglove <https://github.com/greatfet-hardware/foxglove>`__: advanced level shifting and probing (support for +/- 12v would be awesome...)
    - `Gladiolus <https://github.com/greatfet-hardware/gladiolus>`__: IR
    - Heliopsis: googly eyes
    - Hydrangea: NFC
    - `Indigo <https://github.com/greatscottgadgets/greatfet/wiki/Indigo>`__: ChipWhisperer 20-Pin Connector + Voltage Glitching
    - `Jasmine <https://github.com/greatfet-hardware/jasmine>`__: Lithium Polymer battery pack/charger
    - `Kniphofia <https://github.com/greatscottgadgets/greatfet/wiki/Kniphofia>`__: iCE40 FPGA capable of remapping neighbor pins
    - `Lily <https://github.com/greatfet-hardware/lily>`__: RF power detector
    - `Lucky Bamboo <https://github.com/greatfet-hardware/luckybamboo>`__: quadruple 2.4 GHz ADF7242 transceiver for monitoring Bluetooth LE advertising channels
    - Magnolia: TDR, dual ethernet (LAN tap)
    - Narcissus: a jig for testing GreatFET One
    - `Orchid <https://github.com/greatscottgadgets/greatfet/wiki/Orchid>`__: Breakout board for common hardware hacking interfaces
    - `Peony <https://github.com/greatscottgadgets/greatfet/wiki/Peony>`__: SDR based on AT86RF215IQ
    - `Quince <https://github.com/greatfet-hardware/quince>`__: 2.4 GHz SDR using 1 bit ADC over SGPIO
    - Rhododendron: Hi-Speed USB passive sniffer
    - Stellaria: experimental random number generator
    - Tulip: an awesome blinky neighbor
    - `Ursinia <https://github.com/greatfet-hardware/ursinia>`__: Grove base

rough ideas
~~~~~~~~~~~

    - RNG
    - DAQ
    - pulse generation (TTL)
    - arbitrary waveform generation
    - magstripe
    - barcode
    - dual cc1101
    - cc1200 + cc2500 for rfcat-like functionality in Sub-GHz and 2.5 GHz
    - cc1352 for WiFi, BLE 5, Zigbee, Thread, Wireless M-Bus, 802.15.4g, 6LoWPAN, KNX RF, Wi-SUN®, and 2FSK/4FSK proprietary protocols
    - Fieldbus Neighbor with CAN, RS-232, TIA-422, TIA-485 (possibly support for two of these neighbors operating at the same time for MitM and other dual interface abilities)
    - ODB-II ?
    - DTMF
    - NFC/RFID
    - DPA
    - JTAG (like bus blaster and/or Black Magic Probe) (could be satisfied by Foxglove)
    - SX1257 (SDR, no FPGA needed)
    - SD card
    - clock generator (for multi-HackRF)
    - ultrasound
    - USB Type-C sniffer
    - LCD
    - nRF8001
    - general comms: Ethernet, Bluetooth, Wi-Fi, etc.?
    - one or more of the Cypress Wi-Fi and/or Bluetooth chips formerly owned by Broadcom
    - KNX
    - mmWave sensor/radar
    - energy harvesting (buck/boost)
    - USB Type-C power delivery, accessory mode, alternate mode breakout/sniff/hack
    - weather station
    - zero insertion force neighbor
