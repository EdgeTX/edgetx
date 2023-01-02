
class Switch:

    TYPE_2POS = '2POS'
    TYPE_3POS = '3POS'
    TYPE_ADC  = 'ADC'
    
    def __init__(self, name, sw_type, flags):
        self.name = name
        self.type = sw_type
        self.flags = flags

class Switch2POS(Switch):
    def __init__(self, name, gpio, pin, flags=0):
        super(Switch2POS, self).__init__(name, Switch.TYPE_2POS, flags)
        self.gpio = gpio
        self.pin = pin
        
class Switch3POS(Switch):
    def __init__(self, name, gpio_high, pin_high, gpio_low, pin_low, flags=0):
        super(Switch3POS, self).__init__(name, Switch.TYPE_3POS, flags)
        self.gpio_high = gpio_high
        self.pin_high = pin_high
        self.gpio_low = gpio_low
        self.pin_low = pin_low

class SwitchADC(Switch):
    def __init__(self, name, adc_input, flags=0):
        super(SwitchADC, self).__init__(name, Switch.TYPE_ADC, flags)
        self.adc_input = adc_input
        

def AZ_seq():
    return [chr(i) for i in range(ord('A'), ord('Z') + 1)]

# switches from A to Z
def parse_switches(hw_defs, adc_parser):

    switches = []

    def find_switch(name):
        for sw in switches:
            if name == sw.name:
                return sw
        return None
    
    for s in AZ_seq():

        name = f'S{s}'

        reg = f'SWITCHES_GPIO_REG_{s}'
        reg_high = f'{reg}_H'
        reg_low = f'{reg}_L'

        pin = f'SWITCHES_GPIO_PIN_{s}'
        pin_high = f'{pin}_H'
        pin_low = f'{pin}_L'

        inverted = f'SWITCHES_{s}_INVERTED'
        
        adc_input_name = f'SW{s}'

        switch = None
        if reg in hw_defs:
            # 2POS switch
            reg = hw_defs[reg]
            pin = hw_defs[pin]
            switch = Switch2POS(name, reg, pin)
        elif (reg_high in hw_defs) and (reg_low in hw_defs):
            # 3POS switch
            reg_high = hw_defs[reg_high]
            pin_high = hw_defs[pin_high]
            reg_low = hw_defs[reg_low]
            pin_low = hw_defs[pin_low]
            switch = Switch3POS(name, reg_high, pin_high, reg_low, pin_low)
        else:
            # ADC switch
            if adc_parser.find_input(adc_input_name):
                switch = SwitchADC(name, adc_input_name)

        if switch:
            if inverted in hw_defs:
                switch.inverted = True
                # print(switch.inverted)
            switches.append(switch)

    for i in range(1, 6 + 1):
        f_sw_marker = f'FUNCTION_SWITCH_{i}'
        if f_sw_marker in hw_defs:
            switch = find_switch(hw_defs[f_sw_marker])
            if switch:
                switch.type = 'FSWITCH'
                switch.name = f'SW{i}'

    return switches
