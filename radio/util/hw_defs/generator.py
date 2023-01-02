
import json
import jinja2

import legacy_names

from json_index import build_adc_index, build_adc_gpio_port_index, build_switch_gpio_port_index

def generate_from_template(json_filename, template_filename, target):

    with open(json_filename) as json_file:
        with open(template_filename) as template_file:

            root_obj = json.load(json_file)

            adc_inputs = root_obj.get('adc_inputs')
            adc_index = build_adc_index(adc_inputs)
            adc_gpios = build_adc_gpio_port_index(adc_inputs)

            switches = root_obj.get('switches')
            switch_gpios = build_switch_gpio_port_index(switches);

            legacy_inputs = legacy_names.inputs_by_target(target)
            
            env = jinja2.Environment(
                lstrip_blocks=True,
                trim_blocks=True
            )

            template_str = template_file.read()
            template = env.from_string(template_str)

            print(template.render(root_obj,
                                  adc_index=adc_index,
                                  adc_gpios=adc_gpios,
                                  switch_gpios=switch_gpios,
                                  legacy_inputs=legacy_inputs))
