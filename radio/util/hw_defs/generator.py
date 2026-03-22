import json
import os
import sys
import jinja2
import pydantic

import legacy_names
import json_index

from models import HardwareDefinition, KeyEnum

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


def is_ext_input(input):
    if str(getattr(input, "type", "")) != "FLEX":
        return False

    name = str(getattr(input, "name", ""))
    return name.startswith("EXT")


def generate_from_template(json_filename, template_filename, target):
    with open(json_filename) as json_file:
        raw_data = json_file.read()

        # Validate using Pydantic model
        try:
            hw_def = HardwareDefinition.from_json(raw_data)
        except pydantic.ValidationError as e:
            eprint(f"ERROR validating '{json_filename}' (target={target}):")
            for err in e.errors():
                loc = " -> ".join(str(l) for l in err["loc"])
                eprint(f"  {loc}: {err['msg']}")
            sys.exit(1)

        # Load remaining fields not yet in the Pydantic model
        root_obj = json.loads(raw_data)

        adc_index = json_index.build_adc_index(hw_def.adc_inputs)
        adc_gpios = json_index.build_adc_gpio_port_index(hw_def.adc_inputs)
        switch_gpios = json_index.build_switch_gpio_port_index(hw_def.switches)
        key_gpios = json_index.build_key_gpio_port_index(hw_def.keys)

        trims = root_obj.get("trims")
        trim_gpios = json_index.build_trim_gpio_port_index(trims)

        legacy_inputs = legacy_names.inputs_by_target(target)

        template_dir = os.path.dirname(os.path.abspath(template_filename))
        template_name = os.path.basename(template_filename)

        env = jinja2.Environment(
            loader=jinja2.FileSystemLoader(template_dir),
            lstrip_blocks=True,
            trim_blocks=True,
        )

        env.tests["ext_input"] = is_ext_input

        template = env.get_template(template_name)

        key_index = list([str(i) for i in KeyEnum])

        context = dict(
            adc_inputs=hw_def.adc_inputs,
            switches=hw_def.switches,
            keys=hw_def.keys,
            trims=trims,
            adc_index=adc_index,
            adc_gpios=adc_gpios,
            switch_gpios=switch_gpios,
            key_gpios=key_gpios,
            trim_gpios=trim_gpios,
            legacy_inputs=legacy_inputs,
            main_labels=MAIN_CONTROL_LUT,
            key_index=key_index,
        )

        try:
            print(template.render(context))
        except jinja2.UndefinedError as e:
            eprint(f"ERROR rendering template '{template_name}'"
                   f" with json='{json_filename}', target='{target}':")
            eprint(f"  {e}")
            none_keys = [k for k, v in context.items() if v is None]
            if none_keys:
                eprint(f"  None-valued context keys: {none_keys}")
            sys.exit(1)
