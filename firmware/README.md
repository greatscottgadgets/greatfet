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

To build a firmware image for the GreatFET One:
```
cd greatfet_usb
mkdir build
cd build
cmake .. -DBOARD=GREATFET_ONE
make greatfet_usb
```

If building for another board, replace ```GREATFET_ONE``` with the the
appropriate board string. The following board strings are currently recognized:
 * `GREATFET_ONE` for the GreatFET One
 * `NXP_XPLORER` for the LPC4330 Xplorer.
 * `RAD1O_BADGE` for the CCCamp 2015 rad1o-badge.

If you're running a GreatFET One or NXP Xplorer that's already flashed with a
GreatFET firmware, you can load your newly-built firmware using the following
command:

```
make greatfet_usb-flash
```

(You'll need the GreatFET tools installed for these commands to work. See the
top-level readme for this firmware for more information.)

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

If you're using a GreatFET One that didn't come pre-flashed (e.g. one you made
yourself), or using a stock NXP Xplorer, follow the instructions for DFU mode
below to start off by running the GreatFET firmware from RAM. You can then use
this firmware to program the device permanenly, from flash.

If you're using a CCCamp rad1o badge, follow the instructions for installing
as a l0adable below.

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

## Installing as a l0adable

If you have a rad1o badge, you can install the firmware as a "l0adable"
application-- which maintains the ability to use your badge's normal functions.

First, you'll need to build the GreatFET firmware using the instructions above.
It's important that you specify the `-DBOARD=RAD1O_BADGE` option above in order
to ensure that the firmware is built and linked as a l0adable.

Next, boot your rad1o in USB Mass Storage mode by holding the joystick UP
while powering on the device. The screen should display:

```
MSC enabled
```

To install the GreatFET application, find the `greatfet_usb.bin` file, which
should be located in:

```
# Assuming you created the 'build' directory above...
build/greatfet_usb/greatfet_usb.bin
```

To install, you'll need to rename this binary to a filename ending in `b1n`
on the badge's UMS device. It's recommended to do this from a terminal, rather
than from the UI on your machine:

```
# Assuming the rad1o badge has been mounted to /Volumes/NO\ NAME/:
cp greatfet_usb/greatfet_usb.bin /Volumes/NO \NAME/greatfet.b1n
```

Eject the badge's USB mass storage device. You should now be able to select
your application as a rad1o application. You can always access the list of
applications-- and choose the startup application-- by holding LEFT on the
joystick as you start the badge.
