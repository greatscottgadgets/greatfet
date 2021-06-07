================================================
Getting Started with Firmware Development
================================================

Here's some quick info for people with GreatFETs who want to get started with firmware development.



Fresh install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To get started, you will need to install the dependencies:

.. code-block:: sh

	sudo apt-get install gcc-arm-none-eabi libnewlib-arm-none-eabi cmake make dfu-util python-setuptools python-yaml
	pip install pyyaml

Acquire the code by cloning the repository:

.. code-block:: sh

	git clone --recursive https://github.com/greatscottgadgets/greatfet.git



Updating your repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You should already have the prerequisites installed as above. Now update the GreatFET repository that you have previously cloned:

.. code-block:: sh

	cd greatfet
	git pull
	git submodule update



Installing host tools
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The host tools are written in Python. To install them, run the following from the root of the cloned repository:

.. code-block:: sh

	pushd libgreat/host/
	python setup.py build
	sudo python setup.py install
	popd

	pushd host/
	python setup.py build
	sudo python setup.py install
	popd



Building and flashing firmware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The firmware currently constitutes two parts, ``libopencm3`` and ``greatfet_usb``. We build the library first, then the firmware on top of it:

.. code-block:: sh

	cd firmware/libopencm3
	make
	cd ../greatfet_usb
	mkdir build
	cd build
	cmake ..
	make

This will produce a file named ``greatfet_usb.bin`` which can be written to a GreatFET One using ``greatfet_firmware -w greatfet_usb.bin``.

If you need to recover from an empty flash or non-functional firmware, you will need to use DFU to recover. Remember, if the firmware written to flash was non-functional, the DFU version will be too, you will need to return to a known good version to restore GreatFET.

To write the file, first hold the DFU button while resetting the board, ``lsusb`` will show a line such as ``Bus 002 Device 007: ID 1fc9:000c NXP Semiconductors``, which is the NXP LPC4330 in DFU mode. You can write the firmware to the GreatFET One's RAM using ``greatfet_firmware -V greatfet_usb.bin``. The firmware will run immediately. If you wish to run from ROM you then need to use ``greatfet_firmware`` as above.



Re-Building and flashing firmware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When rebuilding software the following is recommended

.. code-block:: sh

	cd firmware/libopencm3
	make clean
	make
	cd ../greatfet_usb
	mkdir build
	cd build
	cmake ..
	make clean
	make

This will produce a file named ``greatfet_usb.bin`` which can be written to a GreatFET One using ``greatfet_firmware -w greatfet_usb.bin``.