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

#pragma once

#include <QtCore/QString>
#include <math.h>
#include <gtest/gtest.h>

#define SWAP_DEFINED
#include "edgetx.h"
#include "model_init.h"
#include "switches.h"
#include "hal/switch_driver.h"

#define CHANNEL_MAX (1024*256)

extern int32_t lastAct;

// from hal/adc_driver.cpp
extern void anaResetFiltered();
extern void anaSetFiltered(uint8_t chan, uint16_t val);

void doMixerCalculations();

extern const char * zchar2string(const char * zstring, int size);
extern const char * nchar2string(const char * string, int size);
#define EXPECT_ZSTREQ(c_string, z_string)   EXPECT_STREQ(c_string, zchar2string(z_string, sizeof(z_string)))
#define EXPECT_STRNEQ(c_string, n_string)   EXPECT_STREQ(c_string, nchar2string(n_string, sizeof(n_string)))

#if defined(PCBFRSKY)
#define RADIO_RESET() \
  g_eeGeneral.switchConfig = 0x00007bff
#else
  #define RADIO_RESET()
#endif

inline void SYSTEM_RESET()
{
#if !defined(STORAGE_MODELSLIST)
  memset(modelHeaders, 0, sizeof(modelHeaders));
#endif
  generalDefault();
  g_eeGeneral.templateSetup = 0;
  for (int i=0; i<switchGetMaxSwitches(); i++) {
    simuSetSwitch(i, -1);
  }
}

inline void MODEL_RESET()
{
  memset(&g_model, 0, sizeof(g_model));
  anaResetFiltered();
  extern uint8_t s_mixer_first_run_done;
  s_mixer_first_run_done = false;
  evalMixes(1);  // this is needed to reset fp_act
  lastFlightMode = 255;
}

inline void MIXER_RESET()
{
  memset(channelOutputs, 0, sizeof(channelOutputs));
  memset(chans, 0, sizeof(chans));
  memset(ex_chans, 0, sizeof(ex_chans));
  memset(act, 0, sizeof(act));
  memset(mixState, 0, sizeof(mixState));
  mixerCurrentFlightMode = lastFlightMode = 0;
  lastAct = 0;
  logicalSwitchesReset();
}

inline void TELEMETRY_RESET()
{
  telemetryData.clear();
  telemetryData.rssi.set(100);
  for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
    telemetryItems[i].clear();
  }
  memclear(g_model.telemetrySensors, sizeof(g_model.telemetrySensors));
}

class OpenTxTest : public testing::Test 
{
  protected:  // You should make the members protected s.t. they can be
              // accessed from sub-classes.

    // virtual void SetUp() will be called before each test is run.  You
    // should define it if you need to initialize the varaibles.
    // Otherwise, this can be skipped.
    virtual void SetUp() 
    {
      SYSTEM_RESET();
      MODEL_RESET();
      MIXER_RESET();
      setModelDefaults();
      RADIO_RESET();
    }
};
