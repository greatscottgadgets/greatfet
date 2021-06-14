================================================
Using GreatFET APIs
================================================

Getting a GreatFET object
~~~~~~~~~~~~~~~~~~~~~~~~~

A GreatFET object may be created with the following Python code:

.. code-block:: python

	import greatfet

	gf = greatfet.GreatFET()

``gf shell`` will do this automatically before spawning an IPython instance. ``greatfet.GreatFET()`` can also take the same keyword arguments as PyUSB's ``usb.find()``, to allow specifying a device e.g. by serial number.



Using APIs
~~~~~~~~~~

`LibGreat <https://github.com/greatscottgadgets/libgreat>`__ provides the general comms API for talking to LibGreat devices.

Generically, APIs can be accessed in Python as ``gf.apis.<api_name>``, e.g. ``gf.apis.firmware``, for the firmware API. Alternatively, the same objects can be accessed by API names as strings via the ``gf.comms.apis`` dict.

On the command line, you can list the APIs supported by a GreatFET with ``greatfet info -A``.

Some APIs are also additionally exposed as "interfaces" on a GreatFET object, e.g. ``gf.<interface>``. These are primarily for convenience—they provide default configurations and simplified interfaces for their relevant APIs.

Some APIs also have command line tools as helpers. These will be subcommands of ``greatfet`` (which can be shortened to ``gf``). Invoking ``greatfet`` without any arguments will list the currently supported subcommands.




Debugging
~~~~~~~~~

In the event that something does not goes as expected, you can run ``greatfet dmesg`` on the command line to get a log of events that have occured on the GreatFET. From Python, you can run ``gf.read_debug_ring()`` to get a string object for the same text.

The debug ring-buffer persists across soft resets (including e.g. firmware flashes), but *not* across hard-resets, like pressing the reset button or unplugging and replugging the device.

``gf.apis.debug.peek`` and ``gf.apis.debug.poke`` can also be used to read from and write to raw memory addresses, for advanced debugging.




LED
~~~

LEDs are a simple place to start. GreatFET Azalea has 4 LEDs, though LED 1 blinks on and off periodically as a "heartbeat" by default. *Note that the LEDs are 1-indexed!*

There is a convenience interface in Python:

.. code-block:: sh

	gf.leds[2].on()
	gf.leds[2].off()
	gf.leds[2].toggle()

As well as a command-line helper tool.

.. code-block:: sh

	$ greatfet led --on 2
	$ greatfet led --off 2
	$ greatfet led -t 2



GPIO
~~~~

The GPIO peripheral can be easily controlled from the convenience interface ``gf.gpio``.

``gf.gpio.read_pin_state((1, 6))`` will read the logic value for GPIO pin 1[6] (pin mappings for peripherals can be found `temporarily here <https://gf.ktemkin.com/>`__) Likewise ``gf.gpio.set_pin_state((1, 6), 0)`` can be used to set the same pin to logic low.




Logic Analyzer
~~~~~~~~~~~~~~

The logic analysis functionality is easiest from the command-line helper. For example:

.. code-block:: sh

	$ greatfet logic -p out.sr -f 2M -n 4

This will sample 4 channels at 2MSPS, and output a Sigrok-compatible file as ``out.sr``.

The default pin mappings are as follows:

.. list-table :: 
  :header-rows: 1
  :widths: 1 1

  * - Channel 	
    - Pin
  * - 0 	
    - J1_P4
  * - 1
    - J1_P6
  * - 2 	
    - J1_P28
  * - 3 	
    - J1_P30
  * - 4 	
    - J2_P36
  * - 5 	
    - J2_P34
  * - 6 	
    - J2_P33
  * - 7 	
    - J1_P7
  * - 8 	
    - J1_P34
  * - 9 	
    - J2_P9
  * - 10 	
    - J1_P25
  * - 11 	
    - J1_P32
  * - 12 	
    - J1_P31
  * - 13 	
    - J2_P3
  * - 14 	
    - J1_P3
  * - 15 	
    - J1_P5



Pattern Generator
~~~~~~~~~~~~~~~~~

Pattern generation functionality can be done from the command-line helper. For example:

.. code-block:: sh

	$ greatfet pattern counter -n 1K -w 8

This will generate an 8-bit counter at 1KHz.

The default pin mappings are the same as the mappings for logic analysis.



UART
~~~~

This class can be used to talk "serial", and has both a command-line helper tool available and a convenience interface as ``gf.uart``.

If you happen to have a USB-serial converter handy, then you can test it out by connecting TXD, RXD, and GND of the USB-serial converter to J1_P34, J1_P33, and any available ground pin (like J1_P1) respectively. Then you can simply run ``greatfet uart``, and e.g. on Linux, ``dterm /dev/ttyUSB0 115200``. Typing in either terminal will show the respective characters on the other.

Naturally, all of the same functionality can be used from Python via the ``uart`` interface:

.. code-block:: sh

	data = gf.uart.read()
	gf.uart.write(b"Hello, world!")




ADC
~~~

The analog to digital converter is easily usable from the command line helper tool. Simply running ``greatfet adc`` will print the voltage on ADC0 (mapped to J2_P5 by default).

In Python, there is an interface for the default ADC configuration as ADC0. To read a single sample:

.. code-block:: sh

	gf.adc.read_samples(1)

The 10-bit digital to analog converter has a command line helper too, with ``greatfet dac -S <value>``. Note that the value must be the number to set the DAC too, not a voltage. For example, ``greatfet dac -S 512`` will set the DAC to ~1.6 volts.



DAC
~~~

GreatFET's digital to analog converter is mapped to J2_P5. The API allows you to either set the raw value loaded into the DAC, or specify a target voltage (which is calculated as ``value = (voltage * 1024) / 3.3)``. Note however that the voltage must be specified in millivolts, e.g.: ``gf.apis.dac.set_voltage(int(2.5 * 1000))`` will set the DAC voltage to 2.5.

The command-line helper tool can take either the raw value or a voltage:

.. code-block:: sh

	$ greatfet dac -f raw 776
	$ greatfet dac 2.5

Both will set the DAC to ~2.5 volts.




SWRA124/Chipcon
~~~~~~~~~~~~~~~

This class is used to debug microcontrollers implementing the CC1110/CC2430/CC2510 debug interface described in SWRA124.

For simple dumping and flashing of firmware, it is easiest to use the command line utility:

.. code-block:: sh

	$ greatfet chipcon --read firmware.bin --length 0x8000
	$ greatfet chipcon --write firmware.bin

The same functionality, as well as more advanced functionality, can be accessed programmatically through the Python API:

.. code-block:: sh

	cc = gf.create_programmer('chipcon')

	# `debug_init` must be called before any debugging can happen.
	cc.debug_init()

	# Read the entire flash (for a 32k flash).
	flash = cc.read_flash(start_address=0, length=32 * 1024)

	# Reprogram the flash.
	cc.program_flash(flash, erase=True, verify=True)




I²C
~~~

Class for communication over I²C buses. Can be used from Python, e.g. ``gf.i2c.scan()``, or with the command-line helper, e.g. ``gf i2c --scan``.