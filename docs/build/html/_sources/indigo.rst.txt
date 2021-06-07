================================================
Indigo
================================================

Indigo is the ChipWhisperer interface + bonus voltage glitching neighbour. This page is mostly used as some notes on the design + required firmware changes to properly support this.

This board has two main use-cases: (1) triggering the ChipWhisperer for glitching & side-channel power analysis, while using the GreatFET to communicate with your 'target' device, or (2) communicating with any of the ChipWhisperer targets (such as the various UFO board targets).



Triggering
~~~~~~~~~~

The main usage of the GreatFET in this use-case is actually to communicate with the target, while providing a high-precision trigger based on certain packets sent to / received from the target. This is handy as the GreatFET has lots of nice code for doing stuff like USB host testing etc.



Clocking Stuff
~~~~~~~~~~~~~~

You may need to clock the ChipWhisperer in such a way it remains phase-locked to the crystal on the LPC4330.

**TODO: Check clock routing, can we enable CLKOUT to one I/O pin routed to neighbours.**



Triggering from M0 Core
~~~~~~~~~~~~~~~~~~~~~~~

The LPC4330 has both a M4 and M0 core. This project relies on the M0 core for triggering. Basically the deal is:

#. The M0 core is configured to be have a relevant interrupt based on what you wish to trigger on. For example if triggering on USB, you'd have a USB interrupt. If triggering based on a USART message, you'd trigger from the USART RX or TX interrupt (depending on what you want).
#. The M0 core (should) have very constant-time delay between the physical event (packet over wire)
#. The interrupt can either cause an external trigger as soon as the interrupt happens (useful when you KNOW the next byte/packet is the one to trigger on), or potentially trigger based on some data analysis. **TODO: Check for interaction between M0/M4 core. For example if M4 core reads peripheral does it 'lock' it from M0 core, meaning we may have some timing jitter? Also need to avoid doing anything such as reading registers that will auto-clear flags.**

The separate M0 core is critical as attempting to trigger from the M4 core may be unreliable. The M4 core has other interrupts (such as the USB SOF) to deal with, and means there could be some jitter from the time the physical event happens & the time the M4 core gets around to toggling the external I/O pin.



Serial to ChipWhisperer
~~~~~~~~~~~~~~~~~~~~~~~

Sending messages to the ChipWhisperer could be handy. Use I2C to UART bus when needed perhaps (instead of one of the UARTs?). The LPC4330 seems to have lots of UARTs though, and could be handy for speed reasons to skip the I2C bus.