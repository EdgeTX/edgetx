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

#include "edgetx.h"
#include "MultiProtoDefs.h"

bool setBindProtocolSelection()
{
  int16_t xPos = calibratedAnalogs[ADC_MAIN_LH];
  int16_t yPos = calibratedAnalogs[ADC_MAIN_LV];

  // Center: D8
  if (abs(xPos < 50 && abs(yPos) < 50)) {
    g_model.moduleData[INTERNAL_MODULE].multi.rfProtocol = MODULE_SUBTYPE_MULTI_FRSKY;
    g_model.moduleData[INTERNAL_MODULE].subType = MULTI_FRSKYD_SUBTYPE_D8;
    storageDirty(EE_MODEL);
    return true;
  }

  // Top left: D16 FCC
  if (xPos < -512 && yPos > 512) {
    g_model.moduleData[INTERNAL_MODULE].multi.rfProtocol = MODULE_SUBTYPE_MULTI_FRSKYX;
    g_model.moduleData[INTERNAL_MODULE].subType = MULTI_FRSKYX_SUBTYPE_D16_FCC;
    g_model.moduleData[INTERNAL_MODULE].failsafeMode = FAILSAFE_NOPULSES;
    storageDirty(EE_MODEL);
    return true;
  }

  // Top right: D16 LBT
  if (xPos > 512 && yPos > 512) {
    g_model.moduleData[INTERNAL_MODULE].multi.rfProtocol = MODULE_SUBTYPE_MULTI_FRSKYX;
    g_model.moduleData[INTERNAL_MODULE].subType = MULTI_FRSKYX_SUBTYPE_D16_LBT;
    g_model.moduleData[INTERNAL_MODULE].failsafeMode = FAILSAFE_NOPULSES;
    storageDirty(EE_MODEL);
    return true;
  }

  // Bottom left: V2.1 D16 FCC
  if (xPos < -512 && yPos < -512) {
    g_model.moduleData[INTERNAL_MODULE].multi.rfProtocol = MODULE_SUBTYPE_MULTI_FRSKYX2;
    g_model.moduleData[INTERNAL_MODULE].subType = MULTI_FRSKYX2_SUBTYPE_D16_FCC;
    g_model.moduleData[INTERNAL_MODULE].failsafeMode = FAILSAFE_NOPULSES;
    storageDirty(EE_MODEL);
    return true;
  }

  // Bottom right: V2.1 D16 LBT
  if (xPos > 512 && yPos < -512) {
    g_model.moduleData[INTERNAL_MODULE].multi.rfProtocol = MODULE_SUBTYPE_MULTI_FRSKYX2;
    g_model.moduleData[INTERNAL_MODULE].subType = MULTI_FRSKYX2_SUBTYPE_D16_LBT;
    g_model.moduleData[INTERNAL_MODULE].failsafeMode = FAILSAFE_NOPULSES;
    storageDirty(EE_MODEL);
    return true;
  }

  return false;
}

void bindButtonHandler(event_t event)
{
  if( !isModuleMultimodule(INTERNAL_MODULE))
    return;

  if (event == EVT_KEY_LONG(KEY_BIND) && getMultiBindStatus(INTERNAL_MODULE) == MULTI_BIND_NONE) {
    if (setBindProtocolSelection()) {
      setMultiBindStatus(INTERNAL_MODULE, MULTI_BIND_INITIATED);
      moduleState[INTERNAL_MODULE].mode = MODULE_MODE_BIND;
    }
  }

  if (getMultiBindStatus(INTERNAL_MODULE) == MULTI_BIND_INITIATED) {
    if (FAST_BLINK_ON_PHASE)
      LED_BIND();
    else
      ledOff();
  }

  if (getMultiBindStatus(INTERNAL_MODULE) == MULTI_BIND_FINISHED) {
    setMultiBindStatus(INTERNAL_MODULE, MULTI_BIND_NONE);
    moduleState[INTERNAL_MODULE].mode = MODULE_MODE_NORMAL;
    LED_ERROR_END();
  }
}
