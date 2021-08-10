================================================
GreatFET Classes
================================================

The new *GreatFET Communications Protocol* uses a simple Class/Verb scheme to organize protocol-independent commands. **Classes** organize groups of related functions, while **verbs** provide the functions themselves. This very much parallels the organization of GoodFET commands.

Examples of *classes*:

    - SPI functionality
    - FaceDancer ("GreatDancer") functionality
    - Debug utilities
    - GPIO

Examples of *verbs*:

    - Configure the SPI bus to set e.g. polarity
    - Read a USB1 status register (for consumption by the FaceDancer host)
    - Return the contents of the device's debug ring
    - Set a particular pin to a given value

Each class is identified by a unique **32-bit integer**. Each verb is identified by a *separately-namespaced* **32-bit integer**. To add a class to GreatFET, first reserve its number in the Class Registry below.



Class Registry
~~~~~~~~~~~~~~

.. list-table ::
  :header-rows: 1
  :widths: 1 1 2

  * - Class Number
    - Class Name
    - Description 
  * - 0x0 
    - core
    - core device identification functionality
  * - 0x1
    - firmware
    - verbs for working with/updating device firmware
  * - 0x10
    - debug
    - debug utilities
  * - 0x11
    - selftest
    - utilities for self-testing libgreat-based boards
  * - 0x100   
    - example  
    - example verbs meant to illustrate the comms protocol
  * - 0x101   
    - spi_flash  
    - verbs for programming SPI flashes
  * - 0x102   
    - heartbeat  
    - control the GreatFET's idle/"heartbeat" LED 
  * - 0x103   
    - gpio  
    - raw control over the GreatFET's GPIO pins
  * - 0x104   
    - greatdancer  
    - remote control over the GreatFET's USB1 port in device mode, for e.g. FaceDancer 
  * - 0x105   
    - usbhost  
    - remote control over the GreatFET's USB port in host mode, for e.g. FaceDancer
  * - 0x106   
    - glitchkit  c
    - control over the GlitchKit API, and control over simple triggers
  * - 0x107   
    - glitchkit_usb  
    - control over functionality intended to help with timing USB fault injection
  * - 0x108   
    - i2c  
    - communication as an I2C controller
  * - 0x109   
    - spi  
    - communication as SPI controller
  * - 0x10A   
    - leds 
    - control over a given board's LEDs
  * - 0x10B   
    - jtag  
    - functions for debugging over JTAG
  * - 0x10C   
    - jtag_msp430  
    - MSP430 specific JTAG functions
  * - 0x10D   
    - logic_analyzer  
    - allows one to use the GreatFET's SGPIO interface as a logic analyzer
  * - 0x10E   
    - sdir  
    - Functionality for Software Defined Infrared
  * - 0x10F   
    - usbproxy  
    - Firmware functionality supporting USBProxy
  * - 0x110   
    - pattern_generator  
    - allows one to use the GreatFET's SGPIO interface as a pattern generator
  * - 0x111   
    - adc  
    - analog to digital converter functionality
  * - 0x112   
    - uart  
    - functionality for talking 'serial'
  * - 0x113   
    - usb_analysis  
    - functionality for USB analysis e.g. with Rhododendron
  * - 0x114   
    - swra124  
    - debugging/programming for TI cc111x, cc243x, and cc251x
  * - 0x115   
    - loadables  
    - API for loading and running small firmware extensions
  * - 0x116   
    - clock_gen  
    - clock generation / control
  * - 0x117   
    - benchmarking  
    - measurement of GreatFET communications and functions
  * - 0x118   
    - can  
    - functionality for communication over CAN
  * - 0x119
    - SWD
    - functionality for communicating with ARM SWD interfaces
