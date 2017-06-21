# GreatFET Firmware

The primary firmware source code for GreatFET devices is greatfet_usb.  Most
of the other directories contain firmware source code for test and development.
The common directory contains source code shared by multiple GreatFET firmware
projects.

## Building and Installing to Flash

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

To build and install a firmware image for the GreatFET One:
```
cd greatfet_usb
mkdir build
cd build
cmake .. -DBOARD=GREATFET_ONE
make greatfet_usb-flash
```

If building for another board, replace ```GREATFET_ONE``` with the the
appropriate board string. The following board strings are currently recognized:
 * ```GREATFET_ONE``` for the GreatFET One
 * ```NXP_XPLORER``` for the LPC4330 Xplorer.

Reset the device by pressing the RESET button once, and your GreatFET should
start up running the new firmware. You can check the software version by
using the greatfet_info tool provided:

```
# greatfet_info
Found a GreatFET One!
  Board ID: 0
  Firmware version: git-87f4da4
  Part ID: 300a00a0394358
  Serial number: 0000000000000000ffffffffffffffff
```

## DFU Mode: Running from RAM

If your board doesn't currently have GreatFET firmware (e.g. if you built the
board yourself, or if you're using a NXP Xplorer), or your firmware's in a bad
state, you can temporarily load firmware into RAM using the LPC4330 DFU
bootloader. Once the firmware is running, you can then opt to write the firmware
to flash so will persist across reset.

For loading firmware into RAM with DFU you will also need:

http://dfu-util.gnumonks.org/

Each of the currently supproted boards provides an accessible DFU bootloader:
 * To start up GreatFET One in DFU mode, hold down the DFU button while powering
   it on or while pressing and releasing the RESET button.
 * To start the LPC Xplorer in DFU mode, adjust the J4 switches so SW1-4
   are OFF, ON, OFF, and ON respectively, and then press the reset button. To
   resume loading from flash, restore them to so SW1-4 are ON, OFF, OFF, OFF.

Once the GreatFET is in DFU mode, run the following command from your firmware
build tree:
```
make greatfet_usb-program
```

You should see a blinking LED, indicating that the GreatFET firmware is running
and alive.

If you want to continue to program the flash, you can then issue the "flash"
command while the firmware is running.
```
make greatfet_usb-flash
```
