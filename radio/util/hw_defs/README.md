# Hardware Definitions

The Python scripts in this directory are responsible for 2 different tasks:
- generating a JSON definition file for each target,
- and generating different include files used to describe the hardware to the software.

While the exact format is still moving, the goal of this file is to replace the various
hardware definitions used in EdgeTx at various places and offer a simpler way to define
new targets.

At the end of the process, the JSON definition will be the *master copy* used by:
- radio firmware,
- radio simlator,
- and Companion.

## JSON Definition

Currently, the JSON definition file is still generated during the build process using
the target's `hal.h` and various Python scripts:

- `generate_hw_def.py`: command line to generate JSON file as well as process
  the JSON file into include files.

- `hal_json.py`: reads the `#define ...` definitions coming from `hal.h` into a dictionary (AVPs).

- `hal_adc.py`: parse ADC definitions and related analog inputs.

- `legacy_names.py`: hard-coded definitions providing additional information not present in `hal.h`
  related to analog inputs (by target; mostly names and labels).

- `pot_config.py`: provides default configuration of pots and sliders for each target.

- `hal_keys.py`: parse keys & trims.

- `hal_switches.py`: parse switches.

- `switch_config.py`: default switch configuration (all targets) as well as display positions
  (only `stdlcd` targets).

Once the format is stable enough and does not require editing each target separately
to add or rename each attribute, the JSON definition for each target will be generated
and added to version control. Then the Python scripts will obsoleted and removed. The
target's `hal.h` should be cleaned-up as well with the definitions present in JSON removed.

## Removing parsed defines from `hal.h`

As we now have JSON files for some parts of the hardware definitions, it would be
good to remove these redundant definitions and use only the JSON files.

For a smooth transition, the removal and checks should happen as automated as possible.

This includes:
- logging which defines are used from `hal.h`: [logging_dict.py](logging_dict.py)
- the output is then sorted and duplicates removed:
```
./tools/generate-hw-defs.sh 2>hw_defs.log
cat hw_defs.log | sort -u | grep -v -E 'None|LL_GPIO|PWM_STICKS' > hw_defs.sorted
rm hw_defs.log
```
- `hw_defs.sorted` can then be used to remove the definitions with [define_remover.py](../../../define_remover.py).
- a report about the usage of the removed definition is generated with [define_usage_checker.py](../../../define_usage_checker.py).
