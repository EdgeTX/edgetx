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
#include "io/frsky_firmware_update.h"
#include "tasks/mixer_task.h"

#include "pxx2_ota.h"
#include "pxx2_transport.h"

bool Pxx2OtaUpdate::waitStep(uint8_t step, uint8_t timeout)
{
  OtaUpdateInformation * destination = moduleState[module].otaUpdateInformation;
  uint8_t elapsed = 0;

  watchdogSuspend(100 /*1s*/);

  while (step != destination->step) {
    if (elapsed++ > timeout) {
      return false;
    }
    RTOS_WAIT_MS(1);
    telemetryWakeup();
  }

  return true;
}

const char* Pxx2OtaUpdate::nextStep(uint8_t step, const char* rxName,
                                    uint32_t address, const uint8_t* buffer)
{
  OtaUpdateInformation * destination = moduleState[module].otaUpdateInformation;

  destination->step = step;
  destination->address = address;

  for (uint8_t retry = 0;; retry++) {
    uint8_t* module_buffer = pulsesGetModuleBuffer(module);
    Pxx2Pulses pxx2(module_buffer);
    pxx2.sendOtaUpdate(module, rxName, address, (const char *) buffer);

    // send the frame immediately
    auto mod = pulsesGetModuleDriver(module);
    auto mod_st = (etx_module_state_t*)mod->ctx;

    auto drv = modulePortGetSerialDrv(mod_st->tx);
    auto ctx = modulePortGetCtx(mod_st->tx);
    drv->sendBuffer(ctx, module_buffer, pxx2.getSize());

    if (waitStep(step + 1, 20)) {
      return nullptr;
    }
    else if (retry == 100) {
      return "Transfer failed";
    }
  }
}

const char* Pxx2OtaUpdate::doFlashFirmware(const char* filename,
                                           ProgressHandler progressHandler)
{
  FIL file;
  uint8_t buffer[32];
  UINT count;
  const char * result;

  result = nextStep(OTA_UPDATE_START, rxName, 0, nullptr);
  if (result) {
    return result;
  }

  if (f_open(&file, filename, FA_READ) != FR_OK) {
    return "Open file failed";
  }

  uint32_t size;
  const char * ext = getFileExtension(filename);
  if (ext && !strcasecmp(ext, FRSKY_FIRMWARE_EXT)) {
    FrSkyFirmwareInformation * information = (FrSkyFirmwareInformation *) buffer;
    if (f_read(&file, buffer, sizeof(FrSkyFirmwareInformation), &count) != FR_OK ||
        count != sizeof(FrSkyFirmwareInformation)) {
      f_close(&file);
      return "Format error";
    }
    size = information->size;
  }
  else {
    size = f_size(&file);
  }

  uint32_t done = 0;
  while (1) {
    progressHandler(getBasename(filename), STR_OTA_UPDATE, done, size);
    if (f_read(&file, buffer, sizeof(buffer), &count) != FR_OK) {
      f_close(&file);
      return "Read file failed";
    }

    result = nextStep(OTA_UPDATE_TRANSFER, nullptr, done, buffer);
    if (result) {
      return result;
    }

    if (count < sizeof(buffer)) {
      f_close(&file);
      break;
    }

    done += count;
  }

  return nextStep(OTA_UPDATE_EOF, nullptr, done, nullptr);
}

void Pxx2OtaUpdate::flashFirmware(const char * filename, ProgressHandler progressHandler)
{
  mixerTaskStop();

  watchdogSuspend(100 /*1s*/);
  RTOS_WAIT_MS(100);

  moduleState[module].mode = MODULE_MODE_OTA_UPDATE;
  const char * result = doFlashFirmware(filename, progressHandler);
  moduleState[module].mode = MODULE_MODE_NORMAL;

  AUDIO_PLAY(AU_SPECIAL_SOUND_BEEP1 );
  BACKLIGHT_ENABLE();

  if (result) {
    POPUP_WARNING(STR_FIRMWARE_UPDATE_ERROR, result);
  }
  else {
    POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);
  }

  watchdogSuspend(100);
  RTOS_WAIT_MS(100);

  mixerTaskStart();
}


