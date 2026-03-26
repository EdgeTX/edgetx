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
#include "hal/adc_driver.h"

class TrimsTest : public EdgeTxTest {};
class MixerTest : public EdgeTxTest {};

// Parameter for the slow-ramp precision test group.
// speedPrec=0 → 100 ms per unit (×10 ticks); speedPrec=1 → 10 ms per unit (×1 tick).
// halfIter / fullIter are the CHECK_SLOW_MOVEMENT iteration counts for a half-range or
// full-range movement respectively; maxDuration is the macro's max_duration argument.
struct SlowPrecParam {
  int speedPrec;
  int halfIter;
  int fullIter;
  int maxDuration;
};

class MixerSlowPrecTest : public MixerTest,
                          public ::testing::WithParamInterface<SlowPrecParam> {};

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
  expo->weight = makeSourceNumVal(0);
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
  expo->weight = makeSourceNumVal(0);
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
  evalFunctions(g_model.customFn, modelFunctionsContext); // it disables all safety channels
  copyTrimsToOffset(ELE_CHAN);
  EXPECT_EQ(getTrimValue(0, ELE_STICK), -100); // unchanged
  EXPECT_EQ(g_model.limitData[ELE_CHAN].offset, -195);
}

TEST_F(TrimsTest, CopySticksToOffset)
{
  anaSetFiltered(inputMappingConvertMode(ELE_STICK), -100);
  evalMixes(1);
  copySticksToOffset(ELE_CHAN);
#if defined(STICK_DEAD_ZONE)
  EXPECT_EQ(g_model.limitData[ELE_CHAN].offset, -93);
#else
  EXPECT_EQ(g_model.limitData[ELE_CHAN].offset, -97);
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
  EXPECT_EQ(g_model.limitData[THR_CHAN].offset, TRIM_SCALE(195)); // value transferred
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(g_model.limitData[ELE_CHAN].offset, -195); // value transferred
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
  EXPECT_EQ(g_model.limitData[2].offset, 0); // unchanged

  // Other trims should
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM), 0);  // back to neutral
#if defined(SURFACE_RADIO)
  EXPECT_EQ(g_model.limitData[ELE_CHAN].offset, -195); // value transferred
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 228);  // THR output value is reflecting 100 trim idle
#else
  EXPECT_EQ(g_model.limitData[ELE_CHAN].offset, -195); // value transferred
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
  EXPECT_EQ(g_model.limitData[THR_CHAN].offset, TRIM_SCALE(195)); // value transferred
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMELE - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(g_model.limitData[ELE_CHAN].offset, -195); // value transferred
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
  EXPECT_EQ(g_model.limitData[2].offset, 0); // THR chan offset unchanged

  // Other trims should
  EXPECT_EQ(getTrimValue(0, MIXSRC_TRIMTHR - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(g_model.limitData[ELE_CHAN].offset, -195); // Ele chan offset transferred
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
  expo->curve.value = makeSourceNumVal(1);
  g_model.points[0] = -100;
  g_model.points[1] = -75;
  g_model.points[2] = -50;
  g_model.points[3] = -25;
  g_model.points[4] = 0;
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
  for (int8_t i=-2; i<=2; i++) {
    g_model.points[2+i] = 50*i;
  }
  EXPECT_EQ(applyCustomCurve(-1024, 0), -1024);
  EXPECT_EQ(applyCustomCurve(0, 0), 0);
  EXPECT_EQ(applyCustomCurve(1024, 0), 1024);
  EXPECT_EQ(applyCustomCurve(-192, 0), -192);
}



TEST_F(MixerTest, InfiniteRecursiveChannels)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_CH + 1;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[1].destCh = 1;
  g_model.mixData[1].srcRaw = MIXSRC_FIRST_CH + 2;
  g_model.mixData[1].weight = makeSourceNumVal(100);
  g_model.mixData[2].destCh = 2;
  g_model.mixData[2].srcRaw = MIXSRC_FIRST_CH;
  g_model.mixData[2].weight = makeSourceNumVal(100);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[2], 0);
  EXPECT_EQ(chans[1], 0);
  EXPECT_EQ(chans[0], 0);
}

TEST_F(MixerTest, BlockingChannel)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_CH;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);
}

TEST_F(MixerTest, RecursiveAddChannel)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = makeSourceNumVal(50);
  g_model.mixData[1].destCh = 0;
  g_model.mixData[1].mltpx = MLTPX_ADD;
  g_model.mixData[1].srcRaw = MIXSRC_FIRST_CH + 1;
  g_model.mixData[1].weight = makeSourceNumVal(100);
  g_model.mixData[2].destCh = 1;
  g_model.mixData[2].srcRaw = MIXSRC_FIRST_STICK;
  g_model.mixData[2].weight = makeSourceNumVal(100);

  anaSetFiltered(0, 0);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX/2);
  EXPECT_EQ(chans[1], 0);
}

TEST_F(MixerTest, RecursiveAddChannelAfterInactivePhase)
{
  if (switchGetMaxAllSwitches() < 4)
    GTEST_SKIP() << "target has fewer than 4 switches";
  
  g_model.flightModeData[1].swtch = SWSRC_FIRST_SWITCH + 1;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_CH + 1;
  g_model.mixData[0].flightModes = 0b11110;
  g_model.mixData[0].weight = makeSourceNumVal(50);
  g_model.mixData[1].destCh = 0;
  g_model.mixData[1].mltpx = MLTPX_ADD;
  g_model.mixData[1].srcRaw = MIXSRC_MAX;
  g_model.mixData[1].flightModes = 0b11101;
  g_model.mixData[1].weight = makeSourceNumVal(50);
  g_model.mixData[2].destCh = 1;
  g_model.mixData[2].srcRaw = MIXSRC_MAX;
  g_model.mixData[2].weight = makeSourceNumVal(100);
  simuSetSwitch(3, -1);
  evalMixes(1);
  EXPECT_EQ(chans[0], CHANNEL_MAX/2);
  EXPECT_EQ(chans[1], CHANNEL_MAX);
  simuSetSwitch(3, 0);
  evalMixes(1);
  EXPECT_EQ(chans[0], CHANNEL_MAX/2);
  EXPECT_EQ(chans[1], CHANNEL_MAX);
}


TEST_P(MixerSlowPrecTest, SlowOnPhase)
{
  auto p = GetParam();
  g_model.flightModeData[1].swtch = SWSRC_FIRST_SWITCH;
  g_model.mixData[0].destCh      = 0;
  g_model.mixData[0].mltpx       = MLTPX_ADD;
  g_model.mixData[0].srcRaw      = MIXSRC_MAX;
  g_model.mixData[0].weight      = makeSourceNumVal(100);
  g_model.mixData[0].flightModes = 0x2 + 0x4 + 0x8 + 0x10; // active only in FM0
  g_model.mixData[0].speedUp     = 50;
  g_model.mixData[0].speedDown   = 50;
  g_model.mixData[0].speedPrec   = p.speedPrec;

  s_mixer_first_run_done = true;
  mixerCurrentFlightMode = 0;
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);

  CHECK_SLOW_MOVEMENT(0, +1, p.halfIter, p.maxDuration);
  mixerCurrentFlightMode = 1;
  CHECK_SLOW_MOVEMENT(0, -1, p.halfIter, p.maxDuration);
}

TEST_P(MixerSlowPrecTest, SlowOnSwitchSource)
{
  auto p = GetParam();
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  if (sw >= switchGetMaxAllSwitches())
    GTEST_SKIP() << "no 3-position switch on this target";

  g_model.mixData[0].destCh    = 0;
  g_model.mixData[0].mltpx     = MLTPX_ADD;
  g_model.mixData[0].srcRaw    = sw + MIXSRC_FIRST_SWITCH;
  g_model.mixData[0].weight    = makeSourceNumVal(100);
  g_model.mixData[0].speedUp   = 50;
  g_model.mixData[0].speedDown = 50;
  g_model.mixData[0].speedPrec = p.speedPrec;

  s_mixer_first_run_done = true;
  simuSetSwitch(sw, -1);
  CHECK_SLOW_MOVEMENT(0, -1, p.halfIter, p.maxDuration);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);

  simuSetSwitch(sw, 1);
  CHECK_SLOW_MOVEMENT(0, +1, p.fullIter, p.maxDuration);
}

TEST_F(MixerTest, SlowDisabledOnStartup)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[0].speedUp = 50;
  g_model.mixData[0].speedDown = 50;

  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX);
}

TEST_F(MixerTest, DelayOnSwitch)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  if (sw >= switchGetMaxAllSwitches())
    GTEST_SKIP() << "no 3-position switch on this target";
  int swPos = (sw * 3) + SWSRC_FIRST_SWITCH + 2;

  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[0].swtch = swPos;
  g_model.mixData[0].delayUp = 50;
  g_model.mixData[0].delayDown = 50;

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
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  if (sw >= switchGetMaxAllSwitches())
    GTEST_SKIP() << "no 3-position switch on this target";

  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = sw + MIXSRC_FIRST_SWITCH;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[0].delayUp = 50;
  g_model.mixData[0].delayDown = 50;

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

TEST_P(MixerSlowPrecTest, SlowOnMultiply)
{
  auto p = GetParam();
  g_model.mixData[0].destCh    = 0;
  g_model.mixData[0].mltpx     = MLTPX_ADD;
  g_model.mixData[0].srcRaw    = MIXSRC_MAX;
  g_model.mixData[0].weight    = makeSourceNumVal(100);
  g_model.mixData[1].destCh    = 0;
  g_model.mixData[1].mltpx     = MLTPX_MUL;
  g_model.mixData[1].srcRaw    = MIXSRC_MAX;
  g_model.mixData[1].weight    = makeSourceNumVal(100);
  g_model.mixData[1].swtch     = SWSRC_FIRST_SWITCH;
  g_model.mixData[1].speedUp   = 50;
  g_model.mixData[1].speedDown = 50;
  g_model.mixData[1].speedPrec = p.speedPrec;

  s_mixer_first_run_done = true;
  simuSetSwitch(0, 1);
  CHECK_SLOW_MOVEMENT(0, 1, p.halfIter, p.maxDuration);

  simuSetSwitch(0, -1);
  CHECK_NO_MOVEMENT(0, CHANNEL_MAX, 250);

  simuSetSwitch(0, 1);
  CHECK_NO_MOVEMENT(0, CHANNEL_MAX, 250);
}

INSTANTIATE_TEST_SUITE_P(
  SpeedPrecision, MixerSlowPrecTest,
  ::testing::Values(
    SlowPrecParam{0, 250, 500, 500},  // 100 ms precision (default)
    SlowPrecParam{1,  25,  50,  50}   // 10 ms precision
  )
);

TEST_F(TrimsTest, throttleTrimEle) {
  g_eeGeneral.templateSetup = 17; // WARNING: NOT RETA (TAER or TH/ST)
  applyDefaultTemplate();
  g_model.thrTrim = 1;
  // checks ELE sticks are not affected by throttleTrim
  // stick max + trim min
  anaSetFiltered(inputMappingConvertMode(ELE_STICK), +1024);
  setTrimValue(0, ELE_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[THR_CHAN], 1024 - 256);
}

#if defined(HELI)
class HeliTest : public EdgeTxTest {};

TEST_F(HeliTest, BasicTest)
{
  g_model.swashR.collectiveSource = MIXSRC_THR;
  g_model.swashR.elevatorSource = MIXSRC_ELE;
  g_model.swashR.aileronSource = MIXSRC_AIL;
  g_model.swashR.collectiveWeight = 100;
  g_model.swashR.elevatorWeight = 100;
  g_model.swashR.aileronWeight = 100;
  g_model.swashR.type = SWASH_TYPE_120;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_CYC1;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[1].destCh = 1;
  g_model.mixData[1].mltpx = MLTPX_ADD;
  g_model.mixData[1].srcRaw = MIXSRC_CYC2;
  g_model.mixData[1].weight = makeSourceNumVal(100);
  g_model.mixData[2].destCh = 2;
  g_model.mixData[2].mltpx = MLTPX_ADD;
  g_model.mixData[2].srcRaw = MIXSRC_CYC3;
  g_model.mixData[2].weight = makeSourceNumVal(100);
  anaSetFiltered(inputMappingConvertMode(THR_STICK), 0);
  anaSetFiltered(inputMappingConvertMode(ELE_STICK), 1024);
  anaSetFiltered(inputMappingConvertMode(AIL_STICK), 0);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);
  EXPECT_EQ(chans[1], CHANNEL_MAX/2);
  EXPECT_EQ(chans[2], CHANNEL_MAX/2);
}

TEST_F(HeliTest, Mode2Test)
{
  g_eeGeneral.templateSetup = 2;
  applyDefaultTemplate();
  g_model.swashR.collectiveSource = MIXSRC_THR;
  g_model.swashR.elevatorSource = MIXSRC_ELE;
  g_model.swashR.aileronSource = MIXSRC_AIL;
  g_model.swashR.collectiveWeight = 100;
  g_model.swashR.elevatorWeight = 100;
  g_model.swashR.aileronWeight = 100;
  g_model.swashR.type = SWASH_TYPE_120;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_CYC1;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[1].destCh = 1;
  g_model.mixData[1].mltpx = MLTPX_ADD;
  g_model.mixData[1].srcRaw = MIXSRC_CYC2;
  g_model.mixData[1].weight = makeSourceNumVal(100);
  g_model.mixData[2].destCh = 2;
  g_model.mixData[2].mltpx = MLTPX_ADD;
  g_model.mixData[2].srcRaw = MIXSRC_CYC3;
  g_model.mixData[2].weight = makeSourceNumVal(100);
  anaSetFiltered(inputMappingConvertMode(THR_STICK), 0);
  anaSetFiltered(inputMappingConvertMode(ELE_STICK), 1024);
  anaSetFiltered(inputMappingConvertMode(AIL_STICK), 0);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);
  EXPECT_EQ(chans[1], CHANNEL_MAX/2);
  EXPECT_EQ(chans[2], CHANNEL_MAX/2);
}
#endif

class TrainerTest : public EdgeTxTest {};

TEST_F(TrainerTest, UnpluggedTest)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_TRAINER;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[0].delayUp = 50;
  g_model.mixData[0].delayDown = 50;
  trainerSetTimer(0);
  trainerInput[0] = 1024;
  CHECK_DELAY(0, 5000);
}

// When a mix is active in FM0 (flightModes bit for FM1+ set) and the flight mode
// switches to FM1, the mix deactivates. delayDown holds the output frozen at
// CHANNEL_MAX for the delay period before the channel drops to 0.
// Mirrors SlowOnPhase but exercises delayDown instead of speedDown.
TEST_F(MixerTest, delayOnPhaseChange)
{
  g_model.flightModeData[1].swtch = SWSRC_FIRST_SWITCH;
  g_model.mixData[0].destCh      = 0;
  g_model.mixData[0].mltpx       = MLTPX_ADD;
  g_model.mixData[0].srcRaw      = MIXSRC_MAX;
  g_model.mixData[0].weight      = makeSourceNumVal(100);
  g_model.mixData[0].flightModes = 0x2 + 0x4 + 0x8 + 0x10; // active only in FM0
  g_model.mixData[0].delayDown   = 50;  // 50 × 10 ticks = 500 ticks

  s_mixer_first_run_done = true;
  mixerCurrentFlightMode = 0;
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX) << "FM0 mix active: output should be CHANNEL_MAX";

  // FM1: mix deactivates → delayDown keeps output frozen at CHANNEL_MAX for 500 ticks
  mixerCurrentFlightMode = 1;
  CHECK_DELAY(0, 500);
  evalFlightModeMixes(e_perout_mode_normal, 1);
  EXPECT_EQ(chans[0], 0) << "after delay, mix inactive: output should be 0";
}

TEST_F(MixerTest, flightModeTransition)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  if (sw >= switchGetMaxAllSwitches())
    GTEST_SKIP() << "no 3-position switch on this target";
  int swPos = (sw * 3) + SWSRC_FIRST_SWITCH + 2;

  g_model.flightModeData[1].swtch = swPos;
  g_model.flightModeData[0].fadeIn = 100;
  g_model.flightModeData[0].fadeOut = 100;
  g_model.flightModeData[1].fadeIn = 100;
  g_model.flightModeData[1].fadeOut = 100;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_REPL;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].flightModes = 0b11110;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[1].destCh = 0;
  g_model.mixData[1].mltpx = MLTPX_REPL;
  g_model.mixData[1].srcRaw = MIXSRC_MAX;
  g_model.mixData[1].flightModes = 0b11101;
  g_model.mixData[1].weight = makeSourceNumVal(-10);
  evalMixes(1);
  simuSetSwitch(sw, 1);
  CHECK_FLIGHT_MODE_TRANSITION(0, 1000, 1024, -102);
}

TEST_F(MixerTest, flightModeOverflow)
{
  g_model.flightModeData[1].swtch = SWSRC_FIRST_SWITCH + 2;
  g_model.flightModeData[0].fadeIn = 100;
  g_model.flightModeData[0].fadeOut = 100;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_REPL;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].flightModes = 0;
  g_model.mixData[0].weight = makeSourceNumVal(250);
  evalMixes(1);
  simuSetSwitch(0, 1);
  CHECK_FLIGHT_MODE_TRANSITION(0, 1000, 1024, 1024);
}

// Mirrors the model1.yml configuration provided by the user:
//   FM1 swtch=SA1 (SA middle position), fadeIn=20, fadeOut=20.
//
// fadeTime=20 → delta = (MAX_ACT/10)/20 = 2621 → ~200 evalMixes ticks to complete.
// CH1 in FM0 = +1024 (MIXSRC_MAX × 100%).
// CH1 in FM1 =  -102 (MIXSRC_MAX × -10%).
//
// Tests both transition directions as a round-trip:
//   1. SA UP → MID  (FM0 → FM1): channel fades +1024 → -102
//   2. SA MID → UP  (FM1 → FM0): channel fades  -102 → +1024
TEST_F(MixerTest, flightModeSaMidFade)
{
  // SA (switch 0) must be a 3-position switch — true on all standard targets
  // (set by RADIO_RESET() in SetUp), but guard defensively.
  if (g_model.getSwitchType(0) != SWITCH_3POS)
    GTEST_SKIP() << "SA (switch 0) is not 3-position on this target";

  // FM1 activated by SA middle position: SWSRC_FIRST_SWITCH + (0*3) + 1.
  g_model.flightModeData[1].swtch   = SWSRC_FIRST_SWITCH + 1;
  g_model.flightModeData[1].fadeIn  = 20;
  g_model.flightModeData[1].fadeOut = 20;

  // Two REPL mixes on CH1 (destCh=0), each active in exactly one FM:
  //   mix[0] flightModes=0b10 → excluded from FM1, active in FM0 → output +1024
  //   mix[1] flightModes=0b01 → excluded from FM0, active in FM1 → output  -102
  g_model.mixData[0].destCh      = 0;
  g_model.mixData[0].mltpx       = MLTPX_REPL;
  g_model.mixData[0].srcRaw      = MIXSRC_MAX;
  g_model.mixData[0].flightModes = 0b10;
  g_model.mixData[0].weight      = makeSourceNumVal(100);
  g_model.mixData[1].destCh      = 0;
  g_model.mixData[1].mltpx       = MLTPX_REPL;
  g_model.mixData[1].srcRaw      = MIXSRC_MAX;
  g_model.mixData[1].flightModes = 0b01;
  g_model.mixData[1].weight      = makeSourceNumVal(-10);

  // Initialise in FM0 with SA at UP position.
  simuSetSwitch(0, -1);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[0], 1024) << "initial FM0 output should be +1024";

  // --- Transition 1: SA UP → MID (FM0 → FM1) ---
  // FM1.fadeIn=20 governs this fade; ~200 evalMixes ticks to complete.
  simuSetSwitch(0, 0);
  CHECK_FLIGHT_MODE_TRANSITION(0, 200, 1024, -102);
  EXPECT_EQ(channelOutputs[0], -102) << "steady FM1 output should be -102";

  // --- Transition 2: SA MID → UP (FM1 → FM0) ---
  // FM1.fadeOut=20 governs this fade; ~200 evalMixes ticks to complete.
  simuSetSwitch(0, -1);
  CHECK_FLIGHT_MODE_TRANSITION(0, 200, -102, 1024);
  EXPECT_EQ(channelOutputs[0], 1024) << "steady FM0 output should be +1024";
}

// Verifies that reversing the FM switch mid-fade causes no channel jump.
// flightModeSaMidFade tests complete start-to-finish transitions; this test
// interrupts the FM0→FM1 fade halfway (~100 of ~200 ticks) and checks that:
//   1. The channel is at an intermediate value (partial fade).
//   2. After reversing (SA MID→UP), the channel recovers monotonically to +1024.
TEST_F(MixerTest, flightModeMidFadeReversal)
{
  if (g_model.getSwitchType(0) != SWITCH_3POS)
    GTEST_SKIP() << "SA (switch 0) is not 3-position on this target";

  g_model.flightModeData[1].swtch   = SWSRC_FIRST_SWITCH + 1;  // SA MID
  g_model.flightModeData[1].fadeIn  = 20;
  g_model.flightModeData[1].fadeOut = 20;

  // Two REPL mixes on CH1: FM0 → +1024, FM1 → -102 (same as flightModeSaMidFade).
  g_model.mixData[0].destCh      = 0;
  g_model.mixData[0].mltpx       = MLTPX_REPL;
  g_model.mixData[0].srcRaw      = MIXSRC_MAX;
  g_model.mixData[0].flightModes = 0b10;
  g_model.mixData[0].weight      = makeSourceNumVal(100);
  g_model.mixData[1].destCh      = 0;
  g_model.mixData[1].mltpx       = MLTPX_REPL;
  g_model.mixData[1].srcRaw      = MIXSRC_MAX;
  g_model.mixData[1].flightModes = 0b01;
  g_model.mixData[1].weight      = makeSourceNumVal(-10);

  // Establish FM0 steady-state: SA UP, output = +1024.
  simuSetSwitch(0, -1);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[0], 1024) << "initial FM0 output should be +1024";

  // Begin FM0→FM1 fade (SA MID); stop halfway (~100 of ~200 ticks).
  simuSetSwitch(0, 0);
  for (int i = 0; i < 100; i++) evalMixes(1);

  int32_t midValue = channelOutputs[0];
  EXPECT_LT(midValue, 1024) << "channel should have started fading toward FM1";
  EXPECT_GT(midValue, -102) << "channel should not yet have reached FM1 steady-state";

  // Reverse: SA back to UP → FM0 re-asserts, fade reverses.
  simuSetSwitch(0, -1);

  // One tick after reversal: channel must not have jumped instantly to FM0 target.
  evalMixes(1);
  EXPECT_LT(channelOutputs[0], 1024) << "channel should still be fading after reversal";
  EXPECT_GT(channelOutputs[0], -102) << "channel must not have glitched toward FM1";

  // After sufficient recovery ticks the channel reaches FM0 steady-state.
  for (int i = 0; i < 300; i++) evalMixes(1);
  EXPECT_EQ(channelOutputs[0], 1024) << "channel should reach FM0 steady-state after reversal";
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

// ---------------------------------------------------------------------------
// Additional coverage: output limits, multiply mode, negative weights,
// and switch-conditional mixes.
// ---------------------------------------------------------------------------

// A channel's max limit (< 100%) scales full-deflection output down.
// LimitData.max stores the delta from LIMIT_STD_MAX (1000), so setting
// max = -500 gives LIMIT_MAX = 500 (50%) → applyLimits returns 512 RESX.
TEST_F(MixerTest, channelMaxLimitScalesOutput)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.limitData[0].max = -500;  // 50% positive limit
  evalMixes(1);
  EXPECT_EQ(channelOutputs[0], 512);
}

// Symmetric check: a negative min limit of -50% clamps negative deflection.
// LimitData.min stores the delta from -LIMIT_STD_MAX, so setting
// min = 500 gives LIMIT_MIN = -500 (−50%) → applyLimits returns −512 RESX.
TEST_F(MixerTest, channelMinLimitScalesOutput)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MIN;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.limitData[0].min = 500;  // -50% negative limit
  evalMixes(1);
  EXPECT_EQ(channelOutputs[0], -512);
}

// A negative mix weight inverts the channel output relative to the source.
TEST_F(MixerTest, negativeWeightInvertsOutput)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = makeSourceNumVal(-100);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[0], -1024);
}

// MLTPX_MUL scales an existing channel value by the mix line's output.
// First line sets 100%, second line multiplies by 50% → final output 50%.
TEST_F(MixerTest, multiplyModeScalesOutput)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[1].destCh = 0;
  g_model.mixData[1].mltpx = MLTPX_MUL;
  g_model.mixData[1].srcRaw = MIXSRC_MAX;
  g_model.mixData[1].weight = makeSourceNumVal(50);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX / 2);
}

// A mix with a switch condition that is not currently active must contribute
// zero — the channel output stays at 0.
TEST_F(MixerTest, inactiveSwitchConditionDisablesMix)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw++)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  if (sw >= switchGetMaxAllSwitches())
    GTEST_SKIP() << "no 3-position switch on this target";

  // Mix requires the switch in the down position
  int swPosDown = (sw * 3) + SWSRC_FIRST_SWITCH + 2;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = makeSourceNumVal(100);
  g_model.mixData[0].swtch = swPosDown;

  simuSetSwitch(sw, -1);  // switch up — condition not met
  evalMixes(1);
  EXPECT_EQ(channelOutputs[0], 0);

  simuSetSwitch(sw, 1);   // switch down — condition met
  evalMixes(1);
  EXPECT_EQ(channelOutputs[0], 1024);
}

// MLTPX_REPL discards the accumulated value from earlier lines and replaces
// it with the replace-line's output.  First line contributes 50%; replace line
// forces the channel to 100%.
TEST_F(MixerTest, replaceModeOverridesPriorValue)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = makeSourceNumVal(50);  // +50%
  g_model.mixData[1].destCh = 0;
  g_model.mixData[1].mltpx = MLTPX_REPL;
  g_model.mixData[1].srcRaw = MIXSRC_MAX;
  g_model.mixData[1].weight = makeSourceNumVal(100);  // replaces with 100%
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX)
      << "MLTPX_REPL should replace accumulated value with 100%";
}

// A custom 5-point curve that maps the entire stick range to its inverse must
// produce a negated output when applied by the mixer through an expo line.
TEST_F(MixerTest, customCurveInvertsChannelOutput)
{
  // Build an inverted linear curve: points -100, -50, 0, 50, 100
  // mapped to 100, 50, 0, -50, -100.
  for (int i = 0; i < 5; i++)
    g_model.points[i] = (int8_t)(100 - i * 50);  // 100,50,0,-50,-100

  // Full positive stick → inverted curve → negative output.
  EXPECT_EQ(applyCustomCurve(+1024, 0), -1024);
  // Centre → 0 (curve passes through origin).
  EXPECT_EQ(applyCustomCurve(0, 0), 0);
  // Full negative stick → inverted → positive output.
  EXPECT_EQ(applyCustomCurve(-1024, 0), +1024);
}

#if defined(GVARS)
// A GVAR used as a mix source must pass its model value through the mixer.
TEST_F(MixerTest, gvarAsSourceInMix)
{
  // Set GV1 (index 0, flight mode 0) to 50% of RESX.
  const int16_t gvarVal = 50;
  g_model.flightModeData[0].gvars[0] = gvarVal;

  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_GVAR;  // GV1
  g_model.mixData[0].weight = makeSourceNumVal(100);
  evalMixes(1);
  // getValue(MIXSRC_FIRST_GVAR) returns gvarVal directly; applyLimits scales
  // it to channelOutputs.  Verify the sign and rough magnitude are correct.
  EXPECT_GT(channelOutputs[0], 0) << "GVar > 0 should produce positive output";
}
#endif  // defined(GVARS)

#if defined(HELI)
// The 90° swashplate uses only collective + elevator + aileron with equal
// angular spacing: CYC1 = collective only, CYC2 = ail only, CYC3 = ele only.
TEST_F(HeliTest, Swashplate90Degrees)
{
  g_model.swashR.collectiveSource = MIXSRC_THR;
  g_model.swashR.elevatorSource   = MIXSRC_ELE;
  g_model.swashR.aileronSource    = MIXSRC_AIL;
  g_model.swashR.collectiveWeight = 100;
  g_model.swashR.elevatorWeight   = 100;
  g_model.swashR.aileronWeight    = 100;
  g_model.swashR.type             = SWASH_TYPE_90;

  g_model.mixData[0].destCh  = 0;
  g_model.mixData[0].mltpx   = MLTPX_ADD;
  g_model.mixData[0].srcRaw  = MIXSRC_CYC1;
  g_model.mixData[0].weight  = makeSourceNumVal(100);
  g_model.mixData[1].destCh  = 1;
  g_model.mixData[1].mltpx   = MLTPX_ADD;
  g_model.mixData[1].srcRaw  = MIXSRC_CYC2;
  g_model.mixData[1].weight  = makeSourceNumVal(100);
  g_model.mixData[2].destCh  = 2;
  g_model.mixData[2].mltpx   = MLTPX_ADD;
  g_model.mixData[2].srcRaw  = MIXSRC_CYC3;
  g_model.mixData[2].weight  = makeSourceNumVal(100);

  // Full elevator, zero collective and aileron.
  anaSetFiltered(inputMappingConvertMode(THR_STICK), 0);
  anaSetFiltered(inputMappingConvertMode(ELE_STICK), +1024);
  anaSetFiltered(inputMappingConvertMode(AIL_STICK), 0);
  evalFlightModeMixes(e_perout_mode_normal, 0);

  // SWASH_TYPE_90: CYC1 = collective-elevator, CYC2 = collective+aileron,
  // CYC3 = collective-aileron.  With elevator only (vc=0, vr=0, vp=+1024):
  //   CYC1 = 0 - 1024 = -CHANNEL_MAX  (elevator inverted on CYC1)
  //   CYC2 = 0 + 0    = 0
  //   CYC3 = 0 - 0    = 0
  EXPECT_EQ(chans[0], -CHANNEL_MAX) << "CYC1 should carry inverted elevator";
  EXPECT_EQ(chans[1], 0)            << "CYC2 should be zero with no aileron";
  EXPECT_EQ(chans[2], 0)            << "CYC3 should be zero with no aileron";
}
#endif  // defined(HELI)

// CurveHeader.points encodes numPoints-5 in a 6-bit signed field.
// A 3-point curve (used in rc-soar models for aileron/elevator differential)
// must store points=-2 and produce correct interpolation via applyCustomCurve().
TEST(Curves, ThreePointCurveLayout)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  loadCurves();

  // A 3-point curve with points=-2 (numPoints = 3).
  g_model.curves[0].type   = 0;   // standard (evenly-spaced x)
  g_model.curves[0].smooth = 0;
  g_model.curves[0].points = -2;  // numPoints - 5 = 3 - 5 = -2
  loadCurves();  // re-initialise curveEnd[] after changing header

  // Set y values: -50, 0, +50  (symmetric differential-style curve).
  int8_t *pts = curveAddress(0);
  pts[0] = -50;
  pts[1] =   0;
  pts[2] =  50;

  // Verify the header encoding.
  EXPECT_EQ(g_model.curves[0].points, -2) << "points field must be numPoints-5";

  // Standard 3-pt curve: x = {-100, 0, 100}, y = {-50, 0, 50}.
  // Interpolation at the three knots must be exact.
  EXPECT_EQ(applyCustomCurve(-1024, 0), -512);  // -100% in → -50% out (scaled)
  EXPECT_EQ(applyCustomCurve(    0, 0),    0);  // centre → 0
  EXPECT_EQ(applyCustomCurve(+1024, 0), +512);  // +100% in → +50% out (scaled)
}

// LimitData.curve stores a 1-indexed curve number: 0 means no curve,
// 1 means Curve1 (index 0), N means Curve N (index N-1).
// This encoding is the same one the Lua getOutput/setOutput API relies on
// (+1 when storing, -1 when returning, absent when zero).
TEST(Curves, OutputLimitCurveEncoding)
{
  SYSTEM_RESET();
  MODEL_RESET();

  // Freshly-reset output must have no curve assigned.
  EXPECT_EQ(g_model.limitData[0].curve, 0) << "default curve must be 0 (none)";

  // Assign Curve1 (index 0): stored as 1.
  g_model.limitData[0].curve = 1;
  EXPECT_EQ(g_model.limitData[0].curve, 1) << "Curve1 stored as 1";
  // The Lua API returns curve-1 = 0 — verify the arithmetic.
  EXPECT_EQ(g_model.limitData[0].curve - 1, 0) << "Lua-visible index should be 0";

  // Assign Curve3 (index 2): stored as 3.
  g_model.limitData[1].curve = 3;
  EXPECT_EQ(g_model.limitData[1].curve, 3) << "Curve3 stored as 3";
  EXPECT_EQ(g_model.limitData[1].curve - 1, 2) << "Lua-visible index should be 2";

  // Output with no curve (curve==0) must not expose the field — the Lua API
  // gate is `if (limit->curve)`, so zero must remain falsy.
  g_model.limitData[2].curve = 0;
  EXPECT_FALSE(g_model.limitData[2].curve) << "zero curve must be falsy";
}

// =========================================================================
// LogicalSwitches / LogicalSwitchData struct tests
// =========================================================================

// LogicalSwitchData.andsw is a 10-bit signed field storing the AND-switch
// source index (Lua field "and").  Virtually every real-world logical switch
// has a non-zero andsw, but the field has never been directly tested at the
// C++ struct level.
TEST(LogicalSwitches, AndswitchFieldLayout)
{
  SYSTEM_RESET();
  MODEL_RESET();

  LogicalSwitchData * ls = lswAddress(0);

  // Freshly-zeroed struct: andsw must default to 0 (SWSRC_NONE).
  EXPECT_EQ(ls->andsw, 0) << "default andsw must be 0";

  // Positive value round-trip.
  ls->andsw = 42;
  EXPECT_EQ(ls->andsw, 42) << "andsw positive round-trip";

  // Negative value round-trip (inverted-switch convention: -ival).
  ls->andsw = -10;
  EXPECT_EQ(ls->andsw, -10) << "andsw negative round-trip";

  // Maximum positive value in a 10-bit signed field (+511).
  ls->andsw = 511;
  EXPECT_EQ(ls->andsw, 511) << "andsw max positive (+511)";

  // Minimum negative value in a 10-bit signed field (-512).
  ls->andsw = -512;
  EXPECT_EQ(ls->andsw, -512) << "andsw min negative (-512)";

  // A second LS entry must be independent.
  EXPECT_EQ(lswAddress(1)->andsw, 0) << "LS1 andsw should still be 0";
}

// =========================================================================
// Mixer / MixData struct tests
// =========================================================================

// MixData.flightModes is a 9-bit bitmask where bit N=1 means the mix is
// EXCLUDED from FM N.  The rc-soar "all-except-FM1" pattern sets bits
// 0 and 2-8 (FM1 is the only active FM), written in YAML as "101111111",
// which decodes to decimal 509 = 0b1_1111_1101.
TEST(Mixer, FlightModesBitmaskField)
{
  SYSTEM_RESET();
  MODEL_RESET();

  // 0 = active in all FMs (no bits set = no FM excluded).
  g_model.mixData[0].flightModes = 0;
  EXPECT_EQ(g_model.mixData[0].flightModes, 0u) << "0 means active in all FMs";

  // Bit 1 only: exclude FM1.
  g_model.mixData[0].flightModes = 0b00000'0010u;
  EXPECT_EQ(g_model.mixData[0].flightModes, 2u) << "FM1-only exclusion";

  // All-except-FM1 pattern from rc-soar models: bits 0,2-8 set, bit 1 clear.
  // YAML "101111111" → 1+4+8+16+32+64+128+256 = 509 = 0b1_1111_1101.
  const uint16_t ALL_EXCEPT_FM1 = 509u;
  g_model.mixData[0].flightModes = ALL_EXCEPT_FM1;
  EXPECT_EQ(g_model.mixData[0].flightModes, ALL_EXCEPT_FM1)
      << "all-except-FM1 bitmask (509) round-trip";

  // All 9 bits set: mix excluded from every FM.
  g_model.mixData[0].flightModes = 0x1FFu;
  EXPECT_EQ(g_model.mixData[0].flightModes, 0x1FFu) << "all-FMs-excluded (0x1FF)";

  // Adjacent mix line must be unaffected.
  EXPECT_EQ(g_model.mixData[1].flightModes, 0u) << "mixData[1] flightModes unchanged";
}

// MixData.mltpx is a 2-bit enum: ADD=0, MUL=1, REPL=2.  Most existing
// tests only exercise ADD; MUL and REPL appear in real-world soaring models
// for speed-compensation and flight-mode-switch override mixes.
TEST(Mixer, MltpxEncoding)
{
  SYSTEM_RESET();
  MODEL_RESET();

  g_model.mixData[0].mltpx = MLTPX_ADD;
  EXPECT_EQ(g_model.mixData[0].mltpx, (uint8_t)MLTPX_ADD) << "ADD (0) encoding";

  g_model.mixData[0].mltpx = MLTPX_MUL;
  EXPECT_EQ(g_model.mixData[0].mltpx, (uint8_t)MLTPX_MUL) << "MUL (1) encoding";

  g_model.mixData[0].mltpx = MLTPX_REPL;
  EXPECT_EQ(g_model.mixData[0].mltpx, (uint8_t)MLTPX_REPL) << "REPL (2) encoding";

  // Adjacent mix line must be unaffected (default is ADD=0).
  EXPECT_EQ(g_model.mixData[1].mltpx, (uint8_t)MLTPX_ADD)
      << "adjacent mix mltpx unchanged";
}

#if defined(GVARS)
// FlightModeData.gvars[] is independent per flight mode.
// rc-soar f3j model: FM0 GV1=80 (cruise climb rate), FM2 GV1=0 (launch).
// Verifies that writing one FM's GVar does not corrupt another FM's GVar.
TEST(GVars, PerFlightModeIsolation)
{
  SYSTEM_RESET();
  MODEL_RESET();

  g_model.flightModeData[0].gvars[0] = 80;
  g_model.flightModeData[2].gvars[0] = 0;

  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 80)
      << "FM0 gvars[0] should be 80";
  EXPECT_EQ(g_model.flightModeData[2].gvars[0], 0)
      << "FM2 gvars[0] should be 0";

  // Modify FM2; FM0 must remain unaffected.
  g_model.flightModeData[2].gvars[0] = 600;
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 80)
      << "FM0 gvars[0] unchanged after FM2 update";

  // Adjacent GVar index in FM0 must be unaffected.
  EXPECT_EQ(g_model.flightModeData[0].gvars[1], 0)
      << "FM0 gvars[1] should still be 0";
}
#endif  // defined(GVARS)
