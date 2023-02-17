#!/bin/bash
source testing-venv/bin/activate
usbhub --disable-i2c --hub D9D1 power state --port 1 --reset
sleep 1s
greatfet_info
EXIT_CODE="$?"
deactivate
if [ "$EXIT_CODE" == "19" ]
then
    echo "Host tool installation success! Exiting."
    exit 0
elif [ "$EXIT_CODE" == "0" ]
then
    echo "Failed to boot GreatFET into DFU mode! Check DFU pin jumper. Exiting."
    exit 99
elif [ "$EXIT_CODE" == "127" ]
then
    echo "Host tool installation failed! Exiting."
    exit $EXIT_CODE
else
    echo "Unhandled case. Exiting."
    exit $EXIT_CODE
fi
