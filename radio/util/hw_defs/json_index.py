
#
# These methods are used to build helper indexes on JSON structures
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

        if sw_type == '2POS' or sw_type == 'FSWITCH':
            append_to_index(gpios, switch['gpio'], switch['pin'])

        elif sw_type == '3POS':

            append_to_index(gpios, switch['gpio_high'], switch['pin_high'])
            append_to_index(gpios, switch['gpio_low'], switch['pin_low'])

    return gpios

def build_trim_gpio_port_index(trims):

    def index_contact(gpios, contact):
        append_to_index(gpios, contact['gpio'], contact['pin'])

    i = 0
    gpios = {}
    for trim in trims:
        index_contact(gpios, trim['dec'])
        index_contact(gpios, trim['inc'])

    return gpios

def build_key_gpio_port_index(keys):

    i = 0
    gpios = {}
    for key in keys:
        append_to_index(gpios, key['gpio'], key['pin'])

    return gpios
