from pot_config import pot_cfg_by_target

MAX_POTS = 4
MAX_SLIDERS = 4
MAX_EXTS = 4


def eprint(*args, **kwargs):
    from sys import stderr
    print(*args, file=stderr, **kwargs)

class ADCInput:

    TYPE_STICK  = 'STICK'
    TYPE_FLEX   = 'FLEX'
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

class ADC:
    def __init__(self, name, adc):
        self.name = name
        self.adc = adc

class MouseName:
    AXIS = ['x','y']
    def format(self, idx):
        axis = self.AXIS[idx - 1]
        return f'JS{axis}'

def AZ_seq():
    return [chr(i) for i in range(ord('A'), ord('Z') + 1)]

class ADCInputParser:

    ADC_MAIN = 'MAIN'
    ADC_EXT = 'EXT'

    ADC_INPUTS = [
        {
            'range': ['LH','LV','RV','RH','ST','TH'],
            'type': ADCInput.TYPE_STICK,
            'suffix': 'STICK_{}',
            'name': '{}',
        },
        {
            'range': range(1, MAX_POTS + 1),
            'type': ADCInput.TYPE_FLEX,
            'suffix': 'POT{}',
            'name': 'P{}',
        },
        {
            'range': range(1, MAX_SLIDERS + 1),
            'type': ADCInput.TYPE_FLEX,
            'suffix': 'SLIDER{}',
            'name': 'SL{}',
        },
        {
            'range': range(1, MAX_EXTS + 1),
            'type': ADCInput.TYPE_FLEX,
            'suffix': 'EXT{}',
            'name': 'EXT{}',
        },
        {
            'range': range(1, 2 + 1),
            'type': ADCInput.TYPE_FLEX,
            'suffix': 'MOUSE{}',
            'name': MouseName(),
        },
        {
            'type': 'VBAT',
            'name': 'VBAT',
            'suffix': 'BATT',
        },
        {
            'type': 'RTC_BAT',
            'name': 'RTC_BAT',
            'suffix': 'RTC_BAT',
        },
        {
            'range': AZ_seq(),
            'type': ADCInput.TYPE_SWITCH,
            'suffix': 'SW{}',
            'name': 'SW{}',
        },
    ]
    

    def __init__(self, target, hw_defs, labels):
        self.target = target
        self.hw_defs = hw_defs
        self.labels = labels
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
        dirs = self.hw_defs.get('ADC_DIRECTION') or ''
        if dirs:
            dirs = dirs.strip('{} ').split(',')
        eprint(dirs)
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
            adc = ADC('SPI', adc_spi)
            adc.gpio = self.hw_defs['ADC_SPI_GPIO']
            adc.gpio_pin_miso = self.hw_defs['ADC_SPI_GPIO_PIN_MISO']
            adc.gpio_pin_mosi = self.hw_defs['ADC_SPI_GPIO_PIN_MOSI']
            adc.gpio_pin_sck = self.hw_defs['ADC_SPI_GPIO_PIN_SCK']
            adc.gpio_pin_cs = self.hw_defs['ADC_SPI_GPIO_PIN_CS']
            adcs.append(adc)
        
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
            idx = len(self.inputs)
            if adc_input.type == 'STICK':
                if 'PWM_STICKS' in self.hw_defs:
                    ch = self.hw_defs.get(f'STICK_PWM_CHANNEL_{adc_input.name}')
                    adc_input.pwm_channel = idx if ch is None else ch
            if adc_input.type != 'VBAT' and adc_input.type != 'RTC_BAT':
                d = self.dirs[idx]
                if d < 0:
                    adc_input.inverted = True
            if adc_input.type == 'FLEX':
                input_labels = self.labels[adc_input.name]
                adc_input.label = input_labels.get('label')
                adc_input.short_label = input_labels.get('short_label')
                cfg = pot_cfg_by_target(self.target,adc_input.name)
                if cfg:
                    adc_input.default = cfg.get('default')

            self.inputs.append(adc_input)

    def parse_inputs(self):

        self.adcs = self._parse_adcs()
        self.inputs = []

        for adc_input in self.ADC_INPUTS:
            input_type = adc_input['type']
            input_range = adc_input.get('range')

            if input_range is not None:
                for i in input_range:
                    name = adc_input['name'].format(i)
                    suffix = adc_input['suffix'].format(i)
                    try:
                        self._add_input(self._parse_input_type(input_type, name, suffix))
                    except KeyError:
                        pass
            else:
                # type, name, suffix
                name = adc_input['name']
                suffix = adc_input['suffix']
                try:
                    self._add_input(self._parse_input_type(input_type, name, suffix))
                except KeyError:
                    pass

        return { 'adcs': self.adcs, 'inputs': self.inputs }

    def find_input(self, name):
        for adc_input in self.inputs:
            if adc_input.name == name:
                return adc_input
        return None
