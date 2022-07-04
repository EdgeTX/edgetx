/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
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

#pragma once

#include "libopenui/src/bitfield.h"
#include "definitions.h"
#include "dataconstants.h"
#include "opentx_types.h"
#include "myeeprom.h"
#include "opentx_helpers.h"
#include "pulses_common.h"
#include <cstring>
#include "fifo.h"

#include "hal/module_driver.h"
#include "afhds3_module.h"

#define AFHDS3_MIN_FREQ 50
#define AFHDS3_MAX_FREQ 400
#define AFHDS3_MAX_CHANNELS 18
#define AFHDS3_FAILSAFE_MIN -15000
#define AFHDS3_FAILSAFE_MAX 15000

namespace afhds3
{

enum eLNK_EMIStandard {
  LNK_ES_FREE = 0,
  LNK_ES_CE,
  LNK_ES_FCC,
};

enum eDATA_PHYMODE {
  // V0
  CLASSIC_FLCR1_18CH=0,
  CLASSIC_FLCR6_10CH,
  // V1
  ROUTINE_FLCR1_18CH,
  ROUTINE_FLCR6_8CH,
  ROUTINE_LORA_12CH,
  // must be last
  PHYMODE_MAX = ROUTINE_LORA_12CH
};

#if defined(AFHDS3_EXT_UART)
typedef SerialData ExtmoduleData;
#else
typedef PulsesData ExtmoduleData;
#endif

#if defined(INTERNAL_MODULE_AFHDS3)
typedef SerialData IntmoduleData;
#endif

extern etx_module_driver_t externalDriver;

#if defined(INTERNAL_MODULE_AFHDS3)
extern etx_module_driver_t internalDriver;
#endif

void getStatusString(uint8_t module, char* buffer);
void processTelemetryData(uint8_t data, uint8_t module);

}  // namespace afhds3
