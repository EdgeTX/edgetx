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
#include "timers_driver.h"

#if defined(MULTIMODULE)
  #include "pulses/multi.h"
  #if defined(MULTI_PROTOLIST)
    #include "io/multi_protolist.h"
  #endif
#endif

uint8_t   storageDirtyMsk;
tmr10ms_t storageDirtyTime10ms;

#if defined(RTC_BACKUP_RAM)
uint8_t   rambackupDirtyMsk = EE_GENERAL | EE_MODEL;
tmr10ms_t rambackupDirtyTime10ms;
#endif

void storageDirty(uint8_t msk)
{
  storageDirtyMsk |= msk;
  storageDirtyTime10ms = get_tmr10ms();

#if defined(RTC_BACKUP_RAM)
  rambackupDirtyMsk = storageDirtyMsk;
  rambackupDirtyTime10ms = storageDirtyTime10ms;
#endif
}

void preModelLoad()
{
  watchdogSuspend(500/*5s*/);

#if defined(SDCARD)
  logsClose();
#endif

  pulsesStop();
  stopTrainer();
#if defined(COLORLCD)
  deleteCustomScreens();
#endif

  RTOS_WAIT_MS(200);
}

void postRadioSettingsLoad()
{
#if defined(PXX2)
  if (is_memclear(g_eeGeneral.ownerRegistrationID, PXX2_LEN_REGISTRATION_ID)) {
    setDefaultOwnerId();
  }
#endif
#if defined(PCBX12S)
  // AUX2 is hardwired to AUX2 on X12S
  serialSetMode(SP_AUX2, UART_MODE_GPS);
#endif
#if defined(USB_SERIAL)
  // default VCP to CLI if not configured
  // to something else as NONE.
  if (isInternalModuleCrossfire() && serialGetMode(SP_VCP) == UART_MODE_NONE) {
    serialSetMode(SP_VCP, UART_MODE_CLI);
  }
#endif
#if defined(DEFAULT_INTERNAL_MODULE)
  if (!g_eeGeneral.internalModule) {
    g_eeGeneral.internalModule = DEFAULT_INTERNAL_MODULE;
  }
#endif
#if defined(STICK_DEAD_ZONE)
  if (!g_eeGeneral.stickDeadZone) {
    g_eeGeneral.stickDeadZone = DEFAULT_STICK_DEADZONE;
  }
#endif
}

void postModelLoad(bool alarms)
{
#if defined(PXX2)
  if (is_memclear(g_model.modelRegistrationID, PXX2_LEN_REGISTRATION_ID)) {
    memcpy(g_model.modelRegistrationID, g_eeGeneral.ownerRegistrationID, PXX2_LEN_REGISTRATION_ID);
  }
#endif

#if defined(MULTIMODULE) && defined(MULTI_PROTOLIST)
  MultiRfProtocols::removeInstance(EXTERNAL_MODULE);
#endif

  AUDIO_FLUSH();
  flightReset(false);

  customFunctionsReset();

  restoreTimers();

  for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
    TelemetrySensor & sensor = g_model.telemetrySensors[i];
    if (sensor.type == TELEM_TYPE_CALCULATED && sensor.persistent) {
      telemetryItems[i].value = sensor.persistentValue;
      telemetryItems[i].timeout = 0; // make value visible even before the first new value is received)
    }
    else {
      telemetryItems[i].timeout = TELEMETRY_SENSOR_TIMEOUT_UNAVAILABLE;
    }
  }

  loadCurves();

#if defined(GUI)
  if (alarms) {
    checkAll();
    PLAY_MODEL_NAME();
  }
#endif
  pulsesStart();

#if defined(SDCARD)
  referenceModelAudioFiles();
#endif

#if defined(COLORLCD)
  loadCustomScreens();
#endif

  LOAD_MODEL_BITMAP();
  LUA_LOAD_MODEL_SCRIPTS();

  SEND_FAILSAFE_1S();
}

void storageFlushCurrentModel()
{
  saveTimers();

  for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
    TelemetrySensor & sensor = g_model.telemetrySensors[i];
    if (sensor.type == TELEM_TYPE_CALCULATED && sensor.persistent && sensor.persistentValue != telemetryItems[i].value) {
      sensor.persistentValue = telemetryItems[i].value;
      storageDirty(EE_MODEL);
    }
  }

  if (g_model.potsWarnMode == POTS_WARN_AUTO) {
    for (int i=0; i<NUM_POTS+NUM_SLIDERS; i++) {
      if (g_model.potsWarnEnabled & (1 << i)) {
        SAVE_POT_POSITION(i);
      }
    }
    storageDirty(EE_MODEL);
  }
}

#if !defined(STORAGE_MODELSLIST)
void selectModel(uint8_t idx)
{
#if !defined(COLORLCD)
  showMessageBox(STR_LOADINGMODEL);
#endif
  storageFlushCurrentModel();
  storageCheck(true); // force writing of current model data before this is changed
  g_eeGeneral.currModel = idx;
  storageDirty(EE_GENERAL);
  loadModel(idx);
}

uint8_t findEmptyModel(uint8_t id, bool down)
{
  uint8_t i = id;
  for (;;) {
    i = (MAX_MODELS + (down ? i+1 : i-1)) % MAX_MODELS;
    if (!modelExists(i)) break;
    if (i == id) return 0xff; // no free space in directory left
  }
  return i;
}

ModelHeader modelHeaders[MAX_MODELS];

void loadModelHeaders()
{
  for (uint32_t i=0; i<MAX_MODELS; i++) {
    loadModelHeader(i, &modelHeaders[i]);
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
#endif
