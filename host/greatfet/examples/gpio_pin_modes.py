import time

import greatfet
from greatfet.interfaces import gpio

#
# We will be using the followng GPIO pins:
#
#   Inputs:
#     SGPIO0 -- J1_P4  -- (0,0)
#     SGPIO1 -- J1_P6  -- (0,1)
#     SGPIO2 -- J1_P28 -- (0,2)
#     SGPIO3 -- J1_P30 -- (0,3)
#
#   Outputs:
#     SGPIO4 -- J2_P36 -- (3,2)
#     SGPIO5 -- J2_P34 -- (0,5)
#     SGPIO6 -- J2_P33 -- (5,2)
#     SGPIO7 -- J1_P7  -- (0,4)
#

def main():
    gf = greatfet.GreatFET()
    
    (line0, line1, line2, line3) = ((0,0), (0,1), (0,2), (0,3))
    (line4, line5, line6, line7) = ((3,2), (0,5), (5,2), (0,4))
    
    # - Configure inputs --
    
    # Via gpio.configure_pin()...
    gf.gpio.configure_pin(line0, gpio.InputConfiguration(gpio.PullResistor.PULLDOWN))
    gf.gpio.configure_pin(
        line1,
        gpio.InputConfiguration(
            pull_resistor=gpio.PullResistor.PULLDOWN,
            glitch_filter=gpio.GlitchFilter.DISABLE
        )
    )

    # ...or via gf.gpio.get_pin()
    pin2 = gf.gpio.get_pin("J1_P28")
    pin2.set_configuration(gpio.InputConfiguration(
        pull_resistor=gpio.PullResistor.PULLDOWN,
        glitch_filter=gpio.GlitchFilter.ENABLE,
    ))
    pin3 = gf.gpio.get_pin("J1_P30")
    pin3.set_direction(gpio.Directions.IN, configuration=gpio.InputConfiguration(
        pull_resistor=gpio.PullResistor.PULLDOWN,
    ))
    
    # - Configure outputs --

    # Via gpio.configure_pin()...    
    gf.gpio.configure_pin(line4, gpio.OutputConfiguration(), initial_value=False)
    gf.gpio.configure_pin(line5,  gpio.OutputConfiguration(        
        slew_rate=gpio.SlewRate.SLOW,
    ), initial_value=True)
    
    # ...or via gf.gpio.get_pin()
    pin6 = gf.gpio.get_pin("J2_P33")
    pin6.set_direction(gpio.Directions.OUT, initial_value=False)
    pin7 = gf.gpio.get_pin("J1_P7")
    pin7.set_configuration(gpio.OutputConfiguration(slew_rate=gpio.SlewRate.FAST), initial_value=True)

    blinky = True    
    while True:
        state0 = gf.gpio.read_pin_state(line0)
        state1 = gf.gpio.read_pin_state(line1)
        state2 = pin2.read()
        state3 = pin3.read()
        print(f"SGPIO0:{state0} SGPIO1:{state1} SGPIO2:{state2} SGPIO3:{state3}")

        gf.gpio.set_pin_state(line4, blinky)
        gf.gpio.set_pin_state(line5, blinky)
        pin6.write(blinky)
        pin7.write(blinky)
        blinky = not blinky
        
        time.sleep(0.5)
        

if __name__ == "__main__":
    main()
