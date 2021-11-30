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

#include "yaml_modeldata.h"
#include "modeldata.h"

const YamlLookupTable timerModeLut = {
    {TimerData::TIMERMODE_OFF, "OFF"},
    {TimerData::TIMERMODE_ON, "ON"},
    {TimerData::TIMERMODE_START, "START"},
    {TimerData::TIMERMODE_THR, "THR"},
    {TimerData::TIMERMODE_THR_REL, "THR_REL"},
    {TimerData::TIMERMODE_THR_START, "THR_START"},
};

const YamlLookupTable trainerModeLut = {
  {  TRAINER_MODE_OFF, "OFF"  },
  {  TRAINER_MODE_MASTER_JACK, "MASTER_TRAINER_JACK"  },
  {  TRAINER_MODE_SLAVE_JACK, "SLAVE"  },
  {  TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE, "MASTER_SBUS_EXT"  },
  {  TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE, "MASTER_CPPM_EXT"  },
  {  TRAINER_MODE_MASTER_BATTERY_COMPARTMENT, "MASTER_BATT_COMP"  },
  {  TRAINER_MODE_MASTER_BLUETOOTH, "MASTER_BT"  },
  {  TRAINER_MODE_SLAVE_BLUETOOTH, "SLAVE_BT"  },
  {  TRAINER_MODE_MULTI, "MASTER_MULTI"  },
};
