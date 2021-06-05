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

#include "opentx.h"
#include "datastructs_219.h"

#if defined(COLORLCD)
#include "theme.h"
#endif

//
// Summary of all changes between 219 & 220
//
// All Radios:
// ===========
//
//  PACK(struct TimerData {
// -  int32_t  mode:9;            // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
// -  uint32_t start:23;
// -  int32_t  value:24;
// +  uint32_t start:22;
// +  int32_t  swtch:10;
// +  int32_t  value:22;
// +  uint32_t mode:3; // timer mode (OFF, ON, Start, THs, TH%, THt)
//    uint32_t countdownBeep:2;
//    uint32_t minuteBeep:1;
//    uint32_t persistent:2;
//    int32_t  countdownStart:2;
// -  uint32_t direction:1;
//    NOBACKUP(char name[LEN_TIMER_NAME]);
//  });
//
//
// COLORLCD:
// =========
//
// -  CHKSIZE(CustomScreenData, 610);
// -  CHKSIZE(Topbar::PersistentData, 216);
// +  CHKSIZE(CustomScreenData, 850);
// +  CHKTYPE(TopBarPersistentData, 300);
//
// -  CHKSIZE(RadioData, 881);
// -  CHKSIZE(ModelData, 9736);
// +  CHKSIZE(RadioData, 901);
// +  CHKSIZE(ModelData, 11020);
//
//  -> RadioData change probably comes from THEME_DATA (added parameter types)
//  -> ModelData: +1284 (CustomScreenData x 5 = 1200; TopBar::PersistentData: +84)
//

typedef TimerData TimerData_v220;

void convertModelData_219_to_220(ModelData &model)
{
  ModelData* oldModelAllocated = (ModelData*)malloc(sizeof(ModelData));
  ModelData& oldModel = *oldModelAllocated;

  memcpy(&oldModel, &model, sizeof(ModelData));
  ModelData& newModel = (ModelData&)model;

  for (uint8_t i=0; i<MAX_TIMERS_219; i++) {
    //TODO: convert to new timers v220
    //TimerData & timer = newModel.timers[i];
  }

#if defined(PCBHORUS)
  // Clear CustomScreenData + TopBarPersistentData
  // as they cannot be converted (missing option types)
  memset(newModel.screenData, 0,
         sizeof(newModel.screenData) +
         sizeof(newModel.topbarData));
  //TODO: set defaults as with new model
#endif

  //TODO: convert all names from zchar to "normal" chars
  
  free(oldModelAllocated);
}

#if defined(PCBHORUS)
extern OpenTxTheme * defaultTheme;
#endif

void convertRadioData_219_to_220(RadioData & settings)
{
  TRACE("Radio conversion from v219 to v220");

  settings.version = 220;
  settings.variant = EEPROM_VARIANT;

  RadioData* oldSettingsAllocated = (RadioData*)malloc(sizeof(RadioData));
  RadioData& oldSettings = *oldSettingsAllocated;
  memcpy(&oldSettings, &settings, sizeof(RadioData));

#if defined(PCBHORUS)
  // Clear CustomScreenData + TopBarPersistentData
  // as they cannot be converted (missing option types)
  strcpy(g_eeGeneral.themeName, defaultTheme->getName());
  defaultTheme->init();
#endif

  //
  // TODO: convert all names from zchar to "normal" chars
  //
  // g_model.telemetrySensors[i].label
  // g_model.header.name
  // g_model.header.bitmap // ???? really ????
  // timer.name
  // fm->name
  // expo->name
  // g_model.inputNames[chn]
  // mix->name
  // CurveHeader.name
  // cfn->play.name
  // limit->name

  free(oldSettingsAllocated);
}
