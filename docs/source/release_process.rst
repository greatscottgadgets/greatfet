================================================
Release Process
================================================

This is the process for tagging and publishing a release. Change the release version number and paths as appropriate. Release version numbers are in the form YYYY.MM.N where N is the release number for that month (usually 1).



tag the release
~~~~~~~~~~~~~~~

.. code-block:: sh

	git tag -a v2013.07.1 -m 'release 2013.07.1'
	git push --tags



make the release directory
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: sh

	cd /tmp
	git clone ~/src/greatfet
	cd greatfet
	rm -rf .git*
	mkdir firmware-bin
	cd ..
	mv greatfet greatfet-2013.07.1



copy/update RELEASENOTES from previous release
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  - prepend the current release notes to previous release notes



make second clone for firmware build
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: sh

	git clone --recursive ~/src/greatfet
	cd greatfet/firmware/libopencm3
	make
	cd ..



update the firmware VERSION_STRING and compile firmware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: sh

	sed -i 's/git-${VERSION}/2013.07.1/' cmake/greatfet-common.cmake
	mkdir build
	cd build
	cmake ..
	make
	cp flash_stub/flash_stub.dfu /tmp/greatfet-2013.07.1/firmware-bin/
	cp greatfet_usb/greatfet_usb.bin /tmp/greatfet-2013.07.1/firmware-bin/



make the release archives
~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: sh

	tar -cJvf greatfet-2013.07.1.tar.xz greatfet-2013.07.1
	zip -r greatfet-2013.07.1.zip greatfet-2013.07.1



"Draft a new release" on github
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    - call it "release 2013.07.1"
    - paste release notes (just for this release, not previous)
    - upload .tar.xz and .zip files



announce the release
~~~~~~~~~~~~~~~~~~~~

    - irc
    - greatfet mailing list
    - twitter
