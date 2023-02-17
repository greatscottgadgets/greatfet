#!/bin/bash
source testing-venv/bin/activate
usbhub --disable-i2c --hub D9D1 power state --port 1 --reset
sleep 1s
greatfet_firmware --volatile-upload firmware/greatfet_usb/build/greatfet_usb.bin
EXIT_CODE="$?"
deactivate
if [ "$EXIT_CODE" == "0" ]
then
    echo "DFU installation success! Exiting."
    exit $EXIT_CODE
elif [ "$EXIT_CODE" == "19" ]
then
    echo "No GreatFET found! Disconnected? Exiting."
    exit $EXIT_CODE
elif [ "$EXIT_CODE" == "127" ]
then
    echo "Host tool installation failed! Exiting."
    exit $EXIT_CODE
else
    echo "Unhandled case. Exiting."
    exit $EXIT_CODE
fi
