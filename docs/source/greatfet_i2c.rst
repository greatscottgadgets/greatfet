================================================
greatfet_i2c
================================================

greatfet_i2c is a tool for talking to an I2C device with GreatFET.



Pin Usage
~~~~~~~~~

.. list-table :: 
  :header-rows: 1
  :widths: 1 1 1 

  * - signal
    - symbol
    - pin
  * - SDA
    - I2C0_SDA
    - J2.39
  * - SCL
    - I2C0_SCL
    - J2.40



Example Usage
~~~~~~~~~~~~~

`scan(GreatFET) <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/commands/greatfet_i2c.py#L67>`__ - scans the GreatFET I2C Bus for connected I2C devices and displays responses from 7-bit addresses.

.. code-block:: sh

	$ gf i2c -z
	I2C address(es):
	0x0 W
	0x20 W
	0x20 R
	0x7c W

	******** W/R bit set at each valid address ********
	     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	00: W- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	20: WR -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	70: -- -- -- -- -- -- -- -- -- -- -- -- W- -- -- --

`read(GreatFET, address, receive_length, log_function) <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/commands/greatfet_i2c.py#L50>`__ - reads a 'receive_length' amount of bytes over the I2C bus from a device with the specified 7-bit address. Optionally displays results.

`write(GreatFET, address, data, log_function) <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/commands/greatfet_i2c.py#L60>`__ - writes bytes over the I2C bus to a device with the specified 7-bit address. Optionally displays results.

.. code-block:: sh

	$ gf i2c -a 0x20 -w 0x01 0x02 0x03 -r 2 -v
	Trying to find a GreatFET device...
	GreatFET One found. (Serial number: 000057cc67e630318c57)
	Writing to address 0x20
	I2C write status: 0x18
	Bytes received from address 0x20:
	0x3
	0x3
	I2C read status: 0x40

Transmits bytes '0x01', '0x02', and '0x03' to an I2C device with address 0x20 (Crocus in this case), receives 2 bytes in response, and displays the results.



API
~~~

- `I2CDevice(GreatFET, address) <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/interfaces/i2c_device.py#L8>`__ - class representation of an I2C device that can send/receive data over the GreatFET I2C bus using the given address
- `I2CBus(GreatFET) <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/interfaces/i2c_bus.py#L8>`__ - class representation of a GreatFET I2C bus
- `attach_device(I2CDevice) <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/interfaces/i2c_bus.py#L45>`__ - attaches a given I2C device to this bus.
- `read(address, receive_length) <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/interfaces/i2c_bus.py#L59>`__ - reads data from the I2C bus.
- `write(address, data) <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/interfaces/i2c_bus.py#L83>`__ - writes data to the I2C bus.
- `transmit(address, data, receive_length) <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/interfaces/i2c_bus.py#L102>`__ - wrapper function for back to back TX/RX.
- `scan() <https://github.com/greatscottgadgets/greatfet/blob/master/host/greatfet/interfaces/i2c_bus.py#L119>`__ - TX/RX over the I2C bus, and receives ACK/NAK in response for valid/invalid addresses.

