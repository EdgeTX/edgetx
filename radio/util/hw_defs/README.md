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
- a report about the usage of the removed definition is generated with [define_usage_checker.py](../../../define_usage_checker.py) (see output in next sub-section).

### Reported usage of removed hardware definitions

```
============================================================
USAGE ANALYSIS RESULTS
============================================================

❌ USED NAMES (16) - DO NOT REMOVE:
  • ADC_CHANNEL_RAW1
    Used in 1 file(s):
      radio/src/targets/pl18/nb4p_key_driver.cpp:105: LL_ADC_REG_SetSequencerRanks(ADC_MAIN, LL_ADC_REG_RANK_1, ADC_CHANNEL_RAW1);
      radio/src/targets/pl18/nb4p_key_driver.cpp:106: LL_ADC_SetChannelSamplingTime(ADC_MAIN, ADC_CHANNEL_RAW1, LL_ADC_SAMPLINGTIME_3CYCLES);

  • ADC_DIRECTION
    Used in 1 file(s):
      radio/src/targets/horus/hal.h:224: #error "Missing ADC_DIRECTION array"

  • ADC_GPIO_PIN_RAW1
    Used in 1 file(s):
      radio/src/targets/pl18/nb4p_key_driver.cpp:62: pinInit.Pin = ADC_GPIO_PIN_RAW1;

  • ADC_GPIO_PIN_STICK_TH
    Used in 2 file(s):
      radio/src/targets/taranis/board.h:122: #if defined(ADC_GPIO_PIN_STICK_TH)
      radio/src/targets/pl18/board.h:32: #if defined(ADC_GPIO_PIN_STICK_TH)

  • ADC_MAIN
    Used in 2 file(s):
      radio/src/targets/pl18/board.cpp:182: LL_ADC_Disable(ADC_MAIN);
      radio/src/targets/pl18/nb4p_key_driver.cpp:69: uint32_t adc_idx = (((uint32_t) ADC_MAIN) - ADC1_BASE) / 0x100UL;
      radio/src/targets/pl18/nb4p_key_driver.cpp:78: LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC_MAIN), &commonInit);
      radio/src/targets/pl18/nb4p_key_driver.cpp:81: LL_ADC_Disable(ADC_MAIN);
      radio/src/targets/pl18/nb4p_key_driver.cpp:88: LL_ADC_Init(ADC_MAIN, &adcInit);
      ... and 7 more occurrences

  • ADC_SPI
    Used in 1 file(s):
      radio/src/boards/generic_stm32/analog_inputs.cpp:30: #if defined(ADC_SPI)
      radio/src/boards/generic_stm32/analog_inputs.cpp:57: #if defined(ADC_SPI)
      radio/src/boards/generic_stm32/analog_inputs.cpp:66: #if defined(ADC_SPI)
      radio/src/boards/generic_stm32/analog_inputs.cpp:76: #if defined(ADC_SPI)

  • KEYS_GPIO_REG_DOWN
    Used in 1 file(s):
      radio/src/gui/common/stdlcd/radio_ghost_menu.cpp:56: #elif defined(KEYS_GPIO_REG_DOWN)

  • KEYS_GPIO_REG_LEFT
    Used in 1 file(s):
      radio/src/gui/navigation/navigation.h:29: #elif defined(KEYS_GPIO_REG_LEFT)

  • KEYS_GPIO_REG_MENU
    Used in 3 file(s):
      radio/src/lua/interface.cpp:1244: #if defined(KEYS_GPIO_REG_MENU)
      radio/src/lua/api_general.cpp:1697: #if defined(KEYS_GPIO_REG_MENU)
      radio/src/gui/common/stdlcd/radio_sdmanager.cpp:307: #if defined(KEYS_GPIO_REG_MENU)

  • KEYS_GPIO_REG_MINUS
    Used in 2 file(s):
      radio/src/targets/simu/opentxsimulator.cpp:443: #elif defined(KEYS_GPIO_REG_PLUS) && defined(KEYS_GPIO_REG_MINUS)
      radio/src/gui/common/stdlcd/radio_ghost_menu.cpp:58: #elif defined(KEYS_GPIO_REG_MINUS)

  • KEYS_GPIO_REG_PAGEDN
    Used in 1 file(s):
      radio/src/keys.cpp:470: #if defined(KEYS_GPIO_REG_PAGEDN) && !defined(KEYS_GPIO_REG_PAGEUP)

  • KEYS_GPIO_REG_PAGEUP
    Used in 4 file(s):
      radio/src/gui/navigation/navigation.h:31: #elif defined(KEYS_GPIO_REG_PAGEUP) && defined(KEYS_GPIO_REG_TELE)
      radio/src/gui/navigation/navigation.h:89: #elif defined(KEYS_GPIO_REG_PAGEUP)
      radio/src/keys.cpp:470: #if defined(KEYS_GPIO_REG_PAGEDN) && !defined(KEYS_GPIO_REG_PAGEUP)
      radio/src/gui/colorlcd/libui/keyboard_number.cpp:114: #if (defined(KEYS_GPIO_REG_PAGEUP) || defined(USE_HATS_AS_KEYS)) && \
      radio/src/gui/colorlcd/libui/keyboard_text.cpp:98: #if (defined(KEYS_GPIO_REG_PAGEUP) || defined(USE_HATS_AS_KEYS)) && \

  • KEYS_GPIO_REG_PLUS
    Used in 2 file(s):
      radio/src/targets/simu/opentxsimulator.cpp:443: #elif defined(KEYS_GPIO_REG_PLUS) && defined(KEYS_GPIO_REG_MINUS)
      radio/src/gui/common/stdlcd/radio_ghost_menu.cpp:45: #elif defined(KEYS_GPIO_REG_PLUS)

  • KEYS_GPIO_REG_SHIFT
    Used in 3 file(s):
      radio/src/gui/navigation/navigation.h:27: #elif defined(KEYS_GPIO_REG_SHIFT)
      radio/src/simu.cpp:375: #if defined(KEYS_GPIO_REG_SHIFT)
      radio/src/keys.cpp:478: #if defined(KEYS_GPIO_REG_SHIFT)

  • KEYS_GPIO_REG_TELE
    Used in 1 file(s):
      radio/src/gui/navigation/navigation.h:31: #elif defined(KEYS_GPIO_REG_PAGEUP) && defined(KEYS_GPIO_REG_TELE)

  • KEYS_GPIO_REG_UP
    Used in 1 file(s):
      radio/src/gui/common/stdlcd/radio_ghost_menu.cpp:43: #elif defined(KEYS_GPIO_REG_UP)
```
