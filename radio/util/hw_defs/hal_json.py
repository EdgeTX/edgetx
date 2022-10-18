
import re
import sys
import json

from hal_switches import Switch, parse_switches
from hal_adc import ADCInput, SPI_ADCInput, ADC, ADCInputParser
from hal_keys import Key, Trim, parse_trims, parse_keys

import legacy_names

#
# Return a file handle or STDIN
#
def open_file(filename):
    
    if filename and not filename == '-':
        return open(filename)
    else:
        return sys.stdin

#
# Read lines of defines into a dictionary
#
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
        if isinstance(obj, Trim):
            return prune_dict(obj.__dict__)
        if isinstance(obj, Key):
            return prune_dict(obj.__dict__)

        # Let the base class default method raise the TypeError
        return json.JSONEncoder.default(self, obj)

#
# Parse HAL defines into JSON
#
def parse_defines(filename, target):

    hw_defs = parse_hw_defs(filename)
    out_defs = {}

    # parse ADC first, we might have switches using ADC
    legacy_inputs = legacy_names.inputs_by_target(target)
    adc_parser = ADCInputParser(target,hw_defs,legacy_inputs)
    adc_inputs = adc_parser.parse_inputs()
    out_defs["adc_inputs"] = adc_inputs

    switches = parse_switches(target, hw_defs, adc_parser)
    out_defs["switches"] = switches

    keys = parse_keys(hw_defs)
    out_defs["keys"] = keys

    trims = parse_trims(hw_defs)
    out_defs["trims"] = trims

    print(json.dumps(out_defs, cls=DictEncoder, indent=2))

