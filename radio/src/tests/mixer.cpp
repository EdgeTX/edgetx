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

#include "gtests.h"
#include "mixes.h"
#include "expos.h"
#include "curves.h"
#include "model_arena.h"
#include "hal/adc_driver.h"

class TrimsTest : public EdgeTxTest {};
class MixerTest : public EdgeTxTest {};

#define CHECK_NO_MOVEMENT(channel, value, duration) \
    for (int i=1; i<=(duration); i++) { \
      evalFlightModeMixes(e_perout_mode_normal, 1); \
      GTEST_ASSERT_EQ((value), chans[(channel)]); \
    }

#define CHECK_SLOW_MOVEMENT(channel, sign, duration, max_duration) \
    do { \
    for (int i=1; i<=(duration); i++) { \
      evalFlightModeMixes(e_perout_mode_normal, 1); \
      lastAct = lastAct + (sign) * (1<<19)/max_duration; \
      GTEST_ASSERT_EQ(256 * (lastAct >> 8), chans[(channel)]); \
    } \
    } while (0)

#define CHECK_DELAY(channel, duration) \
    do { \
      int32_t value = chans[(channel)]; \
      for (int i=1; i<=(duration); i++) { \
        evalFlightModeMixes(e_perout_mode_normal, 1); \
        GTEST_ASSERT_EQ(chans[(channel)], value); \
      } \
    } while (0)

#define CHECK_FLIGHT_MODE_TRANSITION(channel, duration, initValue, endValue) \
    do { \
      uint32_t delta = 0xffff / duration; \
      int32_t weightInit = 0xffff; \
      int32_t weightEnd = 0; \
      for (int i = 0; i <= (duration); i++) { \
        evalMixes(1); \
        GTEST_ASSERT_LE( abs(((initValue) * weightInit + (endValue) * weightEnd) / 0xffff - channelOutputs[(channel)]), 1); \
        weightInit = weightInit - delta; \
        weightEnd = weightEnd + delta; \
      } \
      for (int i = 0; i < 100; i++) { /* be sure the transition is finished*/ \
        evalMixes(1); \
      } \
    } while (0)

#define MIXSRC_CYC1    (MIXSRC_FIRST_HELI)
#define MIXSRC_CYC2    (MIXSRC_FIRST_HELI + 1)
#define MIXSRC_CYC3    (MIXSRC_FIRST_HELI + 2)

#if defined(SURFACE_RADIO)
  #define ELE_CHAN          0
  #define ELE_THRTRIMSW     1
  #define THR_STICK_MIN_THR_POS 0
  #define TRIM_SCALE(x) (x / 2)
#else
  #define ELE_CHAN          1
  #define ELE_THRTRIMSW     1
  #define THR_STICK_MIN_THR_POS -1024
  #define TRIM_SCALE(x) (x)
#endif

#define THR_CHAN          inputMappingGetThrottle()
#define THR_STICK         THR_CHAN
#define ELE_STICK         ELE_CHAN
#define AIL_STICK         3

#define MIXSRC_ELE        (MIXSRC_FIRST_STICK + ELE_CHAN)
#define MIXSRC_THR        (MIXSRC_FIRST_STICK + THR_CHAN)
#define MIXSRC_AIL        (MIXSRC_FIRST_STICK + AIL_STICK)

#define MIXSRC_TRIMELE    (MIXSRC_FIRST_TRIM + ELE_CHAN)
#define MIXSRC_TRIMTHR    (MIXSRC_FIRST_TRIM + THR_CHAN)

#define ELE_TRIM_SOURCE   (-1 -ELE_CHAN)
#define THR_TRIM_SOURCE   (-1 -THR_CHAN)


TEST_F(MixerTest, throttleInvert)
{
  // Mode 1 / reversed
  g_eeGeneral.stickMode = 0;
  g_model.throttleReversed = 1;
  anaSetFiltered(inputMappingConvertMode(THR_STICK), -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);

  // Mode 2 / reversed
  g_eeGeneral.stickMode = 1;
  g_model.throttleReversed = 1;
  anaSetFiltered(inputMappingConvertMode(THR_STICK), -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);

  // Mode 2 / normal
  g_eeGeneral.stickMode = 1;
  g_model.throttleReversed = 0;
  anaSetFiltered(inputMappingConvertMode(THR_STICK), -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024);
}

TEST_F(TrimsTest, throttleTrim)
{
  g_model.thrTrim = 1;
  // stick max + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK), +1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS + TRIM_SCALE(512));
  // stick min + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS + TRIM_SCALE(256));
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS);
#if defined(SURFACE_RADIO)
  // stick bellow 0  + trim max :no trim in reverse
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -512);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -512);
#endif
  // now the same tests with extended Trims
  g_model.extendedTrims = 1;
  // stick max + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS + TRIM_SCALE(2048));
  // stick min + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS + TRIM_SCALE(1024));
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS);
}

TEST_F(TrimsTest, invertedThrottlePlusThrottleTrim)
{
  g_model.throttleReversed = 1;
  g_model.thrTrim = 1;
  // stick max + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024);
  // stick max + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024); // no effect since we are in reverse mode
#else
  EXPECT_EQ(channelOutputs[2], -1024+256);
#endif
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024); // no effect since we are in reverse mode
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024+512);
#endif
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024); // no effect since we are in reverse mode
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024+512);
#endif
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);

  // now the same tests with extended Trims
  g_model.extendedTrims = 1;
  // stick max + trim max : no effect since we are in reverse mode
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024);
#if defined(SURFACE_RADIO)
  // stick max + trim mid: no effect since we are in reverse mode
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024);
#else
  // stick max + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024+1024);
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024+2048);
#endif
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);
  // stick min + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);
}

TEST_F(TrimsTest, throttleTrimWithZeroWeightOnThrottle)
{
  g_model.thrTrim = 1;
  // the input already exists
  ExpoData *expo = expoAddress(THR_STICK);
  expo->weight.setNumeric(0);
  // stick max + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 256);
  // stick max + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_LE(abs(channelOutputs[THR_CHAN] - 128), 1);  //can't use precise comparison here because of lower precision math on 9X
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 256);
  // stick min + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_LE(abs(channelOutputs[THR_CHAN] - 128), 1);
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);

  // now some tests with extended Trims
  g_model.extendedTrims = 1;
  // trim min + various stick positions = should always be same value
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);

  // trim max + various stick positions = should always be same value
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
}

TEST_F(TrimsTest, invertedThrottlePlusthrottleTrimWithZeroWeightOnThrottle)
{
  g_model.throttleReversed = 1;
  g_model.thrTrim = 1;
  // the input already exists
  ExpoData *expo = expoAddress(THR_STICK);
  expo->weight.setNumeric(0);
  // stick max + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
  // stick max + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_LE(abs(channelOutputs[THR_CHAN] - 128), 1);
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 256);
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
  // stick min + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_LE(abs(channelOutputs[THR_CHAN] - 128), 1);
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 256);

  // now some tests with extended Trims
  g_model.extendedTrims = 1;
  // trim min + various stick positions = should always be same value
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);

  // trim max + various stick positions = should always be same value
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 0);
}

TEST_F(TrimsTest, CopyTrimsToOffset)
{
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, -100); // -100 on elevator/steering
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext); // it disables all safety channels
  copyTrimsToOffset(ELE_CHAN);
  EXPECT_EQ(getTrimValue(0, ELE_STICK), -100); // unchanged
  EXPECT_EQ(GV_DECODE(g_model.limitData[ELE_CHAN].offset), -195);
}

TEST_F(TrimsTest, CopySticksToOffset)
{
  anaSetFiltered(inputMappingConvertMode(ELE_STICK), -100);
  evalMixes(1);
  copySticksToOffset(ELE_CHAN);
#if defined(STICK_DEAD_ZONE)
  EXPECT_EQ(GV_DECODE(g_model.limitData[ELE_CHAN].offset), -93);
#else
  EXPECT_EQ(GV_DECODE(g_model.limitData[ELE_CHAN].offset), -97);
#endif
}

TEST_F(TrimsTest, MoveTrimsToOffsets)
{
  // No trim idle only
  g_model.thrTrim = 0;
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  0);
  setTrimValue(0, MIXSRC_TRIMTHR - MIXSRC_FIRST_TRIM, 100);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, -100);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], TRIM_SCALE(200));  // THR output value is reflecting 100 trim
  moveTrimsToOffsets();
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMTHR - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(GV_DECODE(g_model.limitData[THR_CHAN].offset), TRIM_SCALE(195)); // value transferred
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(GV_DECODE(g_model.limitData[ELE_CHAN].offset), -195); // value transferred
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], 99); // THR output value is still reflecting 100 trim
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], 200); // THR output value is still reflecting 100 trim
#endif
}

TEST_F(TrimsTest, MoveTrimsToOffsetsWithTrimIdle)
{
  // Trim idle only
  g_model.thrTrim = 1;
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);  // Min stick
  g_model.limitData[THR_CHAN].offset = 0;
  g_model.limitData[ELE_CHAN].offset = 0;
  setTrimValue(0, MIXSRC_TRIMTHR - MIXSRC_FIRST_TRIM, 100);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, -100);
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], 228);  // THR output value is reflecting 100 trim idle
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], -568);  // THR output value is reflecting 100 trim idle
#endif
  moveTrimsToOffsets();

  // Trim affecting Throttle should not be affected
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMTHR - MIXSRC_FIRST_TRIM), 100);  // unchanged
  EXPECT_EQ(GV_DECODE(g_model.limitData[2].offset), 0); // unchanged

  // Other trims should
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM), 0);  // back to neutral
#if defined(SURFACE_RADIO)
  EXPECT_EQ(GV_DECODE(g_model.limitData[ELE_CHAN].offset), -195); // value transferred
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 228);  // THR output value is reflecting 100 trim idle
#else
  EXPECT_EQ(GV_DECODE(g_model.limitData[ELE_CHAN].offset), -195); // value transferred
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -568);  // THR output value is reflecting 100 trim idle
#endif
}

TEST_F(TrimsTest, MoveTrimsToOffsetsWithCrossTrims)
{
  // No trim idle only
  // Cross trims
  g_model.thrTrim = 0;
  g_model.limitData[THR_CHAN].offset = 0;
  g_model.limitData[ELE_CHAN].offset = 0;
  g_model.setThrottleStickTrimSource(MIXSRC_TRIMELE);
  ExpoData *expo = expoAddress(THR_CHAN);
  expo->trimSource = ELE_TRIM_SOURCE;
  expo = expoAddress(ELE_CHAN);
  expo->trimSource = THR_TRIM_SOURCE;

  anaSetFiltered(inputMappingConvertMode(THR_STICK),  0);
  anaSetFiltered(inputMappingConvertMode(ELE_STICK),  0);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, 100);
  setTrimValue(0, MIXSRC_TRIMTHR - MIXSRC_FIRST_TRIM, -100);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], TRIM_SCALE(200));  // THR output value is reflecting 100 Ele trim
  EXPECT_EQ(channelOutputs[ELE_CHAN], -200);             // ELE output value is reflecting -100 Thr trim
  moveTrimsToOffsets();
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], 99);  // THR output value remains unchanged
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], 200);  // THR output value remains unchanged
#endif
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMTHR - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(GV_DECODE(g_model.limitData[THR_CHAN].offset), TRIM_SCALE(195)); // value transferred
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(GV_DECODE(g_model.limitData[ELE_CHAN].offset), -195); // value transferred
}

TEST_F(TrimsTest, MoveTrimsToOffsetsWithCrosstrimsAndTrimIdle)
{
  // Trim idle only
  // Cross trims
  g_model.limitData[THR_CHAN].offset = 0;
  g_model.limitData[ELE_CHAN].offset = 0;
  g_model.thrTrim = 1;
  g_model.thrTrimSw = ELE_THRTRIMSW;
  ExpoData *expo = expoAddress(THR_CHAN);
  expo->trimSource = ELE_TRIM_SOURCE;
  expo = expoAddress(ELE_CHAN);
  expo->trimSource = THR_TRIM_SOURCE;
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);  // Min throttle
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, 100);
  setTrimValue(0, MIXSRC_TRIMTHR - MIXSRC_FIRST_TRIM, -100);
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], 228);  // THR output value is reflecting 100 ele trim idle
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], -568);  // THR output value is reflecting 100 ele trim idle
#endif
  moveTrimsToOffsets();

  // Trim affecting Throttle (now Ele because of crosstrims) should not be affected
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM), 100);  // unchanged
  EXPECT_EQ(GV_DECODE(g_model.limitData[2].offset), 0); // THR chan offset unchanged

  // Other trims should
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMTHR - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(GV_DECODE(g_model.limitData[ELE_CHAN].offset), -195); // Ele chan offset transferred
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], 228);  // THR output value is still reflecting 100 trim idle
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], -568);  // THR output value is still reflecting 100 trim idle
#endif
}

#if !defined(SURFACE_RADIO)
TEST_F(TrimsTest, InstantTrim)
{
  anaSetFiltered(AIL_STICK, 50);
  instantTrim();
#if defined(STICK_DEAD_ZONE)
  EXPECT_EQ(23, getTrimValue(0, AIL_STICK));
#else
  EXPECT_EQ(25, getTrimValue(0, AIL_STICK));
#endif
}

TEST_F(TrimsTest, InstantTrimNegativeCurve)
{
  ExpoData *expo = expoAddress(AIL_STICK);
  expo->curve.type = CURVE_REF_CUSTOM;
  expo->curve.value.setNumeric(1);
  curveHeaderAllocAt(0);  // allocate curve header
  g_modelArena.ensureSectionCapacity(ARENA_POINTS, 5);
  g_modelArena.ensureSectionCapacity(ARENA_POINTS, 5);
  curvePointsBase()[0] = -100;
  curvePointsBase()[1] = -75;
  curvePointsBase()[2] = -50;
  curvePointsBase()[3] = -25;
  curvePointsBase()[4] = 0;
  anaSetFiltered(AIL_STICK, 512);
  instantTrim();
#if defined(STICK_DEAD_ZONE)
  EXPECT_EQ(127, getTrimValue(0, AIL_STICK));
#else
  EXPECT_EQ(128, getTrimValue(0, AIL_STICK));
#endif
}
#endif

TEST(Curves, LinearIntpol)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  curveHeaderAllocAt(0);  // allocate curve header slot 0
  g_modelArena.ensureSectionCapacity(ARENA_POINTS, 5);
  g_modelArena.ensureSectionCapacity(ARENA_POINTS, 5);
  for (int8_t i=-2; i<=2; i++) {
    curvePointsBase()[2+i] = 50*i;
  }
  EXPECT_EQ(applyCustomCurve(-1024, 0), -1024);
  EXPECT_EQ(applyCustomCurve(0, 0), 0);
  EXPECT_EQ(applyCustomCurve(1024, 0), 1024);
  EXPECT_EQ(applyCustomCurve(-192, 0), -192);
}



TEST_F(MixerTest, InfiniteRecursiveChannels)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_CHANNEL, 1);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(1)).destCh = 1;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_CHANNEL, 2);
  (*mixAddress(1)).weight.setNumeric(100);
  (*mixAddress(2)).destCh = 2;
  (*mixAddress(2)).srcRaw = SourceRef_(SOURCE_TYPE_CHANNEL, 0);
  (*mixAddress(2)).weight.setNumeric(100);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[2], 0);
  EXPECT_EQ(chans[1], 0);
  EXPECT_EQ(chans[0], 0);
}

TEST_F(MixerTest, BlockingChannel)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_CHANNEL, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);
}

TEST_F(MixerTest, RecursiveAddChannel)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(50);
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_ADD;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_CHANNEL, 1);
  (*mixAddress(1)).weight.setNumeric(100);
  (*mixAddress(2)).destCh = 1;
  (*mixAddress(2)).srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  (*mixAddress(2)).weight.setNumeric(100);

  anaSetFiltered(0, 0);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX/2);
  EXPECT_EQ(chans[1], 0);
}

TEST_F(MixerTest, RecursiveAddChannelAfterInactivePhase)
{
  if (switchGetMaxAllSwitches() < 4) return;
  if (adcGetMaxInputs(ADC_INPUT_MAIN) < 3) return;  // needs 3 default mixes

  flightModeAddress(1)->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 1);
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_CHANNEL, 1);
  (*mixAddress(0)).flightModes = 0b11110;
  (*mixAddress(0)).weight.setNumeric(50);
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_ADD;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).flightModes = 0b11101;
  (*mixAddress(1)).weight.setNumeric(50);
  (*mixAddress(2)).destCh = 1;
  (*mixAddress(2)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(2)).weight.setNumeric(100);
  simuSetSwitch(3, -1);
  evalMixes(1);
  EXPECT_EQ(chans[0], CHANNEL_MAX/2);
  EXPECT_EQ(chans[1], CHANNEL_MAX);
  simuSetSwitch(3, 0);
  evalMixes(1);
  EXPECT_EQ(chans[0], CHANNEL_MAX/2);
  EXPECT_EQ(chans[1], CHANNEL_MAX);
}


TEST_F(MixerTest, SlowOnPhase)
{
  flightModeAddress(1)->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 0);
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(0)).flightModes = 0x2 + 0x4 + 0x8 + 0x10 /*only enabled in phase 0*/;
  (*mixAddress(0)).speedUp = 50;
  (*mixAddress(0)).speedDown = 50;

  s_mixer_first_run_done = true;
  mixerCurrentFlightMode = 0;
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);

  CHECK_SLOW_MOVEMENT(0, +1, 250, 500);

  mixerCurrentFlightMode = 1;
  CHECK_SLOW_MOVEMENT(0, -1, 250, 500);
}

TEST_F(MixerTest, SlowOnSwitchSource)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;

  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_SWITCH, (uint16_t)sw);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(0)).speedUp = 50;
  (*mixAddress(0)).speedDown = 50;

  s_mixer_first_run_done = true;

  simuSetSwitch(sw, -1);
  CHECK_SLOW_MOVEMENT(0, -1, 250, 500);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);

  simuSetSwitch(sw, 1);
  CHECK_SLOW_MOVEMENT(0, +1, 500, 500);
}

TEST_F(MixerTest, SlowOnPhasePrec10ms)
{
  flightModeAddress(1)->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 0);
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(0)).flightModes = 0x2 + 0x4 + 0x8 + 0x10 /*only enabled in phase 0*/;
  (*mixAddress(0)).speedUp = 50;
  (*mixAddress(0)).speedDown = 50;
  (*mixAddress(0)).speedPrec = 1;

  s_mixer_first_run_done = true;
  mixerCurrentFlightMode = 0;
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);

  CHECK_SLOW_MOVEMENT(0, +1, 25, 50);

  mixerCurrentFlightMode = 1;
  CHECK_SLOW_MOVEMENT(0, -1, 25, 50);
}

TEST_F(MixerTest, SlowOnSwitchSourcePrec10ms)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;

  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_SWITCH, (uint16_t)sw);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(0)).speedUp = 50;
  (*mixAddress(0)).speedDown = 50;
  (*mixAddress(0)).speedPrec = 1;

  s_mixer_first_run_done = true;

  simuSetSwitch(sw, -1);
  CHECK_SLOW_MOVEMENT(0, -1, 25, 50);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);

  simuSetSwitch(sw, 1);
  CHECK_SLOW_MOVEMENT(0, +1, 50, 50);
}

TEST_F(MixerTest, SlowDisabledOnStartup)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(0)).speedUp = 50;
  (*mixAddress(0)).speedDown = 50;

  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX);
}

TEST_F(MixerTest, DelayOnSwitch)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;
  uint16_t swPos = (uint16_t)(sw * 3 + 2);

  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(0)).swtch = SwitchRef_(SWITCH_TYPE_SWITCH, swPos);
  (*mixAddress(0)).delayUp = 50;
  (*mixAddress(0)).delayDown = 50;

  simuSetSwitch(sw, -1);

  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);

  simuSetSwitch(sw, 1);
  CHECK_DELAY(0, 500);

  evalFlightModeMixes(e_perout_mode_normal, 1);
  EXPECT_EQ(chans[0], CHANNEL_MAX);

  simuSetSwitch(sw, 0);
  CHECK_DELAY(0, 500);

  evalFlightModeMixes(e_perout_mode_normal, 1);
  EXPECT_EQ(chans[0], 0);
}

TEST_F(MixerTest, DelayOnSwitch2)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;

  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_SWITCH, (uint16_t)sw);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(0)).delayUp = 50;
  (*mixAddress(0)).delayDown = 50;

  simuSetSwitch(sw, -1);

  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);

  simuSetSwitch(sw, 1);
  CHECK_DELAY(0, 500);

  evalFlightModeMixes(e_perout_mode_normal, 1);
  EXPECT_EQ(chans[0], CHANNEL_MAX);

  simuSetSwitch(sw, 0);
  CHECK_DELAY(0, 500);

  evalFlightModeMixes(e_perout_mode_normal, 1);
  EXPECT_EQ(chans[0], 0);
}

TEST_F(MixerTest, SlowOnMultiply)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_MUL;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).weight.setNumeric(100);
  (*mixAddress(1)).swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 0);
  (*mixAddress(1)).speedUp = 50;
  (*mixAddress(1)).speedDown = 50;

  s_mixer_first_run_done = true;

  simuSetSwitch(0, 1);
  CHECK_SLOW_MOVEMENT(0, 1, 250, 500);

  simuSetSwitch(0, -1);
  CHECK_NO_MOVEMENT(0, CHANNEL_MAX, 250);

  simuSetSwitch(0, 1);
  CHECK_NO_MOVEMENT(0, CHANNEL_MAX, 250);
}

TEST_F(MixerTest, SlowOnMultiplyPrec10ms)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_MUL;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).weight.setNumeric(100);
  (*mixAddress(1)).swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 0);
  (*mixAddress(1)).speedUp = 50;
  (*mixAddress(1)).speedDown = 50;
  (*mixAddress(1)).speedPrec = 1;

  s_mixer_first_run_done = true;

  simuSetSwitch(0, 1);
  CHECK_SLOW_MOVEMENT(0, 1, 25, 50);

  simuSetSwitch(0, -1);
  CHECK_NO_MOVEMENT(0, CHANNEL_MAX, 250);

  simuSetSwitch(0, 1);
  CHECK_NO_MOVEMENT(0, CHANNEL_MAX, 250);
}

TEST_F(TrimsTest, throttleTrimEle) {
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_eeGeneral.templateSetup = 17; // WARNING: NOT RETA (TAER or TH/ST)
  applyDefaultTemplate();
  g_model.thrTrim = 1;
  // checks ELE sticks are not affected by throttleTrim
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(ELE_STICK), +1024);
  setTrimValue(0, ELE_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024 - 256);
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_eeGeneral.templateSetup = 0;
  applyDefaultTemplate();
}

#if defined(HELI)
TEST(Heli, BasicTest)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_model.swashR.collectiveSource = SourceRef_(SOURCE_TYPE_STICK, (uint16_t)THR_CHAN);
  g_model.swashR.elevatorSource = SourceRef_(SOURCE_TYPE_STICK, (uint16_t)ELE_CHAN);
  g_model.swashR.aileronSource = SourceRef_(SOURCE_TYPE_STICK, (uint16_t)AIL_STICK);
  g_model.swashR.collectiveWeight = 100;
  g_model.swashR.elevatorWeight = 100;
  g_model.swashR.aileronWeight = 100;
  g_model.swashR.type = SWASH_TYPE_120;
  (*mixAllocAt(0)).destCh = 0;
  (*mixAllocAt(0)).mltpx = MLTPX_ADD;
  (*mixAllocAt(0)).srcRaw = SourceRef_(SOURCE_TYPE_HELI, 0);
  (*mixAllocAt(0)).weight.setNumeric(100);
  (*mixAllocAt(1)).destCh = 1;
  (*mixAllocAt(1)).mltpx = MLTPX_ADD;
  (*mixAllocAt(1)).srcRaw = SourceRef_(SOURCE_TYPE_HELI, 1);
  (*mixAllocAt(1)).weight.setNumeric(100);
  (*mixAllocAt(2)).destCh = 2;
  (*mixAllocAt(2)).mltpx = MLTPX_ADD;
  (*mixAllocAt(2)).srcRaw = SourceRef_(SOURCE_TYPE_HELI, 2);
  (*mixAllocAt(2)).weight.setNumeric(100);
  updateMixCount();
  anaSetFiltered(inputMappingConvertMode(THR_STICK), 0);
  anaSetFiltered(inputMappingConvertMode(ELE_STICK), 1024);
  anaSetFiltered(inputMappingConvertMode(AIL_STICK), 0);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);
  EXPECT_EQ(chans[1], CHANNEL_MAX/2);
  EXPECT_EQ(chans[2], CHANNEL_MAX/2);
}

TEST(Heli, Mode2Test)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_eeGeneral.templateSetup = 2;
  applyDefaultTemplate();
  g_model.swashR.collectiveSource = SourceRef_(SOURCE_TYPE_STICK, (uint16_t)THR_CHAN);
  g_model.swashR.elevatorSource = SourceRef_(SOURCE_TYPE_STICK, (uint16_t)ELE_CHAN);
  g_model.swashR.aileronSource = SourceRef_(SOURCE_TYPE_STICK, (uint16_t)AIL_STICK);
  g_model.swashR.collectiveWeight = 100;
  g_model.swashR.elevatorWeight = 100;
  g_model.swashR.aileronWeight = 100;
  g_model.swashR.type = SWASH_TYPE_120;
  (*mixAllocAt(0)).destCh = 0;
  (*mixAllocAt(0)).mltpx = MLTPX_ADD;
  (*mixAllocAt(0)).srcRaw = SourceRef_(SOURCE_TYPE_HELI, 0);
  (*mixAllocAt(0)).weight.setNumeric(100);
  (*mixAllocAt(1)).destCh = 1;
  (*mixAllocAt(1)).mltpx = MLTPX_ADD;
  (*mixAllocAt(1)).srcRaw = SourceRef_(SOURCE_TYPE_HELI, 1);
  (*mixAllocAt(1)).weight.setNumeric(100);
  (*mixAllocAt(2)).destCh = 2;
  (*mixAllocAt(2)).mltpx = MLTPX_ADD;
  (*mixAllocAt(2)).srcRaw = SourceRef_(SOURCE_TYPE_HELI, 2);
  (*mixAllocAt(2)).weight.setNumeric(100);
  updateMixCount();
  anaSetFiltered(inputMappingConvertMode(THR_STICK), 0);
  anaSetFiltered(inputMappingConvertMode(ELE_STICK), 1024);
  anaSetFiltered(inputMappingConvertMode(AIL_STICK), 0);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);
  EXPECT_EQ(chans[1], CHANNEL_MAX/2);
  EXPECT_EQ(chans[2], CHANNEL_MAX/2);
  SYSTEM_RESET();
}
#endif

TEST(Trainer, UnpluggedTest)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_TRAINER, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(0)).delayUp = 50;
  (*mixAddress(0)).delayDown = 50;
  trainerSetTimer(0);
  trainerInput[0] = 1024;
  CHECK_DELAY(0, 5000);
}

TEST_F(MixerTest, flightModeTransition)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  flightModeAddress(1)->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3 + 2));
  flightModeAddress(0)->fadeIn = 100;
  flightModeAddress(0)->fadeOut = 100;
  flightModeAddress(1)->fadeIn = 100;
  flightModeAddress(1)->fadeOut = 100;
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_REPL;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).flightModes = 0b11110;
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_REPL;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).flightModes = 0b11101;
  (*mixAddress(1)).weight.setNumeric(-10);
  evalMixes(1);
  simuSetSwitch(sw, 1);
  CHECK_FLIGHT_MODE_TRANSITION(0, 1000, 1024, -102);
}

TEST_F(MixerTest, flightModeOverflow)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  flightModeAddress(1)->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 2);
  flightModeAddress(0)->fadeIn = 100;
  flightModeAddress(0)->fadeOut = 100;
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_REPL;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).flightModes = 0;
  (*mixAddress(0)).weight.setNumeric(250);
  evalMixes(1);
  simuSetSwitch(0, 1);
  CHECK_FLIGHT_MODE_TRANSITION(0, 1000, 1024, 1024);
}

TEST_F(TrimsTest, throttleTrimWithCrossTrims)
{
  g_model.thrTrim = 1;
  g_model.thrTrimSw = ELE_THRTRIMSW;

  ExpoData *expo = expoAddress(THR_STICK);
  expo->trimSource = ELE_TRIM_SOURCE;

  expo = expoAddress(ELE_STICK);
  expo->trimSource = THR_TRIM_SOURCE;

  // stick max + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  anaSetFiltered(inputMappingConvertMode(ELE_STICK),  0);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK), THR_STICK_MIN_THR_POS);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS + TRIM_SCALE(512));
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK), THR_STICK_MIN_THR_POS);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS + TRIM_SCALE(256));
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK), THR_STICK_MIN_THR_POS);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);

  // now the same tests with extended Trims
  g_model.extendedTrims = 1;
  // stick max + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024+2048);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK), THR_STICK_MIN_THR_POS);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS + TRIM_SCALE(1024));
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  THR_STICK_MIN_THR_POS);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], THR_STICK_MIN_THR_POS);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
}

TEST_F(TrimsTest, invertedThrottlePlusThrottleTrimWithCrossTrims)
{
  g_model.throttleReversed = 1;
  g_model.thrTrim = 1;
  g_model.thrTrimSw = ELE_THRTRIMSW;
  ExpoData *expo = expoAddress(THR_STICK);
  expo->trimSource = ELE_TRIM_SOURCE;
  expo = expoAddress(ELE_STICK);
  expo->trimSource = THR_TRIM_SOURCE;

  // stick max + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  anaSetFiltered(inputMappingConvertMode(ELE_STICK),  0);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick max + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, 0);
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024); // no effect since we are in reverse mode
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024+256);
#endif
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_MIN);
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024); // no effect since we are in reverse mode
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024+512);
#endif
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);

  // now the same tests with extended Trims
  g_model.extendedTrims = 1;
  // stick max + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick max + trim mid
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, 0);
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024); // no effect since we are in reverse mode
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024+1024);
#endif
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  +1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MIN);
  evalMixes(1);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024); // no effect since we are in reverse mode
#else
  EXPECT_EQ(channelOutputs[THR_CHAN], -1024+2048);
#endif
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim max
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
  // stick min + trim min
  anaSetFiltered(inputMappingConvertMode(THR_STICK),  -1024);
  setTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], +1024);
  EXPECT_EQ(channelOutputs[ELE_CHAN], 0);
}

// ==========================================================================
// rc-soar.com documented behavior tests
// https://rc-soar.com/edgetx/index.php
// Enshrine mixer, flight-mode, and channel-cascade semantics so that
// refactoring does not silently break real-world user setups.
// ==========================================================================

// Multiplex ADD: lines accumulate additively on the same channel.
TEST_F(MixerTest, MultiplexAdd)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(60);
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_ADD;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).weight.setNumeric(40);

  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX);  // 60% + 40% = 100%
}

// Multiplex REPL: second line replaces whatever the first produced.
TEST_F(MixerTest, MultiplexReplace)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_REPL;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).weight.setNumeric(-50);

  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], -CHANNEL_MAX / 2);  // REPL overwrites to -50%
}

// Multiplex MUL: multiplies with the result of all lines above.
TEST_F(MixerTest, MultiplexMultiplyBasic)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(100);
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_MUL;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).weight.setNumeric(50);

  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX / 2);  // 100% * 50% = 50%
}

// Multiplex MUL is order-sensitive: ADD+ADD+MUL differs from ADD+MUL+ADD.
TEST_F(MixerTest, MultiplexMultiplyOrderSensitive)
{
  g_modelArena.ensureSectionCapacity(ARENA_MIXES, 3);

  // Case A: ADD(60) + ADD(40) + MUL(50) = (60+40)*50% = 50%
  *mixAddress(0) = {};
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(60);
  *mixAddress(1) = {};
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_ADD;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).weight.setNumeric(40);
  *mixAddress(2) = {};
  (*mixAddress(2)).destCh = 0;
  (*mixAddress(2)).mltpx = MLTPX_MUL;
  (*mixAddress(2)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(2)).weight.setNumeric(50);
  updateMixCount();

  evalFlightModeMixes(e_perout_mode_normal, 0);
  int32_t caseA = chans[0];
  EXPECT_EQ(caseA, CHANNEL_MAX / 2);  // 100% * 50% = 50%

  // Case B: ADD(60) + MUL(50) + ADD(40) = 60*50% + 40 = 30+40 = 70%
  MIXER_RESET();
  *mixAddress(0) = {};
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_ADD;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(60);
  *mixAddress(1) = {};
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_MUL;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).weight.setNumeric(50);
  *mixAddress(2) = {};
  (*mixAddress(2)).destCh = 0;
  (*mixAddress(2)).mltpx = MLTPX_ADD;
  (*mixAddress(2)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(2)).weight.setNumeric(40);
  updateMixCount();

  evalFlightModeMixes(e_perout_mode_normal, 0);
  int32_t caseB = chans[0];

  EXPECT_NE(caseA, caseB);  // order matters
}

// Weight-then-offset: output = (source * weight) + offset.
TEST_F(MixerTest, WeightThenOffset)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  (*mixAddress(0)).weight.setNumeric(50);
  (*mixAddress(0)).offset.setNumeric(50);

  // Stick at +100%: 50% of 1024 + 50% offset
  anaSetFiltered(inputMappingConvertMode(0), +1024);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  int32_t full = chans[0];

  // Stick at 0: 0 + 50% offset
  anaSetFiltered(inputMappingConvertMode(0), 0);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  int32_t mid = chans[0];

  // Stick at -100%: -50% + 50% offset = 0
  anaSetFiltered(inputMappingConvertMode(0), -1024);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  int32_t low = chans[0];

  // full should be ~100%, mid ~50%, low ~0%
  EXPECT_NEAR(full, CHANNEL_MAX, CHANNEL_MAX / 100);
  EXPECT_NEAR(mid, CHANNEL_MAX / 2, CHANNEL_MAX / 100);
  EXPECT_NEAR(low, 0, CHANNEL_MAX / 100);
}

// Cascaded channels bypass output clipping: a channel used as a mix source
// carries its internal (>100%) value, not the clipped output.
TEST_F(MixerTest, CascadedChannelBypassesOutputClipping)
{
  // CH0: stick at 200% weight (overdrives to 200% internally)
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  (*mixAddress(0)).weight.setNumeric(200);
  // CH1: reads CH0 at 50% weight — if unclipped, 200%*50% = 100%
  (*mixAddress(1)).destCh = 1;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_CHANNEL, 0);
  (*mixAddress(1)).weight.setNumeric(50);

  anaSetFiltered(inputMappingConvertMode(0), +1024);
  evalMixes(1);

  // CH0 output is clipped to 100%
  EXPECT_EQ(channelOutputs[0], 1024);
  // CH1 sees the unclipped CH0 (200%), applies 50% → 100%
  // If CH0 were clipped before cascading, CH1 would be only 50%.
  EXPECT_GT(channelOutputs[1], 512);  // must be more than 50%
  EXPECT_NEAR(channelOutputs[1], 1024, 2);  // should be ~100%
}

// Flight mode priority: FM1 has highest priority, FM0 is fallback.
// getFlightMode() iterates FM1..FM8, first match wins; FM0 returned if none.
TEST_F(MixerTest, FlightModePriority)
{
  // Find two distinct 3-pos switches
  int sw1 = findHwSwitch(SWITCH_3POS);
  int sw2 = findHwSwitch(SWITCH_3POS, sw1);
  if (sw2 < 0) return;  // not enough switches on this target

  auto sw1Up = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw1 * 3));
  auto sw2Up = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw2 * 3));

  // FM1 activated by sw1↑, FM2 by sw2↑
  flightModeAddress(1)->swtch = sw1Up;
  flightModeAddress(2)->swtch = sw2Up;

  // Three REPL mix lines, one per FM, using distinguishable weights:
  //   FM0-only: weight=10  FM1-only: weight=20  FM2-only: weight=30
  // flightModes bitmask: bit set = DISABLED in that FM.
  g_modelArena.ensureSectionCapacity(ARENA_MIXES, 3);
  *mixAddress(0) = {};
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).mltpx = MLTPX_REPL;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(0)).weight.setNumeric(10);
  (*mixAddress(0)).flightModes = ~1u & 0x1FF;  // enabled only in FM0

  *mixAddress(1) = {};
  (*mixAddress(1)).destCh = 0;
  (*mixAddress(1)).mltpx = MLTPX_REPL;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(1)).weight.setNumeric(20);
  (*mixAddress(1)).flightModes = ~2u & 0x1FF;  // enabled only in FM1

  *mixAddress(2) = {};
  (*mixAddress(2)).destCh = 0;
  (*mixAddress(2)).mltpx = MLTPX_REPL;
  (*mixAddress(2)).srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  (*mixAddress(2)).weight.setNumeric(30);
  (*mixAddress(2)).flightModes = ~4u & 0x1FF;  // enabled only in FM2
  updateMixCount();

  // Neither switch active → FM0 (fallback)
  simuSetSwitch(sw1, 0);
  simuSetSwitch(sw2, 0);
  evalMixes(1);
  EXPECT_NEAR(channelOutputs[0], 1024 * 10 / 100, 1);

  // Both switches active → FM1 wins (higher priority than FM2)
  simuSetSwitch(sw1, -1);
  simuSetSwitch(sw2, -1);
  evalMixes(1);
  EXPECT_NEAR(channelOutputs[0], 1024 * 20 / 100, 1);

  // Only sw2 active → FM2
  simuSetSwitch(sw1, 0);
  simuSetSwitch(sw2, -1);
  evalMixes(1);
  EXPECT_NEAR(channelOutputs[0], 1024 * 30 / 100, 1);
}

// Cumulative weight through cascaded channels:
// CH0 = stick * 80%, CH1 = CH0 * 25% → CH1 should be stick * 20%.
TEST_F(MixerTest, CascadedWeightMultiplication)
{
  (*mixAddress(0)).destCh = 0;
  (*mixAddress(0)).srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  (*mixAddress(0)).weight.setNumeric(80);
  (*mixAddress(1)).destCh = 1;
  (*mixAddress(1)).srcRaw = SourceRef_(SOURCE_TYPE_CHANNEL, 0);
  (*mixAddress(1)).weight.setNumeric(25);

  anaSetFiltered(inputMappingConvertMode(0), +1024);
  evalMixes(1);

  // CH0 = 80% of 1024 ≈ 819
  EXPECT_NEAR(channelOutputs[0], 1024 * 80 / 100, 2);
  // CH1 = 25% of CH0 = 20% of 1024 ≈ 205
  EXPECT_NEAR(channelOutputs[1], 1024 * 20 / 100, 2);
}

#if defined(GVARS)

class GVarLimitTest : public EdgeTxTest {};

TEST_F(GVarLimitTest, EncodingRoundTrip)
{
  // Verify GV_ENCODE/GV_DECODE round-trip for positive values
  EXPECT_EQ(GV_DECODE(GV_ENCODE(0)), 0);
  EXPECT_EQ(GV_DECODE(GV_ENCODE(500)), 500);
  EXPECT_EQ(GV_DECODE(GV_ENCODE(1000)), 1000);
  EXPECT_EQ(GV_DECODE(GV_ENCODE(1500)), 1500);

  // Verify round-trip for negative values
  EXPECT_EQ(GV_DECODE(GV_ENCODE(-1)), -1);
  EXPECT_EQ(GV_DECODE(GV_ENCODE(-500)), -500);
  EXPECT_EQ(GV_DECODE(GV_ENCODE(-1000)), -1000);
  EXPECT_EQ(GV_DECODE(GV_ENCODE(-1500)), -1500);

  // Verify encoded positive values have bit 15 clear (not detected as gvar)
  EXPECT_FALSE(GV_IS_GV_VALUE(GV_ENCODE(0)));
  EXPECT_FALSE(GV_IS_GV_VALUE(GV_ENCODE(500)));
  EXPECT_FALSE(GV_IS_GV_VALUE(GV_ENCODE(-500)));
  EXPECT_FALSE(GV_IS_GV_VALUE(GV_ENCODE(-1500)));
}

TEST_F(GVarLimitTest, GVarEncodingRoundTrip)
{
  // Verify gvar index encoding round-trip
  for (int idx = 0; idx < getGVarCount(); idx++) {
    int16_t encoded = GV_VALUE_FROM_INDEX(idx);
    EXPECT_TRUE(GV_IS_GV_VALUE(encoded)) << "idx=" << idx;
    EXPECT_EQ(GV_INDEX_FROM_VALUE(encoded), idx) << "idx=" << idx;
  }
  for (int idx = -1; idx >= -getGVarCount(); idx--) {
    int16_t encoded = GV_VALUE_FROM_INDEX(idx);
    EXPECT_TRUE(GV_IS_GV_VALUE(encoded)) << "idx=" << idx;
    EXPECT_EQ(GV_INDEX_FROM_VALUE(encoded), idx) << "idx=" << idx;
  }
}

TEST_F(GVarLimitTest, GVarNoCollisionWithNumeric)
{
  // Verify that valid numeric limit values are never detected as gvar
  for (int v = -1500; v <= 1500; v++) {
    int16_t encoded = GV_ENCODE(v);
    EXPECT_FALSE(GV_IS_GV_VALUE(encoded)) << "value=" << v;
  }
}

TEST_F(GVarLimitTest, GVarInOffset)
{
  // Set up a simple passthrough mix (input 0 -> channel 0)
  g_modelArena.ensureSectionCapacity(ARENA_MIXES, 1);
  MixData *mix = mixAddress(0);
  mix->destCh = 0;
  mix->srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  mix->weight.setNumeric(100);

  // Set GV1 = 200 (offset in tenths, so 20.0)
  GVAR_VALUE(0, 0) = 200;

  // Set limit offset to GV1 (index 0)
  LimitData *ld = limitAddress(0);
  ld->offset = GV_VALUE_FROM_INDEX(0);

  EXPECT_TRUE(GV_IS_GV_VALUE(ld->offset));
  EXPECT_EQ(GV_INDEX_FROM_VALUE(ld->offset), 0);

  // Verify LIMIT_OFS resolves the gvar
  int32_t ofs = LIMIT_OFS(ld);
  EXPECT_EQ(ofs, 2000);  // GV1=200, getGVarValuePrec1 returns 200*10=2000
}

TEST_F(GVarLimitTest, GVarInMinMax)
{
  g_modelArena.ensureSectionCapacity(ARENA_MIXES, 1);
  MixData *mix = mixAddress(0);
  mix->destCh = 0;
  mix->srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  mix->weight.setNumeric(100);

  // GV1 = 800 (in tenths: 80.0%), GV2 = -500 (in tenths: -50.0%)
  GVAR_VALUE(0, 0) = 800;
  GVAR_VALUE(1, 0) = -500;

  LimitData *ld = limitAddress(0);

  // Set max to GV1 (index 0)
  ld->max = GV_VALUE_FROM_INDEX(0);
  EXPECT_TRUE(GV_IS_GV_VALUE(ld->max));
  int32_t maxVal = LIMIT_MAX(ld);
  EXPECT_EQ(maxVal, 8000);  // GV1=800, prec1=8000

  // Set min to -GV2 (index -2, i.e., negated GV2)
  ld->min = GV_VALUE_FROM_INDEX(-2);
  EXPECT_TRUE(GV_IS_GV_VALUE(ld->min));
  int32_t minVal = LIMIT_MIN(ld);
  EXPECT_EQ(minVal, 5000);  // -GV2 = -(-500) = 500, prec1=5000
}

TEST_F(GVarLimitTest, MixerAppliesGVarLimits)
{
  // Set up a passthrough mix
  g_modelArena.ensureSectionCapacity(ARENA_MIXES, 1);
  MixData *mix = mixAddress(0);
  mix->destCh = 0;
  mix->srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  mix->weight.setNumeric(100);

  // GV1 = 50 -> LIMIT_MAX = prec1(50) = 500 -> calc1000toRESX(500) = 512
  GVAR_VALUE(0, 0) = 50;
  LimitData *ld = limitAddress(0);
  ld->max = GV_VALUE_FROM_INDEX(0);
  ld->min = GV_ENCODE(0);  // min at default (0 + offset = -100%)

  // Full positive stick
  anaSetFiltered(inputMappingConvertMode(0), 1024);
  evalMixes(1);

  // The max limit resolves to GV1*10 = 500 tenths -> RESX 512
  // Output should be clamped to that limit
  int16_t output = channelOutputs[0];
  EXPECT_GT(output, 0);
  EXPECT_LE(output, 512);  // clamped at ~5% of full range
}

TEST_F(GVarLimitTest, NumericLimitNegativeValues)
{
  // Verify negative numeric values in LimitData work through the mixer
  g_modelArena.ensureSectionCapacity(ARENA_MIXES, 1);
  MixData *mix = mixAddress(0);
  mix->destCh = 0;
  mix->srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  mix->weight.setNumeric(100);

  LimitData *ld = limitAddress(0);
  // Set offset to -200 (tenths: -20.0)
  ld->offset = GV_ENCODE(-200);

  EXPECT_FALSE(GV_IS_GV_VALUE(ld->offset));
  EXPECT_EQ(GV_DECODE(ld->offset), -200);
  EXPECT_EQ(LIMIT_OFS(ld), -200);  // numeric offset returned directly

  // Zero stick, offset should shift output negative
  anaSetFiltered(inputMappingConvertMode(0), 0);
  evalMixes(1);
  EXPECT_LT(channelOutputs[0], 0);
}

#endif // GVARS
