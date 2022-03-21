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
#include "heartbeat_driver.h"
#include "aux_serial_driver.h"

int16_t ppmInput[MAX_TRAINER_CHANNELS];
uint8_t ppmInputValidityTimer;
uint8_t currentTrainerMode = 0xff;

void checkTrainerSignalWarning()
{
  enum {
    PPM_IN_IS_NOT_USED = 0,
    PPM_IN_IS_VALID,
    PPM_IN_INVALID
  };

  static uint8_t ppmInputValidState = PPM_IN_IS_NOT_USED;

  if (ppmInputValidityTimer && (ppmInputValidState == PPM_IN_IS_NOT_USED)) {
    ppmInputValidState = PPM_IN_IS_VALID;
  }
  else if (!ppmInputValidityTimer && (ppmInputValidState == PPM_IN_IS_VALID)) {
    ppmInputValidState = PPM_IN_INVALID;
    AUDIO_TRAINER_LOST();
  }
  else if (ppmInputValidityTimer && (ppmInputValidState == PPM_IN_INVALID)) {
    ppmInputValidState = PPM_IN_IS_VALID;
    AUDIO_TRAINER_BACK();
  }
}

void stopTrainer()
{
  switch (currentTrainerMode) {
    case TRAINER_MODE_MASTER_TRAINER_JACK:
      stop_trainer_capture();
      break;

    case TRAINER_MODE_SLAVE:
      stop_trainer_ppm();
      break;

#if defined(SBUS_TRAINER)
    case TRAINER_MODE_MASTER_SERIAL:
      sbusSetGetByte(nullptr);
      break;
#endif

#if defined(TRAINER_MODULE_CPPM)
    case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
      stop_trainer_module_cppm();
      break;
#endif

#if defined(TRAINER_MODULE_SBUS)
    case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
      sbusSetGetByte(nullptr);
      stop_trainer_module_sbus();
      break;
#endif
  }

#if defined(INTMODULE_HEARTBEAT_GPIO) && !defined(SIMU) && \
    (defined(TRAINER_MODULE_CPPM) || defined(TRAINER_MODULE_SBUS))
  if ((currentTrainerMode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE ||
       currentTrainerMode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE)
      && (isModulePXX2(INTERNAL_MODULE) || isModulePXX1(INTERNAL_MODULE))) {
    init_intmodule_heartbeat();
  }
#endif

  currentTrainerMode = 0xFF;
}

void checkTrainerSettings()
{
  uint8_t requiredTrainerMode = g_model.trainerData.mode;

  if (requiredTrainerMode != currentTrainerMode) {
    if (currentTrainerMode != 0xFF) {
      stopTrainer();
    }

    currentTrainerMode = requiredTrainerMode;

    switch (requiredTrainerMode) {
      case TRAINER_MODE_MASTER_TRAINER_JACK:
        init_trainer_capture();
        break;

      case TRAINER_MODE_SLAVE:
        init_trainer_ppm();
        break;

#if defined(SBUS_TRAINER)
      case TRAINER_MODE_MASTER_SERIAL:
        sbusSetGetByte(sbusAuxGetByte);
        break;
#endif

#if defined(TRAINER_MODULE_CPPM)
      case TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE:
        init_trainer_module_cppm();
        break;
#endif

#if defined(TRAINER_MODULE_SBUS)
      case TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE:
        init_trainer_module_sbus();
        sbusSetGetByte(trainerModuleSbusGetByte);
        break;
      case TRAINER_MODE_MASTER_IBUS_EXTERNAL_MODULE:
        init_trainer_module_ibus();
        sbusSetGetByte(trainerModuleSbusGetByte);
        break;
#endif
    }

#if defined(INTMODULE_HEARTBEAT_GPIO) && !defined(SIMU) && \
    (defined(TRAINER_MODULE_CPPM) || defined(TRAINER_MODULE_SBUS))
    if (requiredTrainerMode == TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE ||
        requiredTrainerMode == TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE) {
      stop_intmodule_heartbeat();
    }
#endif
  }
}
