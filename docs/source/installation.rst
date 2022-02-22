================================================
Installation
================================================

To install the GreatFET host command line tools and python libraries:

.. code-block:: bash

    pip3 install --upgrade greatfet

Once installed, it's best to perform a device firmware update (dfu) to ensure that the firmware is compatible with the host software. Download the latest release from `GitHub <https://github.com/greatscottgadgets/greatfet/releases>`_, unpack the archive, and navigate to the `firmware-bin` directory.

Before performing the upgrade, you can make backup of the existing firmware with the ``gf`` command utilities. Note, you must first put the device itself into DFU mode by holding down the DFU button, and pressing the RESET button. If LED1 stops flashing, the device is ready.

.. code-block:: bash

    gf fw -r firmware-backup.bin

The new firmware can be flashed with:

.. code-block:: bash

    gf fw -d -w greatfet_usb.bin

After completion, running ``gf info`` will print a device summary, including the firmware version number.
