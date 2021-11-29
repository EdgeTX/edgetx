/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "yaml_generalsettings.h"

const YamlLookupTable beeperModeLut = {
  {  GeneralSettings::BEEPER_QUIET, "mode_quiet" },
  {  GeneralSettings::BEEPER_ALARMS_ONLY, "mode_alarms" },
  {  GeneralSettings::BEEPER_NOKEYS, "mode_nokeys" },
  {  GeneralSettings::BEEPER_ALL, "mode_all"  },
};

enum BacklightMode {
  e_backlight_mode_off  = 0,
  e_backlight_mode_keys = 1,
  e_backlight_mode_sticks = 2,
  e_backlight_mode_all = e_backlight_mode_keys+e_backlight_mode_sticks,
  e_backlight_mode_on
};

const YamlLookupTable backlightModeLut = {
  {  e_backlight_mode_off, "backlight_mode_off"  },
  {  e_backlight_mode_keys, "backlight_mode_keys"  },
  {  e_backlight_mode_sticks, "backlight_mode_sticks"  },
  {  e_backlight_mode_all, "backlight_mode_all"  },
  {  e_backlight_mode_on, "backlight_mode_on"  },
};

const YamlLookupTable bluetoothModeLut = {
  {  GeneralSettings::BLUETOOTH_MODE_OFF, "OFF"  },
  {  GeneralSettings::BLUETOOTH_MODE_TELEMETRY, "TELEMETRY"  },
  {  GeneralSettings::BLUETOOTH_MODE_TRAINER, "TRAINER"  },
};

const YamlLookupTable uartModeLut = {
  {  GeneralSettings::AUX_SERIAL_OFF, "MODE_NONE"  },
  {  GeneralSettings::AUX_SERIAL_TELE_MIRROR, "MODE_TELEMETRY_MIRROR"  },
  {  GeneralSettings::AUX_SERIAL_TELE_IN, "MODE_TELEMETRY"  },
  {  GeneralSettings::AUX_SERIAL_SBUS_TRAINER, "MODE_SBUS_TRAINER"  },
  {  GeneralSettings::AUX_SERIAL_LUA, "MODE_LUA"  },
};

const YamlLookupTable antennaModeLut = {
  {  GeneralSettings::ANTENNA_MODE_INTERNAL, "MODE_INTERNAL"  },
  {  GeneralSettings::ANTENNA_MODE_ASK, "MODE_ASK"  },
  {  GeneralSettings::ANTENNA_MODE_PER_MODEL, "MODE_PER_MODEL"  },
  {  GeneralSettings::ANTENNA_MODE_EXTERNAL, "MODE_EXTERNAL"  },
  {  GeneralSettings::ANTENNA_MODE_INTERNAL_EXTERNAL, "MODE_INTERNAL_EXTERNAL"  },
};
