=============================
Getting Started with GreatFET
=============================


Prerequisites
-------------

To use GreatFET you will need to ensure the following software is installed:

  * `Python <https://wiki.python.org/moin/BeginnersGuide/Download>`__ v3.9, or later.


GreatFET Host Software Installation
-----------------------------------

You can install the GreatFET host software from the `Python Package Index (PyPI) <https://pypi.org/project/greatfet/>`__ or :doc:`directly from source <getting_started_firmware_development>`.

To install the GreatFET host software from PyPI using the `pip <https://pypi.org/project/pip/>`__ tool:

.. code-block :: sh

    pip install greatfet

.. note::

    For more information on installing Python packages from PyPI please refer to the
    `"Installing Packages" <https://packaging.python.org/en/latest/tutorials/installing-packages/>`__
    section of the Python Packaging User Guide.


Install udev Rules (Linux Only)
-------------------------------

Configure your system to allow access to GreatFET for logged in users:

.. code-block :: sh

    greatfet host-tools --ensure-access

If you’d prefer to perform this step manually, you can find a udev rules file matching your distribution in the ``host/greatfet/assets/`` directory of the `GreatFET source repository <https://github.com/greatscottgadgets/greatfet/tree/main/host/greatfet/assets>`__.


Test Installation
-----------------

Ensure that GreatFET's `USB0` port is connected to the host and type the following command into a terminal:

.. code-block :: sh

    greatfet info

If everything is working you will see the following output:

.. code-block :: text

    Found a GreatFET One!
      Board ID: 0
      Firmware version: v2024.0.0
      Part ID: xxxxxxxxxxxxxx
      Serial number: xxxxxxxxxxxxxxxxxxxx


Upgrading to a new release
--------------------------

You can upgrade the GreatFET host tools to the latest release with:

.. code-block :: sh

    pip install --upgrade greatfet

After upgrading the host tools, update your GreatFET firmware to the latest release with:

.. code-block :: sh

    greatfet_firmware --autoflash
