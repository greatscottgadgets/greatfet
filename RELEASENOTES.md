This the fourth major release of the GreatFET software and firmware stacks. As usual, this release contains firmware images in `firmware-bin`, host software in `host-packages`, and a copy of all sources necessary to build the entire codebase.

## Upgrading to this release

You can upgrade the GreatFET host tools to the latest release with:

    pip install --upgrade greatfet

After upgrading the host tools, update your GreatFET firmware to the latest release with:

    greatfet_firmware --autoflash

Happy hacking!


# Changelog

## v2024.0.1

### GreatFET

* Add IPython to the GreatFET installation by default.
* Update udev rules to use `uaccess` tag rather than the `plugdev` group.
* Board support: [Update the rad1o board file](https://github.com/greatscottgadgets/greatfet/pull/432) (tx @dos1!)

## v2024.0.0

### GreatFET

* [gpio: add support for configuring all gpio pin modes](https://github.com/greatscottgadgets/greatfet/pull/418)
* [uart: fix python KeyError when parity argument not specified](https://github.com/greatscottgadgets/greatfet/pull/375)
* [shell: support versions of IPython >= 3.11](https://github.com/greatscottgadgets/greatfet/pull/414)
* [facedancer: fix usb mass storage example](https://github.com/greatscottgadgets/greatfet/pull/425)
* [shell: fix runtime errors when using uart functionality](https://github.com/greatscottgadgets/greatfet/pull/426)

### libgreat

* [Added TX & RX pin definitions for UART1, USART2&3](https://github.com/greatscottgadgets/libgreat/pull/25)
* [Implement NXP's recommended PLL setup sequence](https://github.com/greatscottgadgets/libgreat/pull/30)
* [Add support for configuring all gpio pin modes](https://github.com/greatscottgadgets/libgreat/pull/35)
* [Don't try to claim USB interface on Windows](https://github.com/greatscottgadgets/libgreat/pull/38)
* [Add initial implementation of usb1 bulk transfer backend](https://github.com/greatscottgadgets/libgreat/pull/33)
* [Do not set USB device address to zero](https://github.com/greatscottgadgets/libgreat/pull/26)


## v2021.2.1

NOTE: We no longer support Python 2. This release targets Python 3.6+.

### Highlights for this release:

* This release adds a new chipcon programmer for Texas Instruments CCxxxx ("SWRA124") devices.
    - In Python: `chipcon = gf.create_programmer('chipcon')`
        - From there you can perform manual operations like `chipcon.run_instruction()` and `read_xdata_memory()`,
        - Or use higher-level interface methods like `program_flash()` and `read_flash()`.
    - Flash operations can also be done from the command line with `greatfet chipcon`.
* This release adds a new programmer for Microchip I2C EEPROM devices: `eeprom = gf.create_programmer('microchipEEPROM', device='24LC128')`.
    - This mainly provides two higher-level methods: `read_bytes(start, end)`, and `write_bytes(start, data)`.

### Major bugfixes:

 - #344: Facedancer with bMaxPacketSize0 < 32 does not work.
