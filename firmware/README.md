The primary firmware source code for GreatFET devices is greatfet_usb.  Most
of the other directories contain firmware source code for test and development.
The common directory contains source code shared by multiple GreatFET firmware
projects.


The firmware is set up for compilation with the GCC toolchain available here:

https://developer.arm.com/open-source/gnu-toolchain/gnu-rm

Most Linux distributions maintain a package for the toolchain, these are the
pefered method of installation.  For example, on Debian based systems:
```
sudo apt-get install gcc-arm-none-eabi
```

Required dependency:

https://github.com/dominicgs/libopencm3

If you are using git, the preferred way to install libopencm3 is to use the
submodule:
```
cd ..
git submodule init
git submodule update
cd firmware/libopencm3
make
```

To build and install a firmware image for GreatFET:
```
cd greatfet_usb
mkdir build
cd build
cmake ..
make
greatfet_firmware -w greatfet_usb.bin
```

For loading firmware into RAM with DFU you will also need:

http://dfu-util.gnumonks.org/

To start up GreatFET One in DFU mode, hold down the DFU button while powering it
on or while pressing and releasing the RESET button.  Release the DFU button
after the 3V3 LED illuminates.

A .dfu file is built by default when building firmware.  Alternatively you can
load a known good .dfu file from a release package with:
```
dfu-util --device 1fc9:000c --alt 0 --download greatfet_usb_ram.dfu
```
