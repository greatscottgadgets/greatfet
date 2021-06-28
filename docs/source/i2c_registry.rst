.. _I2C_Registry:

================================================
I2C Registry 
================================================

The following I2C addresses (in 7-bit format) are reserved by the I2C specification or by certain neighbors:

.. list-table :: 
  :header-rows: 1
  :widths: 1 1 3

  * - address
    - R/W
    - neighbor
  * - 0x00 	
    - W
    - reserved, general call
  * - 0x00 	
    - R 	
    - reserved, start byte
  * - 0x01
    - 
    - reserved, CBUS
  * - 0x02 
    - 
    - reserved, different bus formats
  * - 0x03
    -  
    - reserved, future purposes
  * - 0x04-0x07
    -  
    - reserved, high speed controller code
  * - 0x10-0x11 
    - 
    - Violet
  * - 0x18-0x1f
    -  
    - Operacake (HackRF add-on in optional neighbor mode)
  * - 0x27
    -  
    - `Crocus <https://github.com/greatfet-hardware/crocus>`__
  * - 0x21
    -  
    - `Foxglove <https://github.com/greatfet-hardware/foxglove>`__
  * - 0x26-0x27
    - 
    - Narcissus
  * - 0x20
    - 
    - `Jasmine <https://github.com/greatfet-hardware/jasmine>`__
  * - 0x50
    - 
    - neighbor identification EEPROM
  * - 0x60 
    -
    - `Gladiolus <https://github.com/greatfet-hardware/gladiolus>`__
  * - 0x78-0x7B
    - 
    - reserved, 10-bit peripheral addressing
  * - 0x7C-0x7F 	
    - R 	
    - reserved, device ID

Looking for known I2C addresses for things other than GreatFET neighbors? Check out `Adafruit's handy list <https://learn.adafruit.com/i2c-addresses/the-list>`__.

Confused about 7-bit vs. 8-bit I2C addresses? So is everyone! We try to use 7-bit addresses whenever possible because that is how they are `specified <https://www.nxp.com/docs/en/user-guide/UM10204.pdf>`__. Total Phase has a `nice article <https://www.totalphase.com/support/articles/200349176-7-bit-8-bit-and-10-bit-I2C-Slave-Addressing>`__ on the subject that we suggest reading to make sense of it all.