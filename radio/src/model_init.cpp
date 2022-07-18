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

void clearInputs()
{
  memset(g_model.expoData, 0, sizeof(g_model.expoData));
}

void setDefaultInputs()
{
  for (int i=0; i<NUM_STICKS; i++) {
    uint8_t stick_index = channelOrder(i+1);
    ExpoData *expo = expoAddress(i);
    expo->srcRaw = MIXSRC_Rud - 1 + stick_index;
    expo->curve.type = CURVE_REF_EXPO;
    expo->chn = i;
    expo->weight = 100;
    expo->mode = 3; // TODO constant
    strncpy(g_model.inputNames[i], STR_VSRCRAW[stick_index] + 1, 3);
#if LEN_INPUT_NAME > 3
    g_model.inputNames[i][3] = '\0';
#endif
  }
  storageDirty(EE_MODEL);
}

void clearMixes()
{
  memset(g_model.mixData, 0, sizeof(g_model.mixData));
}

void setDefaultMixes()
{
  for (int i=0; i<NUM_STICKS; i++) {
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

void setVendorSpecificModelDefaults(uint8_t id)
{
#if defined(FRSKY_RELEASE)
  g_model.moduleData[INTERNAL_MODULE].type = IS_PXX2_INTERNAL_ENABLED() ? MODULE_TYPE_ISRM_PXX2 : MODULE_TYPE_XJT_PXX1;
  g_model.moduleData[INTERNAL_MODULE].channelsCount = defaultModuleChannels_M8(INTERNAL_MODULE);
  #if defined(EEPROM)
    g_model.header.modelId[INTERNAL_MODULE] = findNextUnusedModelId(id, INTERNAL_MODULE);
    modelHeaders[id].modelId[INTERNAL_MODULE] = g_model.header.modelId[INTERNAL_MODULE];
  #endif
#endif

    // TODO: we should probably have some default trainer mode
    //       per radio, depending on what is supported
    //
#if defined(PCBXLITE)
  g_model.trainerData.mode = TRAINER_MODE_MASTER_BLUETOOTH;
#endif

#if defined(RADIOMASTER_RTF_RELEASE)
  // Those settings are for headless radio
  g_model.trainerData.mode = TRAINER_MODE_SLAVE;
  g_model.moduleData[INTERNAL_MODULE].type = MODULE_TYPE_MULTIMODULE;
  g_model.moduleData[INTERNAL_MODULE].setMultiProtocol(MODULE_SUBTYPE_MULTI_FRSKY);
  g_model.moduleData[INTERNAL_MODULE].subType = MM_RF_FRSKY_SUBTYPE_D8;
  g_model.moduleData[INTERNAL_MODULE].failsafeMode = FAILSAFE_NOPULSES;
#endif
}

void applyDefaultTemplate()
{
  setDefaultInputs();
  setDefaultMixes();
  setDefaultGVars();

  setDefaultModelRegistrationID();

#if defined(FUNCTION_SWITCHES)
  g_model.functionSwitchConfig = DEFAULT_FS_CONFIG;
  g_model.functionSwitchGroup = DEFAULT_FS_GROUPS;
  g_model.functionSwitchStartConfig = DEFAULT_FS_STARTUP_CONFIG;
  g_model.functionSwitchLogicalState = 0;
#endif

#if defined(COLORLCD)

#if !defined(GTESTS)
  loadDefaultLayout();
#endif

  // enable switch warnings
  #if defined(PCBPL18)
    // Due to some 2 and 3 position toggle switches on PL18EV, not meaningul to use all up default positions on all switches, thus following special default setting:
    // H & G middle, F up, E & D middle, C up, B middle, A up
    // The setting is 3 bits per switch: inactive = 000, up = 001, middle = 010, down = 011
    g_model.switchWarningState = 0b010010001010010001010001;
  #else
    // Not PL18/PL18EV, use all switches up setting:
    for (int i = 0; i < NUM_SWITCHES; i++) {
      g_model.switchWarningState |= (1 << (3*i));
    }
  #endif
#endif

  // TODO: what about switch warnings in non-color LCD radios?
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

#if defined(LUA) && defined(PCBTARANIS) // Horus uses menuModelWizard() for wizard
  VirtualFS &vfs = VirtualFS::instance();
  if (VirtualFS::instance().isFileAvailable(WIZARD_PATH "/" WIZARD_NAME)) {
    vfs.changeDirectory(WIZARD_PATH);
    luaExec(WIZARD_NAME);
  }
#endif
}
