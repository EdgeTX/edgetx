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
#if defined(GVARS) && defined(FLIGHT_MODES)
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

uint8_t editDelay(coord_t y, event_t event, uint8_t attr, const char * str, uint8_t delay, uint8_t prec)
{
  lcdDrawTextAlignedLeft(y, str);
  lcdDrawNumber(MIXES_2ND_COLUMN, y, delay, attr|prec|LEFT);
  if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, delay, DELAY_MAX);
  return delay;
}

#define COPY_MODE 1
#define MOVE_MODE 2
uint8_t s_copyMode = 0;
int8_t s_copySrcRow;
int8_t s_copyTgtOfs;
uint8_t s_maxLines = 8;
uint8_t s_copySrcIdx;
uint8_t s_copySrcCh;

uint8_t editNameCursorPos = 0;

void editSingleName(coord_t x, coord_t y, const char * label, char *name, uint8_t size, event_t event, uint8_t active, uint8_t old_editMode, coord_t lblX)
{
  lcdDrawText(lblX, y, label);
  TRACE("EDIT SINGLE NAME %s", name);
  editName(x, y, name, size, event, active, 0, old_editMode);
}

uint8_t s_currIdx;
uint8_t s_currIdxSubMenu;
mixsrc_t s_currSrcRaw;
uint16_t s_currScale;
