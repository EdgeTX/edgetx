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

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "edgetx.h"
#include "timers.h"
#include "model_init.h"

void eeLoadModel(uint8_t index)
{
  if (index < MAX_MODELS) {
    preModelLoad();

    uint16_t size = eeLoadModelData(index);

#if defined(SIMU) && defined(EEPROM_ZONE_SIZE)
    if (sizeof(uint16_t) + sizeof(g_model) > EEPROM_ZONE_SIZE) {
      TRACE("Model data size can't exceed %d bytes (%d bytes)", int(EEPROM_ZONE_SIZE-sizeof(uint16_t)), (int)sizeof(g_model));
    }
#endif

#if defined(SIMU)
    if (size > 0 && size != sizeof(g_model)) {
      TRACE("Model data read=%d bytes vs %d bytes\n", size, (int)sizeof(ModelData));
    }
#endif

    bool alarms = true;
    if (size < EEPROM_MIN_MODEL_SIZE) { // if not loaded a fair amount
      setModelDefaults(index);
      storageCheck(true);
      alarms = false;
    }

    postModelLoad(alarms);
  }
}

uint8_t eeFindEmptyModel(uint8_t id, bool down)
{
  uint8_t i = id;
  for (;;) {
    i = (MAX_MODELS + (down ? i+1 : i-1)) % MAX_MODELS;
    if (!eeModelExists(i)) break;
    if (i == id) return 0xff; // no free space in directory left
  }
  return i;
}

void selectModel(uint8_t sub)
{
#if !defined(COLORLCD)
  showMessageBox(STR_LOADINGMODEL);
#endif
  storageFlushCurrentModel();
  storageCheck(true); // force writing of current model data before this is changed
  g_eeGeneral.currModel = sub;
  storageDirty(EE_GENERAL);
  eeLoadModel(sub);
}

ModelHeader modelHeaders[MAX_MODELS];
void eeLoadModelHeaders()
{
  for (uint32_t i=0; i<MAX_MODELS; i++) {
    eeLoadModelHeader(i, &modelHeaders[i]);
  }
}

void storageClearRadioSettings()
{
  memclear(&g_eeGeneral, sizeof(RadioData));
}

bool storageReadRadioSettings(bool allowFixes)
{
  if (!eepromOpen() || !eeLoadGeneral(allowFixes)) {
    if (!allowFixes) {
      storageClearRadioSettings();
      return false;
    }
    storageEraseAll(true);
  }
  else {
    eeLoadModelHeaders();
  }

  for (uint8_t i=0; languagePacks[i]; i++) {
    if (!strncmp(g_eeGeneral.ttsLanguage, languagePacks[i]->id, 2)) {
      currentLanguagePackIdx = i;
      currentLanguagePack = languagePacks[i];
    }
  }

  postRadioSettingsLoad();

  return true;
}

void storageReadCurrentModel()
{
  eeLoadModel(g_eeGeneral.currModel);
}

void storageReadAll()
{
  storageReadRadioSettings();
  storageReadCurrentModel();
}

void storageEraseAll(bool warn)
{
  TRACE("storageEraseAll");

  generalDefault();
  setModelDefaults(0);

  if (warn) {
    ALERT(STR_STORAGE_WARNING, STR_BAD_RADIO_DATA, AU_BAD_RADIODATA);
  }

  RAISE_ALERT(STR_STORAGE_WARNING, STR_STORAGE_FORMAT, nullptr, AU_NONE);

  storageFormat();
  storageDirty(EE_GENERAL|EE_MODEL);
  storageCheck(true);
}

void checkModelIdUnique(uint8_t index, uint8_t module)
{
  if (isModuleXJTD8(module))
    return;

  uint8_t modelId = g_model.header.modelId[module];
  uint8_t additionalOnes = 0;
  char * name = reusableBuffer.moduleSetup.msg;

  memset(reusableBuffer.moduleSetup.msg, 0, sizeof(reusableBuffer.moduleSetup.msg));

  if (modelId != 0) {
    for (uint8_t i = 0; i < MAX_MODELS; i++) {
      if (i != index) {
        if (modelId == modelHeaders[i].modelId[module]) {
          if ((WARNING_LINE_LEN - 4 - (name - reusableBuffer.moduleSetup.msg)) > (signed)(modelHeaders[i].name[0] ? zlen(modelHeaders[i].name, LEN_MODEL_NAME) : sizeof(TR_MODEL) + 2)) { // you cannot rely exactly on WARNING_LINE_LEN so using WARNING_LINE_LEN-2 (-2 for the ",")
            if (reusableBuffer.moduleSetup.msg[0] != '\0') {
              name = strAppend(name, ", ");
            }
            if (modelHeaders[i].name[0] == 0) {
              name = strAppend(name, STR_MODEL);
              name = strAppendUnsigned(name+strlen(name), i + 1, 2);
            }
            else {
              name += zchar2str(name, modelHeaders[i].name, LEN_MODEL_NAME);
            }
          }
          else {
            additionalOnes++;
          }
        }
      }
    }
  }

  if (additionalOnes) {
    name = strAppend(name, " (+");
    name = strAppendUnsigned(name, additionalOnes);
    strAppend(name, ")");
  }

  if (reusableBuffer.moduleSetup.msg[0]) {
    POPUP_WARNING(STR_MODELIDUSED, reusableBuffer.moduleSetup.msg);
  }
}

uint8_t findNextUnusedModelId(uint8_t index, uint8_t module)
{
  uint8_t usedModelIds[(MAX_RXNUM + 7) / 8];
  memset(usedModelIds, 0, sizeof(usedModelIds));

  for (uint8_t modelIndex = 0; modelIndex < MAX_MODELS; modelIndex++) {
    if (modelIndex == index)
      continue;

    uint8_t id = modelHeaders[modelIndex].modelId[module];
    if (id == 0)
      continue;

    uint8_t mask = 1u << (id & 7u);
    usedModelIds[id >> 3u] |= mask;
  }

  for (uint8_t id = 1; id <= getMaxRxNum(module); id++) {
    uint8_t mask = 1u << (id & 7u);
    if (!(usedModelIds[id >> 3u] & mask)) {
      // found free ID
      return id;
    }
  }

  // failed finding something...
  return 0;
}
