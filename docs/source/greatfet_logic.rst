================================================
greatfet_logic
================================================

greatfet_logic is a logic analyzer implementation for GreatFET. It uses the SGPIO peripheral in the LPC4330 to monitor the state of 8 pins and combines those into an 8-bit integer streamed to the USB host.



Pin usage
~~~~~~~~~

.. list-table :: 
  :header-rows: 1
  :widths: 1 1 1 

  * - signal
    - symbol
    - pin
  * - SPGIO0
    - PP0_0
    - J1.04
  * - SGPIO1
    - P0_1
    - J1.06
  * - SGPIO2
    - P1_15 	
    - J1.28
  * - SGPIO3 	
    - P1_16 	
    - J1.30
  * - SGPIO4 	
    - P6_3 	
    - J2.36
  * - SGPIO5 	
    - P6_6 	
    - J2.34
  * - SGPIO6 	
    - P2_2 	
    - J2.33
  * - SGPIO7 	
    - P1_0 	
    - J1.07