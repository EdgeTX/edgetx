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
#include "hal/adc_driver.h"
#include "input_mapping.h"
#include "mixes.h"
#include "expos.h"
#include "model_arena.h"

#if defined(COLORLCD)
#include "layout.h"
#endif

void clearInputs()
{
  memset(g_modelArena.sectionBase(ARENA_EXPOS), 0,
         getExpoCount() * sizeof(ExpoData));
}

void setDefaultInputs()
{
  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (int i = 0; i < max_sticks; i++) {
    uint8_t stick_index = inputMappingChannelOrder(i);
    ExpoData *expo = expoAllocAt(i);
    expo->srcRaw = SourceRef_(SOURCE_TYPE_STICK, (uint16_t)stick_index);
    expo->curve.type = CURVE_REF_EXPO;
    expo->chn = i;
    expo->weight.setNumeric(100);
    expo->mode = 3; // TODO constant
    strncpy(inputNameAlloc(i), getMainControlLabel(stick_index), LEN_INPUT_NAME);
  }
  updateExpoCount();
  storageDirty(EE_MODEL);
}

void clearMixes()
{
  memset(g_modelArena.sectionBase(ARENA_MIXES), 0,
         getMixCount() * sizeof(MixData));
}

void setDefaultMixes()
{
  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (int i = 0; i < max_sticks; i++) {
    MixData * mix = mixAllocAt(i);
    mix->destCh = i;
    mix->weight.setNumeric(100);
    mix->srcRaw = SourceRef_(SOURCE_TYPE_INPUT, (uint16_t)i);
  }
  updateMixCount();
  storageDirty(EE_MODEL);
}

void setDefaultModelRegistrationID()
{
  memcpy(g_model.modelRegistrationID, g_eeGeneral.ownerRegistrationID, PXX2_LEN_REGISTRATION_ID);
}

void setDefaultGVars()
{
#if defined(FLIGHT_MODES) && defined(GVARS)
  g_modelArena.ensureSectionCapacity(ARENA_FLIGHT_MODES, MAX_FLIGHT_MODES);
  g_modelArena.ensureSectionCapacity(ARENA_GVAR_DATA, MAX_GVARS);
  g_modelArena.ensureSectionCapacity(ARENA_GVAR_VALUES,
                                     MAX_FLIGHT_MODES * MAX_GVARS);
  for (int fmIdx = 1; fmIdx < getFlightModeCount(); fmIdx++) {
    for (int gvarIdx = 0; gvarIdx < getGVarCount(); gvarIdx++) {
      GVAR_VALUE(gvarIdx, fmIdx) = GVAR_MAX + 1;
    }
  }
#endif
}

void setDefaultRSSIValues()
{
  // Set to legacy FrSky values until
  // a better solution is found (module specific?)
  //
  g_model.rfAlarms.warning = 45;
  g_model.rfAlarms.critical = 42;
}

void setVendorSpecificModelDefaults(uint8_t id)
{
#if defined(RADIOMASTER_RTF_RELEASE)
  // Those settings are for headless radio
  g_model.trainerData.mode = TRAINER_MODE_SLAVE;
  g_model.moduleData[INTERNAL_MODULE].type = MODULE_TYPE_MULTIMODULE;
  g_model.moduleData[INTERNAL_MODULE].multi.rfProtocol = MODULE_SUBTYPE_MULTI_FRSKY;
  g_model.moduleData[INTERNAL_MODULE].subType = MULTI_FRSKYD_SUBTYPE_D8;
  g_model.moduleData[INTERNAL_MODULE].failsafeMode = FAILSAFE_NOPULSES;
#endif
}

#if defined(FUNCTION_SWITCHES)
void initCustomSwitches()
{
  for (int i = 0; i < switchGetMaxSwitches(); i += 1) {
    if (switchIsCustomSwitch(i)) {
      uint8_t idx = switchGetCustomSwitchIdx(i);
      if (strncmp(switchGetDefaultName(i), "SW", 2) == 0) {
        g_model.customSwitches[idx].type = SWITCH_2POS;
        g_model.customSwitches[idx].group = 1;
        if (strncmp(switchGetDefaultName(i), "SW1", 3) == 0)
          g_model.customSwitches[idx].start = FS_START_ON;
        else
          g_model.customSwitches[idx].start = FS_START_OFF;
      } else {
        g_model.customSwitches[idx].type = SWITCH_GLOBAL;
        g_model.customSwitches[idx].group = 0;
        g_model.customSwitches[idx].start = FS_START_PREVIOUS;
      }
      g_model.customSwitches[idx].state = 0;
      g_model.customSwitches[idx].name[0] = 0;
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
      g_model.customSwitches[idx].offColor.setColor(0);
      g_model.customSwitches[idx].onColor.setColor(0xFFFFFF);
#endif
    }
  }
  g_model.cfsSetGroupAlwaysOn(1, true);
}
#endif

void applyDefaultTemplate()
{
  setDefaultInputs();
  setDefaultMixes();
  setDefaultGVars();
  setDefaultRSSIValues();

  setDefaultModelRegistrationID();

#if defined(FUNCTION_SWITCHES)
  initCustomSwitches();
#endif

#if defined(COLORLCD)
  g_model.resetScreenData();
  LayoutFactory::deleteCustomScreens();
  LayoutFactory::deleteTopBarWidgets();
  LayoutFactory::loadDefaultLayout();
#endif

  // enable switch warnings
  bool sw1found = false;
  UNUSED(sw1found);
  for (uint64_t i = 0; i < switchGetMaxAllSwitches(); i++) {
    if (SWITCH_WARNING_ALLOWED(i)) {
#if defined(FUNCTION_SWITCHES)
      if (switchIsCustomSwitch(i) && !sw1found) {
        g_model.setSwitchWarning(i, 3);
        sw1found = true;
      } else {
        g_model.setSwitchWarning(i, 1);
      }
    }
#else
      g_model.setSwitchWarning(i, 1);
    }
#endif
  }

#if defined(USE_HATS_AS_KEYS)
  g_model.hatsMode = HATSMODE_GLOBAL;
#endif
}

void setModelDefaults(uint8_t id)
{
  memset(&g_model, 0, sizeof(g_model));
  inputNameIndexReset();

  // Start with empty arena — applyDefaultTemplate allocates sections on demand
  modelArenaInit();

  applyDefaultTemplate();
  
  setVendorSpecificModelDefaults(id);

#if !defined(STORAGE_MODELSLIST)
  // EEPROM model indexes starting with 0
  id++;
#endif
  strAppendUnsigned(strAppend(g_model.header.name, STR_MODEL), id, 2);

#if defined(LUA) && defined(PCBTARANIS)
  if (isFileAvailable(WIZARD_PATH "/" WIZARD_NAME)) {
    f_chdir(WIZARD_PATH);
    luaExec(WIZARD_NAME);
  }
#endif
}
