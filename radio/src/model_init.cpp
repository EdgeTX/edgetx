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

void clearInputs()
{
  memset(g_model.expoData, 0, sizeof(g_model.expoData));
}

void setDefaultInputs()
{
  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (int i = 0; i < max_sticks; i++) {
    uint8_t stick_index = inputMappingChannelOrder(i);
    ExpoData *expo = expoAddress(i);
    expo->srcRaw = MIXSRC_FIRST_STICK + stick_index;
    expo->curve.type = CURVE_REF_EXPO;
    expo->chn = i;
    expo->weight = 100;
    expo->mode = 3; // TODO constant
    strncpy(g_model.inputNames[i], getMainControlLabel(stick_index), LEN_INPUT_NAME);
  }

  storageDirty(EE_MODEL);
}

void clearMixes()
{
  memset(g_model.mixData, 0, sizeof(g_model.mixData));
}

void setDefaultMixes()
{
  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (int i = 0; i < max_sticks; i++) {
    MixData * mix = mixAddress(i);
    mix->destCh = i;
    mix->weight = 100;
    mix->srcRaw = i+1;
  }
  storageDirty(EE_MODEL);
}

void setDefaultModelRegistrationID()
{
  memcpy(g_model.modelRegistrationID, g_eeGeneral.ownerRegistrationID, PXX2_LEN_REGISTRATION_ID);
}

void setDefaultGVars()
{
#if defined(FLIGHT_MODES) && defined(GVARS)
  for (int fmIdx = 1; fmIdx < MAX_FLIGHT_MODES; fmIdx++) {
    for (int gvarIdx = 0; gvarIdx < MAX_GVARS; gvarIdx++) {
      g_model.flightModeData[fmIdx].gvars[gvarIdx] = GVAR_MAX + 1;
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

void applyDefaultTemplate()
{
  setDefaultInputs();
  setDefaultMixes();
  setDefaultGVars();
  setDefaultRSSIValues();

  setDefaultModelRegistrationID();

#if defined(FUNCTION_SWITCHES)
  g_model.functionSwitchConfig = DEFAULT_FS_CONFIG;
  g_model.functionSwitchGroup = DEFAULT_FS_GROUPS;
  g_model.functionSwitchStartConfig = DEFAULT_FS_STARTUP_CONFIG;
  g_model.functionSwitchLogicalState = 0;
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i++) {
    g_model.functionSwitchLedOFFColor[i].setColor(0);
    g_model.functionSwitchLedONColor[i].setColor(0xFFFFFF);
  }
#endif
#endif

#if defined(COLORLCD)

  LayoutFactory::loadDefaultLayout();

  // enable switch warnings
  for (uint64_t i = 0; i < MAX_SWITCHES; i++) {
    if (SWITCH_EXISTS(i)) {
      g_model.switchWarning |= (1ull << (3ull * i));
    }
  }
#else
  // enable switch warnings
  for (uint64_t i = 0; i < MAX_SWITCHES; i++) {
    if (SWITCH_WARNING_ALLOWED(i))
      g_model.switchWarning |= (1ull << (3ull * i));
  }
#endif

#if defined(USE_HATS_AS_KEYS)
  g_model.hatsMode = HATSMODE_GLOBAL;
#endif
}

void setModelDefaults(uint8_t id)
{
  memset(&g_model, 0, sizeof(g_model));
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
