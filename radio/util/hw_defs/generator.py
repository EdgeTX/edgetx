
import json
import jinja2

import legacy_names
import json_index

MAIN_CONTROL_LUT = {
    # 2 Gimbal radios
    "LH": { "str": "Rud", "local": "STR_STICK_NAMES0" },
    "LV": { "str": "Ele", "local": "STR_STICK_NAMES1" },
    "RV": { "str": "Thr", "local": "STR_STICK_NAMES2" },
    "RH": { "str": "Ail", "local": "STR_STICK_NAMES3" },
    # Surface radios
    "ST": { "str": "ST", "local": "STR_SURFACE_NAMES0" },
    "TH": { "str": "TH", "local": "STR_SURFACE_NAMES1" },
}

# MUST be the same order as 'EnumKeys'
KEYS_LUT = [
  "KEY_MENU",
  "KEY_EXIT",
  "KEY_ENTER",

  "KEY_PAGEUP",
  "KEY_PAGEDN",

  "KEY_UP",
  "KEY_DOWN",

  "KEY_LEFT",
  "KEY_RIGHT",

  "KEY_PLUS",
  "KEY_MINUS",

  "KEY_MODEL",
  "KEY_TELE",
  "KEY_SYS",

  "KEY_SHIFT",
  "KEY_BIND",
]

# def eprint(*args, **kwargs):
#     from sys import stderr
#     print(*args, file=stderr, **kwargs)

def is_ext_input(input):

    if input.get('type') != 'FLEX':
        return False

    name = input.get('name')
    if name and name.startswith('EXT'):
        return True

    return False


def generate_from_template(json_filename, template_filename, target):

    with open(json_filename) as json_file:
        with open(template_filename) as template_file:

            root_obj = json.load(json_file)

            adc_inputs = root_obj.get('adc_inputs')
            adc_index = json_index.build_adc_index(adc_inputs)
            adc_gpios = json_index.build_adc_gpio_port_index(adc_inputs)

            switches = root_obj.get('switches')
            switch_gpios = json_index.build_switch_gpio_port_index(switches)

            keys = root_obj.get('keys')
            key_gpios = json_index.build_key_gpio_port_index(keys)

            trims = root_obj.get('trims')
            trim_gpios = json_index.build_trim_gpio_port_index(trims)
            
            legacy_inputs = legacy_names.inputs_by_target(target)
            
            env = jinja2.Environment(
                lstrip_blocks=True,
                trim_blocks=True
            )

            env.tests['ext_input'] = is_ext_input

            template_str = template_file.read()
            template = env.from_string(template_str)

            print(template.render(root_obj,
                                  adc_index=adc_index,
                                  adc_gpios=adc_gpios,
                                  switch_gpios=switch_gpios,
                                  key_gpios=key_gpios,
                                  trim_gpios=trim_gpios,
                                  legacy_inputs=legacy_inputs,
                                  main_labels=MAIN_CONTROL_LUT,
                                  key_index=KEYS_LUT))
