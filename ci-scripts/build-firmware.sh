#!/bin/bash
set -e
source testing-venv/bin/activate
cd firmware/libopencm3
make clean
make
cd ../greatfet_usb
mkdir build
cd build
cmake ..
make
cd ../../..
