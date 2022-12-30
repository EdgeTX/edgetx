
from os import path
import sys
import re
import json
import argparse
import jinja2

MAX_POTS = 4
MAX_SLIDERS = 4
MAX_EXTS = 4

import sys

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

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
        
class ADCInput:

    TYPE_STICK  = 'STICK'
    TYPE_POT    = 'POT'
    TYPE_SWITCH = 'SWITCH'
    # TYPE_BATT   = 'BATT'

    def __init__(self, name, adc_input_type, adc, gpio, pin, channel):
        self.name = name
        self.type = adc_input_type
        self.adc = adc
        self.gpio = gpio
        self.pin = pin
        self.channel = channel

class SPI_ADCInput:

    def __init__(self, name, adc_input_type, channel):
        self.name = name
        self.type = adc_input_type
        self.adc = 'SPI'
        self.channel = channel
        
def prune_dict(d):
    # ret = {}
    # for k, v in d.items():
    #     if v is not None:
    #         ret[k] = v
    # return ret
    return d

class DictEncoder(json.JSONEncoder):

    def default(self, obj):
        if isinstance(obj, Switch):
            return prune_dict(obj.__dict__)
        if isinstance(obj, ADCInput):
            return prune_dict(obj.__dict__)
        if isinstance(obj, SPI_ADCInput):
            return prune_dict(obj.__dict__)
        if isinstance(obj, ADC):
            return prune_dict(obj.__dict__)

        # Let the base class default method raise the TypeError
        return json.JSONEncoder.default(self, obj)


def open_file(filename):
    
    if filename and not filename == '-':
        return open(filename)
    else:
        return sys.stdin
    
def parse_hw_defs(filename):

    hw_defs = {}

    with open_file(filename) as file:
        for line in file.readlines():
            m = re.match(r'#define ([^\s]*)\s*(.*)', line.rstrip())
            name = m.group(1)
            value = m.group(2)
            if value.isnumeric():
                value = int(value)
            elif not value:
                value = None
            hw_defs[name] = value

    return hw_defs

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

class ADC:
    def __init__(self, name, adc):
        self.name = name
        self.adc = adc

class MouseName:
    AXIS = ['x','y']
    def format(self, idx):
        axis = self.AXIS[idx - 1]
        return f'JS{axis}'

class ADCInputParser:

    ADC_MAIN = 'MAIN'
    ADC_EXT = 'EXT'

    ADC_INPUTS = [
        {
            'range': ['LH','LV','RV','RH'],
            'type': ADCInput.TYPE_STICK,
            'suffix': 'STICK_{}',
            'name': '{}',
        },
        {
            'range': range(1, MAX_POTS + 1),
            'type': ADCInput.TYPE_POT,
            'suffix': 'POT{}',
            'name': 'P{}',
        },
        {
            'range': range(1, MAX_SLIDERS + 1),
            'type': ADCInput.TYPE_POT,
            'suffix': 'SLIDER{}',
            'name': 'SL{}',
        },
        {
            'range': range(1, MAX_EXTS + 1),
            'type': ADCInput.TYPE_POT,
            'suffix': 'EXT{}',
            'name': 'EXT{}',
        },
        {
            'range': range(1, 2 + 1),
            'type': 'AXIS',
            'suffix': 'MOUSE{}',
            'name': MouseName(),
        },
        {
            'range': AZ_seq(),
            'type': ADCInput.TYPE_SWITCH,
            'suffix': 'SW{}',
            'name': 'SW{}',
        },
    ]
    

    def __init__(self, hw_defs):
        self.hw_defs = hw_defs
        self.regs = self._parse_regs()
        self.dirs = self._parse_dirs()
        self.adcs = []

    def _parse_regs(self):
        
        regs = {}
        for reg in AZ_seq():
            reg = f'GPIO{reg}'
            pins_def = f'ADC_{reg}_PINS'
            if pins_def in self.hw_defs:
                regs[reg] = self.hw_defs[pins_def]

        return regs

    def _parse_dirs(self):

        ret = []
        dirs = self.hw_defs['ADC_DIRECTION'].strip('{} ').split(',')
        # print(dirs)
        for i in dirs:
            ret.append(int(i))

        return ret
        
    def _find_adc(self, channel_def):

        if (self.ext_list is None) or (channel_def not in self.ext_list):
            return self.ADC_MAIN
        else:
            return self.ADC_EXT
        
    def _find_gpio(self, pin):
        gpio = None
        for reg, pins_def in self.regs.items():
            if pins_def and (pin in pins_def):
                gpio = reg

        return gpio


    def _parse_adc(self, name, periph, prefix):

        adc_periph = self.hw_defs.get(periph)
        if not adc_periph:
            return None

        adc = ADC(name, adc_periph)
        adc.dma = self.hw_defs.get(f'{prefix}_DMA')
        if adc.dma:
            adc.dma_channel = self.hw_defs[f'{prefix}_DMA_CHANNEL']
            adc.dma_stream = self.hw_defs[f'{prefix}_DMA_STREAM']
            adc.dma_stream_irq = self.hw_defs[f'{prefix}_DMA_STREAM_IRQ']
            adc.dma_stream_irq_handler = self.hw_defs[f'{prefix}_DMA_STREAM_IRQHandler']

        adc.sample_time = self.hw_defs[f'{prefix}_SAMPTIME']
        return adc
    
    def _parse_adcs(self):

        adcs = []

        adc_spi = self.hw_defs.get('ADC_SPI')
        if adc_spi:
            #eprint('## Found SPI ADC')
            adcs.append(ADC('SPI', adc_spi))
        
        adc_main = self._parse_adc('MAIN', 'ADC_MAIN', 'ADC')
        if adc_main:
            adcs.append(adc_main)

        self.ext_list = None
        adc_ext = self._parse_adc('EXT', 'ADC_EXT', 'ADC_EXT')
        if adc_ext:
            self.ext_list = self.hw_defs['ADC_EXT_CHANNELS']
            adcs.append(adc_ext)

        return adcs

    def _parse_input_type(self, input_type, name, suffix):

        gpio = None
        pin = None
        d = 1 # non-inverted

        # search for SPI input first
        spi_def = f'ADC_SPI_{suffix}'
        if spi_def in self.hw_defs:
            #eprint('## Found SPI ADC Input:', spi_def)
            channel = self.hw_defs[spi_def]
            return SPI_ADCInput(name, input_type, channel)
        
        if name != 'RTC_BAT':
            pin_def = f'ADC_GPIO_PIN_{suffix}'
            pin = self.hw_defs[pin_def]
            gpio = self._find_gpio(pin_def)

            # check if 'pin' is maybe an alias
            alias = self.hw_defs.get(pin)
            if alias is not None:
                alias_gpio = self._find_gpio(pin)
                gpio = alias_gpio if alias_gpio else gpio
                pin = alias
        
        channel_def = f'ADC_CHANNEL_{suffix}'
        channel = self.hw_defs[channel_def]
        adc = self._find_adc(channel_def)

        return ADCInput(name, input_type, adc, gpio, pin, channel)

    def _add_input(self, adc_input):
        if adc_input is not None:
            if adc_input.type != 'VBAT' and adc_input.type != 'RTC_BAT':
                d = self.dirs[len(self.inputs)]
                if d < 0:
                    adc_input.inverted = True
            self.inputs.append(adc_input)

    def parse_inputs(self):

        self.adcs = self._parse_adcs()
        self.inputs = []

        for adc_input in self.ADC_INPUTS:
            input_type = adc_input['type']
            for i in adc_input['range']:
                name = adc_input['name'].format(i)
                suffix = adc_input['suffix'].format(i)
                try:
                    self._add_input(self._parse_input_type(input_type, name, suffix))
                except KeyError:
                    pass

        try:
            self._add_input(self._parse_input_type('VBAT', 'VBAT', 'BATT'))
        except KeyError:
            pass

        try:
            self._add_input(self._parse_input_type('RTC_BAT', 'RTC_BAT', 'RTC_BAT'))
        except KeyError:
            pass

        return { 'adcs': self.adcs, 'inputs': self.inputs }

    def find_input(self, name):
        for adc_input in self.inputs:
            if adc_input.name == name:
                return adc_input
        return None


def parse_defines(filename):

    hw_defs = parse_hw_defs(filename)
    out_defs = {}

    # parse ADC first, we might have switches using ADC
    adc_parser = ADCInputParser(hw_defs)
    adc_inputs = adc_parser.parse_inputs()
    out_defs["adc_inputs"] = adc_inputs

    switches = parse_switches(hw_defs, adc_parser)
    out_defs["switches"] = switches

    print(json.dumps(out_defs, cls=DictEncoder, indent=2))

#
# These methods are used on JSON structure, not on the object collections
#
def build_adc_index(adc_inputs):

    i = 0
    index = {}
    for adc_input in adc_inputs['inputs']:
        name = adc_input['name']
        index[name] = i
        i = i + 1

    return index

def append_to_index(d, key, val):
    if key not in d:
        d[key] = []

    d[key].append(val)

def build_adc_gpio_port_index(adc_inputs):

    i = 0
    gpios = {}
    for adc_input in adc_inputs['inputs']:

        if adc_input['adc'] == 'SPI':
            continue
        
        gpio = adc_input['gpio']
        if gpio is None:
            i = i + 1
            continue

        if gpio not in gpios:
            gpios[gpio] = []

        pin = {
            'pin': adc_input['pin'],
            'idx': i
        }

        gpios[gpio].append(pin)
        i = i + 1

    return gpios

def build_switch_gpio_port_index(switches):

    i = 0
    gpios = {}
    for switch in switches:
        sw_type = switch['type']

        if sw_type == '2POS':
            # gpio = switch['gpio']

            # if gpio not in gpios:
            #     gpios[gpio] = []

            # gpios[gpio].append(switch['pin'])
            append_to_index(gpios, switch['gpio'], switch['pin'])

        elif sw_type == '3POS':

            append_to_index(gpios, switch['gpio_high'], switch['pin_high'])
            append_to_index(gpios, switch['gpio_low'], switch['pin_low'])

    return gpios

def generate_from_template(json_filename, template_filename):

    with open(json_filename) as json_file:
        with open(template_filename) as template_file:

            root_obj = json.load(json_file)

            adc_inputs = root_obj.get('adc_inputs')
            adc_index = build_adc_index(adc_inputs)
            adc_gpios = build_adc_gpio_port_index(adc_inputs)

            switches = root_obj.get('switches')
            switch_gpios = build_switch_gpio_port_index(switches);

            env = jinja2.Environment(
                lstrip_blocks=True,
                trim_blocks=True
            )

            template_str = template_file.read()
            template = env.from_string(template_str)

            print(template.render(root_obj,
                                  adc_index=adc_index,
                                  adc_gpios=adc_gpios,
                                  switch_gpios=switch_gpios))

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Process hardware definitions')
    parser.add_argument('filename', metavar='filename', nargs='+')
    parser.add_argument('-i', metavar='input', choices=['json','defines'], default='json')
    parser.add_argument('-t', metavar='template')

    args = parser.parse_args()

    if args.i == 'defines':
        for filename in args.filename:
            parse_defines(filename)

    elif args.i == 'json':
        for filename in args.filename:
            generate_from_template(filename, args.t)
