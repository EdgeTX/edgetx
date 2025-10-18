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

const MenuHandler menuTabModel[]  = {
  { menuModelSelect, nullptr },
  { menuModelSetup, nullptr },
#if defined(HELI)
  { menuModelHeli, modelHeliEnabled },
#endif
#if defined(FLIGHT_MODES)
  { menuModelFlightModesAll, modelFMEnabled },
#endif
  { menuModelExposAll, nullptr },
  { menuModelMixAll, nullptr },
  { menuModelLimits, nullptr },
  { menuModelCurvesAll, modelCurvesEnabled },
#if defined(GVARS) && defined(FLIGHT_MODES) && (LCD_W >= 212)
  { menuModelGVars, modelGVEnabled },
#endif
  { menuModelLogicalSwitches, modelLSEnabled },
  { menuModelSpecialFunctions, modelSFEnabled },
#if defined(LUA_MODEL_SCRIPTS)
  { menuModelCustomScripts, modelCustomScriptsEnabled },
#endif
  { menuModelTelemetry, modelTelemetryEnabled },
  { menuModelDisplay, nullptr }
};

uint8_t s_copyMode = 0;
int8_t s_copySrcRow;
int8_t s_copyTgtOfs;
uint8_t s_maxLines = 8;
uint8_t s_copySrcIdx;
uint8_t s_copySrcCh;

uint8_t s_currIdx;
uint8_t s_currIdxSubMenu;
mixsrc_t s_currSrcRaw;
uint16_t s_currScale;
