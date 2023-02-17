#!/bin/bash
source testing-venv/bin/activate
sleep 1s
greatfet_firmware --write firmware/greatfet_usb/build/greatfet_usb.bin
EXIT_CODE="$?"
if [ "$EXIT_CODE" == "1" ]
then
    echo "No GreatFET found! Disconnected? Exiting."
    exit $EXIT_CODE
elif [ "$EXIT_CODE" == "0" ]
then
    echo "Firmware successfully flashed!"
elif [ "$EXIT_CODE" == "127" ]
then
    echo "Host tool installation failed! Exiting."
    exit $EXIT_CODE
else
    echo "Unhandled case. Exiting."
    exit $EXIT_CODE
fi
greatfet_info
deactivate
