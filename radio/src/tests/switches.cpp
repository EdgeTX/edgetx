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

#include "dataconstants.h"
#include "gtests.h"
#include "myeeprom.h"

#include "hal/adc_driver.h"
#include "hal/switch_driver.h"

void setLogicalSwitch(int index, uint16_t _func, int16_t _v1, int16_t _v2, int16_t _v3 = 0, uint8_t _delay = 0, uint8_t _duration = 0, int8_t _andsw = 0)
{
  g_model.logicalSw[index].func = _func;
  g_model.logicalSw[index].v1 = _v1;
  g_model.logicalSw[index].v2 = _v2;
  g_model.logicalSw[index].v3 = _v3;
  g_model.logicalSw[index].delay = _delay;
  g_model.logicalSw[index].duration = _duration;
  g_model.logicalSw[index].andsw = _andsw;
}

#define SWSRC_SW1 (SWSRC_FIRST_LOGICAL_SWITCH)
#define SWSRC_SW2 (SWSRC_FIRST_LOGICAL_SWITCH + 1)

TEST(getSwitch, OldTypeStickyCSW)
{
  RADIO_RESET();
  MODEL_RESET();
  MIXER_RESET();

  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;  // no 3-pos switch on this target
  int swPos = (sw * 3) + SWSRC_FIRST_SWITCH;

  setLogicalSwitch(0, LS_FUNC_AND, swPos, SWSRC_NONE);
  setLogicalSwitch(1, LS_FUNC_OR, SWSRC_SW1, SWSRC_SW2);

  simuSetSwitch(sw, 0);
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  // now trigger SA0, both switches should become true
  simuSetSwitch(sw, -1);
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SWSRC_SW1));
  EXPECT_TRUE(getSwitch(SWSRC_SW2));

  // now release SA0 and SW2 should stay true
  simuSetSwitch(sw, 0);
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_TRUE(getSwitch(SWSRC_SW2));

  // now reset logical switches
  logicalSwitchesReset();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));
}

// Tick logical switches for N cycles.
static void tickLogicalSwitches(int n)
{
  for (int i = 0; i < n; i++) {
    logicalSwitchesTimerTick();
    evalLogicalSwitches();
  }
}

// Sticky LS latches at startup when the set-condition is already satisfied.
// After logicalSwitchesReset(), lastValue starts at its CS_LAST_VALUE_INIT
// sentinel, so the first logicalSwitchesTimerTick() sees a 0->1 edge on the
// set-condition and latches immediately.
TEST(getSwitch, StickyLatchesAtStartupWhenSetConditionTrue)
{
  RADIO_RESET();
  MODEL_RESET();
  MIXER_RESET();

  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;  // no 3-pos switch on this target
  int swUp = (sw * 3) + SWSRC_FIRST_SWITCH;
  int swDn = swUp + 2;

  // Pre-set the switch to the "set" position BEFORE creating the LS
  simuSetSwitch(sw, -1);

  setLogicalSwitch(0, LS_FUNC_STICKY, swUp, swDn);

  // Simulate power-on
  logicalSwitchesReset();

  // After reset + tick, the set-condition is already true so sticky latches
  tickLogicalSwitches(10);
  EXPECT_TRUE(getSwitch(SWSRC_SW1))
      << "Sticky latches at startup when set-condition is already true";

  // Reset via the reset-condition (swDn)
  simuSetSwitch(sw, 1);
  tickLogicalSwitches(5);
  EXPECT_FALSE(getSwitch(SWSRC_SW1)) << "Reset condition should unlatch sticky";
}

// Sticky LS stays false at startup when the set-condition is NOT satisfied.
TEST(getSwitch, StickyFalseAtStartupWhenSetConditionFalse)
{
  RADIO_RESET();
  MODEL_RESET();
  MIXER_RESET();

  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;  // no 3-pos switch on this target
  int swUp = (sw * 3) + SWSRC_FIRST_SWITCH;
  int swDn = swUp + 2;

  // Switch in mid position - set-condition is false
  simuSetSwitch(sw, 0);

  setLogicalSwitch(0, LS_FUNC_STICKY, swUp, swDn);

  logicalSwitchesReset();
  tickLogicalSwitches(10);
  EXPECT_FALSE(getSwitch(SWSRC_SW1)) << "Sticky should not latch when condition is false";

  // Now trigger a rising edge -> should latch
  simuSetSwitch(sw, -1);
  tickLogicalSwitches(5);
  EXPECT_TRUE(getSwitch(SWSRC_SW1)) << "Sticky should latch on rising edge";
}

TEST(getSwitch, nullSW)
{
  MODEL_RESET();
  EXPECT_TRUE(getSwitch(0));
}

TEST(getSwitch, sensorSwitches)
{
  MODEL_RESET();
  telemetryItems[0].setFresh();
  telemetryItems[MAX_TELEMETRY_SENSORS - 1].setFresh();

  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    bool expected = !telemetryItems[i].isOld();
    EXPECT_EQ(expected, getSwitch(SWSRC_FIRST_SENSOR + i)) << "sensor " << i;
    EXPECT_EQ(!expected, getSwitch(-(SWSRC_FIRST_SENSOR + i))) << "sensor " << i;
  }
}

TEST(getSwitch, outOfRange)
{
  MODEL_RESET();

  // Out of range switches behave like a switch that doesn't exist
  for (swsrc_t sw : {SWSRC_LAST + 1, SWSRC_LAST + 100, 511}) {
    EXPECT_FALSE(getSwitch(sw)) << "switch " << sw;
    EXPECT_TRUE(getSwitch(-sw)) << "switch " << -sw;
  }
}


TEST(getSwitch, inputWithTrim)
{
  MODEL_RESET();
  setModelDefaults();
  MIXER_RESET();

  setLogicalSwitch(0, LS_FUNC_VPOS, MIXSRC_FIRST_INPUT, 0, 0);
  anaSetFiltered(0, 0);

  evalMixes(1);
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));

  setTrimValue(0, 0, 32);
  evalMixes(1);
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SWSRC_SW1));

  // a negative input weight reverses the trim along with the stick
  expoAddress(0)->weight = makeSourceNumVal(-100);
  evalMixes(1);
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
}

TEST(evalLogicalSwitches, playFile)
{
  SYSTEM_RESET();
  MODEL_RESET();
  setModelDefaults();
  MIXER_RESET();

  extern BitField<(MAX_LOGICAL_SWITCHES * 2/*on, off*/)> sdAvailableLogicalSwitchAudioFiles;
  char filename[AUDIO_FILENAME_MAXLEN+1];

#define MODELNAME "MODEL01"

  sdAvailableLogicalSwitchAudioFiles.setBit(INDEX_LOGICAL_SWITCH_AUDIO_FILE(0,AUDIO_EVENT_OFF));
  sdAvailableLogicalSwitchAudioFiles.setBit(INDEX_LOGICAL_SWITCH_AUDIO_FILE(0,AUDIO_EVENT_ON));
  sdAvailableLogicalSwitchAudioFiles.setBit(INDEX_LOGICAL_SWITCH_AUDIO_FILE(31,AUDIO_EVENT_OFF));
  sdAvailableLogicalSwitchAudioFiles.setBit(INDEX_LOGICAL_SWITCH_AUDIO_FILE(31,AUDIO_EVENT_ON));

  isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (0 << 16) + AUDIO_EVENT_OFF, filename);
  EXPECT_EQ(strcmp(filename, "/SOUNDS/en/" MODELNAME "/L1-off.wav"), 0);
  isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (0 << 16) + AUDIO_EVENT_ON, filename);
  EXPECT_EQ(strcmp(filename, "/SOUNDS/en/" MODELNAME "/L1-on.wav"), 0);
  isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (31 << 16) + AUDIO_EVENT_OFF, filename);
  EXPECT_EQ(strcmp(filename, "/SOUNDS/en/" MODELNAME "/L32-off.wav"), 0);
  isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (31 << 16) + AUDIO_EVENT_ON, filename);
  EXPECT_EQ(strcmp(filename, "/SOUNDS/en/" MODELNAME "/L32-on.wav"), 0);

  EXPECT_TRUE(isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (31 << 16) + AUDIO_EVENT_ON, filename));
  EXPECT_FALSE(isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (32 << 16) + AUDIO_EVENT_ON, filename));

#undef MODELNAME
}

TEST(getSwitch, edgeInstant)
{
  int sw1 = findHwSwitch(SWITCH_3POS);
  int sw2 = findHwSwitch(SWITCH_3POS, sw1);
  if (sw1 < 0 || sw2 < 0) return;  // needs two distinct 3-pos switches
  int sw1Pos = (sw1 * 3) + SWSRC_FIRST_SWITCH;
  int sw2Pos = (sw2 * 3) + SWSRC_FIRST_SWITCH;
  
  MODEL_RESET();
  MIXER_RESET();
  // LS1 setup: EDGE SD down (0:instant)
  // LS2 setup: (EDGE SD down (0:instant)) AND SA down
  setLogicalSwitch(0, LS_FUNC_EDGE, sw2Pos + 2, -129, -1);
  setLogicalSwitch(1, LS_FUNC_EDGE, sw2Pos + 2, -129, -1, 0, 0, sw1Pos + 2);

  simuSetSwitch(sw1, -1);  //SA up
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  // now trigger SD donw, LS1 should become true
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  // now release SD and LS1 should become false
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  // now reset logical switches
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  // second part with SA down

  simuSetSwitch(sw1, 1);   //SA down
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  // now trigger SD down, LS1 & LS2 should become true
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SWSRC_SW1));
  EXPECT_TRUE(getSwitch(SWSRC_SW2));

  // now release SA and LS1 & LS2 should stay false
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  // now reset logical switches
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  // now bug #2939
  // SD is kept down and SA is toggled
  simuSetSwitch(sw1, -1);   //SA up
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  simuSetSwitch(sw1, 1);   //SA down
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  simuSetSwitch(sw1, -1);   //SA up
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  //test what happens when EDGE condition is true and
  //logical switches are reset - the switch should fire again

  simuSetSwitch(sw1, 1);   //SA down
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));  //switch will not trigger, because SF was already up
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SWSRC_SW1));
  EXPECT_TRUE(getSwitch(SWSRC_SW2));

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));
}

TEST(getSwitch, edgeRelease)
{
  int sw1 = findHwSwitch(SWITCH_3POS);
  int sw2 = findHwSwitch(SWITCH_3POS, sw1);
  if (sw1 < 0 || sw2 < 0) return;  // needs two distinct 3-pos switches
  int sw1Pos = (sw1 * 3) + SWSRC_FIRST_SWITCH;
  int sw2Pos = (sw2 * 3) + SWSRC_FIRST_SWITCH;
  
  MODEL_RESET();
  MIXER_RESET();
  // test for issue #2728
  // LS1 setup: EDGE SDup  (0:release)
  // LS2 setup: (EDGE SDup  (0:release)) AND SAup
  setLogicalSwitch(0, LS_FUNC_EDGE, sw2Pos + 2, -129, 0);
  setLogicalSwitch(1, LS_FUNC_EDGE, sw2Pos + 2, -129, 0, 0, 0, sw1Pos + 2 );

  simuSetSwitch(sw1, -1);   //SA down
  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  simuSetSwitch(sw2, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));


  // second part with SAup
  simuSetSwitch(sw1, 1);   //SA up
  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  simuSetSwitch(sw2, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SWSRC_SW1));
  EXPECT_TRUE(getSwitch(SWSRC_SW2));

  // with switches reset both should remain false
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SWSRC_SW1));
  EXPECT_FALSE(getSwitch(SWSRC_SW2));

}

uint8_t boardGetMaxSwitches();

TEST(FlexSwitches, switchGetPosition)
{
  if (adcGetMaxInputs(ADC_INPUT_FLEX) == 0) return;
  if (MAX_FLEX_SWITCHES == 0) return;
  switchInit();

  auto sw_idx = boardGetMaxSwitches();
  auto sw_name = switchGetDefaultName(sw_idx);
  EXPECT_STREQ("FL1", sw_name);
  EXPECT_FALSE(switchIsFlexValid(sw_idx));
  
  // Configure 1st FLEX input as switch
  g_eeGeneral.potsConfig = FLEX_SWITCH;
  switchConfigFlex(sw_idx, 0);
  EXPECT_TRUE(switchIsFlexValid(sw_idx));

  auto offset = adcGetInputOffset(ADC_INPUT_FLEX);
  anaSetFiltered(offset, -1024);
  EXPECT_EQ(SWITCH_HW_UP, switchGetPosition(sw_idx));

  anaSetFiltered(offset, 0);
  EXPECT_EQ(SWITCH_HW_MID, switchGetPosition(sw_idx));

  anaSetFiltered(offset, +1024);
  EXPECT_EQ(SWITCH_HW_DOWN, switchGetPosition(sw_idx));
}

TEST(FlexSwitches, getValue)
{
  if (adcGetMaxInputs(ADC_INPUT_FLEX) == 0) return;
  if (MAX_FLEX_SWITCHES == 0) return;
  switchInit();

  // Configure 1st FLEX input as switch
  g_eeGeneral.potsConfig = FLEX_SWITCH;
  auto sw_idx = boardGetMaxSwitches();
  switchConfigFlex(sw_idx, 0);

  g_eeGeneral.switchSetType(sw_idx, SWITCH_3POS);
  EXPECT_EQ(SWITCH_3POS, g_model.getSwitchType(sw_idx));

  auto offset = adcGetInputOffset(ADC_INPUT_FLEX);
  anaSetFiltered(offset, -1024);
  EXPECT_EQ(-1024, getValue(MIXSRC_FIRST_SWITCH + sw_idx));

  anaSetFiltered(offset, 0);
  EXPECT_EQ(0, getValue(MIXSRC_FIRST_SWITCH + sw_idx));

  anaSetFiltered(offset, +1024);
  EXPECT_EQ(+1024, getValue(MIXSRC_FIRST_SWITCH + sw_idx));

  g_eeGeneral.switchSetType(sw_idx, SWITCH_2POS);
  EXPECT_EQ(SWITCH_2POS, g_model.getSwitchType(sw_idx));

  anaSetFiltered(offset, -1024);
  EXPECT_EQ(-1024, getValue(MIXSRC_FIRST_SWITCH + sw_idx));

  anaSetFiltered(offset, 0);
  EXPECT_EQ(+1024, getValue(MIXSRC_FIRST_SWITCH + sw_idx));

  anaSetFiltered(offset, +1024);
  EXPECT_EQ(+1024, getValue(MIXSRC_FIRST_SWITCH + sw_idx));
}

TEST(FlexSwitches, getSwitch)
{
  if (adcGetMaxInputs(ADC_INPUT_FLEX) == 0) return;
  if (MAX_FLEX_SWITCHES == 0) return;
  switchInit();

  // Configure 1st FLEX input as switch
  g_eeGeneral.potsConfig = FLEX_SWITCH;
  auto sw_idx = boardGetMaxSwitches();
  switchConfigFlex(sw_idx, 0);

  g_eeGeneral.switchSetType(sw_idx, SWITCH_3POS);
  EXPECT_EQ(SWITCH_3POS, g_model.getSwitchType(sw_idx));

  auto offset = adcGetInputOffset(ADC_INPUT_FLEX);
  anaSetFiltered(offset, -1024);
  EXPECT_TRUE(getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3));
  EXPECT_FALSE(getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 1));
  EXPECT_FALSE(getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 2));

  anaSetFiltered(offset, 0);
  EXPECT_FALSE(getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3));
  EXPECT_TRUE(getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 1));
  EXPECT_FALSE(getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 2));

  anaSetFiltered(offset, +1024);
  EXPECT_FALSE(getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3));
  EXPECT_FALSE(getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 1));
  EXPECT_TRUE(getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 2));
}

#if defined(FUNCTION_SWITCHES)
TEST(FunctionSwitches, holdIsNotRepeatedToggle)
{
  MODEL_RESET();
  setModelDefaults();
  switchInit();

  for (uint8_t i = 0; i < switchGetMaxSwitches(); i++) {
    if (!switchIsCustomSwitch(i)) continue;
    // Toggle type with no group, so a phantom "switch moved" event
    // flips the logical state and is observable
    g_model.cfsSetType(i, SWITCH_TOGGLE);
    g_model.cfsSetGroup(i, 0);
    simuSetSwitch(i, -1);
  }

  setFSStartupPosition();
  evalFunctionSwitches();  // sync previous state with all switches released

  for (uint8_t i = 0; i < switchGetMaxSwitches(); i++) {
    if (!switchIsCustomSwitch(i)) continue;

    bool released = g_model.cfsState(i);
    simuSetSwitch(i, 1);
    evalFunctionSwitches();
    bool pressed = g_model.cfsState(i);
    EXPECT_NE(released, pressed) << "press did not toggle switch index " << (int)i;

    // holding the switch must not generate further toggles
    evalFunctionSwitches();
    EXPECT_EQ(pressed, g_model.cfsState(i))
        << "hold re-toggled switch index " << (int)i;
    evalFunctionSwitches();
    EXPECT_EQ(pressed, g_model.cfsState(i))
        << "hold re-toggled switch index " << (int)i;

    simuSetSwitch(i, -1);
    evalFunctionSwitches();
  }
}
#endif

// ---------------------------------------------------------------------------
// Single logical switch context (lswCtx[]), shared by all flight modes.
// Timer, sticky, edge, delay/duration and delta state no longer depend on
// the flight mode; only a fading-out flight mode sees the LS states frozen
// at the transition.
// ---------------------------------------------------------------------------

class LswTest : public EdgeTxTest {};

// Mixer timing: LS evaluated every 10 ms, LS timers ticked every 100 ms.
static uint8_t s_test_100ms_cnt = 0;
static void mixerTickRealistic(int ticks10ms)
{
  for (int i = 0; i < ticks10ms; i++) {
    evalLogicalSwitches();
    if (++s_test_100ms_cnt >= 10) {
      s_test_100ms_cnt = 0;
      logicalSwitchesTimerTick();
    }
  }
}

TEST_F(LswTest, ResetInitsSingleContext)
{
  lswSetState(0, 1, 42, 999);

  logicalSwitchesReset();

  EXPECT_FALSE(lswGetState(0));
  EXPECT_EQ(-32768, lswGetLastValue(0));  // CS_LAST_VALUE_INIT
}

TEST_F(LswTest, TimerOscillates)
{
  setLogicalSwitch(0, LS_FUNC_TIMER, -109, -109);  // shortest periods
  logicalSwitchesReset();

  bool sawTrue = false, sawFalse = false;
  for (int tick = 0; tick < 50; tick++) {
    evalLogicalSwitches();
    logicalSwitchesTimerTick();
    if (lswGetState(0))
      sawTrue = true;
    else
      sawFalse = true;
  }
  EXPECT_TRUE(sawTrue) << "Timer should produce true states";
  EXPECT_TRUE(sawFalse) << "Timer should produce false states";
}

TEST_F(LswTest, StickyToggle)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;  // no 3-pos switch on this target
  int swUp = (sw * 3) + SWSRC_FIRST_SWITCH;
  int swDn = swUp + 2;

  setLogicalSwitch(0, LS_FUNC_STICKY, swUp, swDn);
  logicalSwitchesReset();

  simuSetSwitch(sw, 0);
  tickLogicalSwitches(5);
  EXPECT_FALSE(lswGetState(0));

  simuSetSwitch(sw, -1);
  tickLogicalSwitches(5);
  EXPECT_TRUE(lswGetState(0));

  simuSetSwitch(sw, 0);
  tickLogicalSwitches(3);
  simuSetSwitch(sw, 1);
  tickLogicalSwitches(5);
  EXPECT_FALSE(lswGetState(0));
}

// A delay keeps running through flight mode changes (with per-FM contexts,
// a change copied the old FM's context and the timers of all FMs ticked
// independently).
TEST_F(LswTest, DelayContinuesThroughFMSwitch)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;
  int swUp = (sw * 3) + SWSRC_FIRST_SWITCH;

  setLogicalSwitch(0, LS_FUNC_AND, swUp, SWSRC_NONE, 0, 10 /* 1.0 s delay */);
  logicalSwitchesReset();
  s_test_100ms_cnt = 0;

  simuSetSwitch(sw, -1);

  mixerCurrentFlightMode = mixerActiveFlightMode = 0;
  mixerTickRealistic(50);
  EXPECT_FALSE(lswGetState(0)) << "Delay should still be running";

  mixerCurrentFlightMode = mixerActiveFlightMode = 2;
  mixerTickRealistic(30);
  EXPECT_FALSE(lswGetState(0)) << "Delay should still be running after FM switch";

  mixerCurrentFlightMode = mixerActiveFlightMode = 0;
  mixerTickRealistic(30);
  EXPECT_TRUE(lswGetState(0)) << "Delay should have expired after 1.1 s total";
}

TEST_F(LswTest, DurationContinuesThroughFMSwitch)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;
  int swUp = (sw * 3) + SWSRC_FIRST_SWITCH;

  setLogicalSwitch(0, LS_FUNC_AND, swUp, SWSRC_NONE, 0, 0, 10 /* 1.0 s */);
  logicalSwitchesReset();
  s_test_100ms_cnt = 0;

  simuSetSwitch(sw, -1);

  mixerCurrentFlightMode = mixerActiveFlightMode = 0;
  mixerTickRealistic(50);
  EXPECT_TRUE(lswGetState(0)) << "Should be active during duration";

  mixerCurrentFlightMode = mixerActiveFlightMode = 1;
  mixerTickRealistic(30);
  EXPECT_TRUE(lswGetState(0)) << "Duration should continue after FM switch";

  mixerTickRealistic(30);
  EXPECT_FALSE(lswGetState(0)) << "Duration should have expired after 1.1 s";
}

TEST_F(LswTest, GetSwitchReadsSingleContext)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;
  int swUp = (sw * 3) + SWSRC_FIRST_SWITCH;

  setLogicalSwitch(0, LS_FUNC_AND, swUp, SWSRC_NONE);
  logicalSwitchesReset();

  simuSetSwitch(sw, -1);
  mixerCurrentFlightMode = mixerActiveFlightMode = 0;
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SWSRC_SW1));

  // same state from any FM while no fade is active
  mixerCurrentFlightMode = mixerActiveFlightMode = 1;
  EXPECT_TRUE(getSwitch(SWSRC_SW1));
  mixerCurrentFlightMode = mixerActiveFlightMode = 5;
  EXPECT_TRUE(getSwitch(SWSRC_SW1));
}

TEST_F(LswTest, DeltaTracksLastValue)
{
  setLogicalSwitch(0, LS_FUNC_DIFFEGREATER, MIXSRC_FIRST_STICK, 10);
  logicalSwitchesReset();

  uint8_t stickIdx = inputMappingConvertMode(0);
  calibratedAnalogs[stickIdx] = 0;

  evalLogicalSwitches();
  EXPECT_EQ(0, lswGetLastValue(0));

  calibratedAnalogs[stickIdx] = 512;
  evalLogicalSwitches();
  EXPECT_EQ(512, lswGetLastValue(0));

  mixerCurrentFlightMode = mixerActiveFlightMode = 3;
  calibratedAnalogs[stickIdx] = 256;
  evalLogicalSwitches();
  EXPECT_EQ(256, lswGetLastValue(0));
}

TEST_F(LswTest, ComparisonNoAccumulatedState)
{
  setLogicalSwitch(0, LS_FUNC_VPOS, MIXSRC_FIRST_STICK, 50);
  logicalSwitchesReset();

  uint8_t stickIdx = inputMappingConvertMode(0);

  calibratedAnalogs[stickIdx] = 1024;
  mixerCurrentFlightMode = mixerActiveFlightMode = 0;
  evalLogicalSwitches();
  EXPECT_TRUE(lswGetState(0));

  mixerCurrentFlightMode = mixerActiveFlightMode = 3;
  evalLogicalSwitches();
  EXPECT_TRUE(lswGetState(0));

  calibratedAnalogs[stickIdx] = 0;
  evalLogicalSwitches();
  EXPECT_FALSE(lswGetState(0));
}

// During a fade, the fading-out FM's mixes see the LS states frozen at the
// transition (as its stale per-FM context did before), while the active FM
// sees the live state.
TEST(getSwitch, fadingOutFmSeesFrozenLsState)
{
  SYSTEM_RESET();
  MODEL_RESET();
  setModelDefaults();
  MIXER_RESET();

  int sw = findHwSwitch();
  ASSERT_GE(sw, 0) << "No usable hardware switch found";
  // "down" for all types: getSwitch handles 2POS/TOGGLE aliasing
  int swActive = (sw * 3) + 2 + SWSRC_FIRST_SWITCH;

  // LS1: stick0 > 0
  setLogicalSwitch(0, LS_FUNC_VPOS, MIXSRC_FIRST_STICK, 0);

  // FM1 selected by the switch, 1 s fade
  g_model.flightModeData[1].swtch = swActive;
  g_model.flightModeData[1].fadeIn = 10;
  g_model.flightModeData[0].fadeOut = 10;

  // FM0, stick positive: LS1 true
  simuSetSwitch(sw, -1);
  anaSetFiltered(0, 512);
  evalMixes(1);
  evalMixes(1);
  EXPECT_TRUE(getSwitch(SWSRC_SW1)) << "LS1 should be TRUE with positive stick";

  // FM1: the fade starts, LS1 still true at the transition
  simuSetSwitch(sw, 1);
  evalMixes(1);

  // stick negative: LS1 becomes false for the active FM
  anaSetFiltered(0, -512);
  for (int i = 0; i < 5; i++) evalMixes(1);

  uint8_t savedFM = mixerCurrentFlightMode;
  mixerCurrentFlightMode = 1;
  EXPECT_FALSE(getSwitch(SWSRC_SW1)) << "active FM sees the live LS state";
  mixerCurrentFlightMode = 0;
  EXPECT_TRUE(getSwitch(SWSRC_SW1)) << "fading-out FM sees the frozen LS state";
  mixerCurrentFlightMode = savedFM;
}
