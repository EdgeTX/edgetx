
class Misc:

    def __init__(self, has_audio_mute, has_bling_leds, has_ext_module_support, has_int_module_support, sport_max_baudrate, surface):
        self.has_audio_mute = has_audio_mute
        self.has_bling_leds = has_bling_leds
        self.has_ext_module_support = has_ext_module_support
        self.has_int_module_support = has_int_module_support
        self.sport_max_baudrate = sport_max_baudrate
        self.surface = surface

def parse_misc(hw_defs):

    if f'RADIO_MT12' in hw_defs:
      surface = 1
    else:
      surface = 0

    if f'SPORT_MAX_BAUDRATE' in hw_defs:
      sport_max_baudrate = hw_defs[f'SPORT_MAX_BAUDRATE']
    else:
      sport_max_baudrate = 400000

    if f'AUDIO_MUTE_GPIO' in hw_defs:
      has_audio_mute = 1
    else:
      has_audio_mute = 0

    if f'BLING_LED_STRIP_LENGTH' in hw_defs:
      has_bling_leds = hw_defs[f'BLING_LED_STRIP_LENGTH']
    else:
      has_bling_leds = 0

    if f'RADIO_T8' in hw_defs:
      has_ext_module_support = 0
    else:
      has_ext_module_support = 1

    if f'PCBX9D' in hw_defs or f'PCBX9DP' in hw_defs or f'PCBX9E' in hw_defs:
      has_int_module_support = 0
    else:
      has_int_module_support = 1

    return Misc(has_audio_mute, has_bling_leds, has_ext_module_support, has_int_module_support, sport_max_baudrate, surface)
