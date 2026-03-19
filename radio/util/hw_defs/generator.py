import json
import os
import sys
import jinja2

import legacy_names
import json_index

from models import KeyEnum

MAIN_CONTROL_LUT = {
    # 2 Gimbal radios
    "LH": {"str": "Rud", "local": "STR_STICK_NAMES0"},
    "LV": {"str": "Ele", "local": "STR_STICK_NAMES1"},
    "RV": {"str": "Thr", "local": "STR_STICK_NAMES2"},
    "RH": {"str": "Ail", "local": "STR_STICK_NAMES3"},
    # Surface radios
    "ST": {"str": "ST", "local": "STR_SURFACE_NAMES0"},
    "TH": {"str": "TH", "local": "STR_SURFACE_NAMES1"},
}


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def _normalize_adc_inputs(adc_inputs):
    """Fill in optional ADC input fields with defaults"""
    for inp in adc_inputs.get('inputs', []):
        inp.setdefault('gpio', None)
        inp.setdefault('pin', None)
        inp.setdefault('channel', None)
        inp.setdefault('inverted', False)
        inp.setdefault('label', None)
        inp.setdefault('short_label', None)
        inp.setdefault('default', None)


def _normalize_switches(switches):
    """Fill in optional switch fields with defaults"""
    for sw in switches:
        sw.setdefault('gpio', None)
        sw.setdefault('pin', None)
        sw.setdefault('is_cfs', False)
        sw.setdefault('inverted', False)


def is_ext_input(input):
    if input.get("type") != "FLEX":
        return False

    name = input.get("name")
    if name and name.startswith("EXT"):
        return True

    return False


def generate_from_template(json_filename, template_filename, target):
    with open(json_filename) as json_file:
        root_obj = json.load(json_file)

        adc_inputs = root_obj.get("adc_inputs")
        _normalize_adc_inputs(adc_inputs)
        adc_index = json_index.build_adc_index(adc_inputs)
        adc_gpios = json_index.build_adc_gpio_port_index(adc_inputs)

        switches = root_obj.get("switches")
        _normalize_switches(switches)
        switch_gpios = json_index.build_switch_gpio_port_index(switches)

        keys = root_obj.get("keys")
        key_gpios = json_index.build_key_gpio_port_index(keys)

        trims = root_obj.get("trims")
        trim_gpios = json_index.build_trim_gpio_port_index(trims)

        legacy_inputs = legacy_names.inputs_by_target(target)

        template_dir = os.path.dirname(os.path.abspath(template_filename))
        template_name = os.path.basename(template_filename)

        env = jinja2.Environment(
            loader=jinja2.FileSystemLoader(template_dir),
            lstrip_blocks=True,
            trim_blocks=True,
            undefined=jinja2.StrictUndefined,
        )

        env.tests["ext_input"] = is_ext_input

        template = env.get_template(template_name)

        key_index = list([str(i) for i in KeyEnum])

        context = dict(
            root_obj,
            adc_index=adc_index,
            adc_gpios=adc_gpios,
            switch_gpios=switch_gpios,
            key_gpios=key_gpios,
            trim_gpios=trim_gpios,
            legacy_inputs=legacy_inputs,
            main_labels=MAIN_CONTROL_LUT,
            key_index=key_index,
        )

        # Validate context: warn about None values that are likely errors
        for name, value in context.items():
            if value is None:
                eprint(
                    f"WARNING: '{name}' is None"
                    f" (target={target}, json={json_filename})"
                )

        try:
            print(template.render(context))
        except jinja2.UndefinedError as e:
            eprint(f"ERROR rendering template '{template_filename}'"
                   f" with json='{json_filename}', target='{target}':")
            eprint(f"  {e}")
            eprint(f"  Context keys: {sorted(context.keys())}")
            none_keys = [k for k, v in context.items() if v is None]
            if none_keys:
                eprint(f"  None-valued keys: {none_keys}")
            raise
