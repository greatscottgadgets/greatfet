================================================
Getting Started with GreatFET
================================================

Follow these instructions to get started with GreatFET on Linux or OSX.

Install or Update Software
##########################

.. code-block:: bash

    pip3 install --upgrade --user greatfet

Pay attention to the output of pip3 because it may tell you that you need to `add something to your PATH <https://www.google.com/search?q=how+to+add+a+directory+to+path>`_.

Install udev Rules (Linux)
##########################

This is an important step that we recommend so that you will be able to use your GreatFET as a non-root user. The example udev rules file that we provide assumes that your user is a member of the plugdev group. If that is not the case, you can either add your user to the plugdev group or modify the file to give permission to the group of your choice instead of plugdev.

.. code-block:: bash

    sudo wget https://raw.githubusercontent.com/greatscottgadgets/greatfet/master/host/misc/54-greatfet.rules -O /etc/udev/rules.d/54-greatfet.rules
    sudo udevadm control --reload-rules

Connect your GreatFET One
#########################

Find the USB0 port on your GreatFET One. This is the port on the left (convex) side of the board, the side with LEDs and buttons. Use a USB cable to connect this port to your host computer.

If you chose to install the udev rules and your GreatFET One was already connected to your computer prior to this step, take this opportunity to disconnect your GreatFET one from your computer and connect it again. This will ensure that the udev rules take effect.

Check your GreatFET One
#######################

Once connected, your GreatFET One should start to slowly flash LED1, indicating that it is operating. Confirm that your host computer can communicate with the GreatFET One by typing:

.. code-block:: bash

    greatfet info

This is an example of a command-line tool that is accessible as a subcommand of the unified “greatfet” command. The “greatfet” command is also installed as “gf”. Let’s try the abbreviated form:

.. code-block:: bash

    gf info

The output of this command shows you information about connected GreatFET devices.

Update Firmware
###############

After installing or updating GreatFET software on your host computer it is important to update the firmware on your GreatFET One to match. This can be done with a single command:

.. code-block:: bash

    gf fw --auto
