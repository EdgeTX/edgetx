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
#include "mixes.h"
#include "expos.h"
#include "curves.h"
#include "myeeprom.h"

#include "hal/adc_driver.h"
#include "hal/switch_driver.h"

static const SwitchRef SW1_REF = SwitchRef_(SWITCH_TYPE_LOGICAL, 0);
static const SwitchRef SW2_REF = SwitchRef_(SWITCH_TYPE_LOGICAL, 1);
static const SwitchRef SWTCH_NONE = {};

#if defined(PCBTARANIS)
TEST(getSwitch, OldTypeStickyCSW)
{
  RADIO_RESET();
  MODEL_RESET();
  MIXER_RESET();

  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  auto swRef = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3));

  // LS0: AND(sw_up, NONE)
  lswAllocAt(0)->func = LS_FUNC_AND;
  lswAddress(0)->v1.swtch = swRef;
  lswAddress(0)->v2.swtch = SWTCH_NONE;
  lswAddress(0)->andsw = SWTCH_NONE;

  // LS1: OR(SW1, SW2)
  lswAllocAt(1)->func = LS_FUNC_OR;
  lswAddress(1)->v1.swtch = SW1_REF;
  lswAddress(1)->v2.swtch = SW2_REF;
  lswAddress(1)->andsw = SWTCH_NONE;

  simuSetSwitch(sw, 0);
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  // now trigger SA0, both switches should become true
  simuSetSwitch(sw, -1);
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SW1_REF));
  EXPECT_TRUE(getSwitch(SW2_REF));

  // now release SA0 and SW2 should stay true
  simuSetSwitch(sw, 0);
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_TRUE(getSwitch(SW2_REF));

  // now reset logical switches
  logicalSwitchesReset();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));
}
#endif

TEST(getSwitch, nullSW)
{
  MODEL_RESET();
  EXPECT_TRUE(getSwitch(SwitchRef{}));
}

#if defined(PCBTARANIS)
TEST(getSwitch, inputWithTrim)
{
  MODEL_RESET();
  setModelDefaults();
  MIXER_RESET();

  // LS0: VPOS(INPUT0, 0)
  lswAllocAt(0)->func = LS_FUNC_VPOS;
  lswAddress(0)->v1.source = SourceRef_(SOURCE_TYPE_INPUT, 0);
  lswAddress(0)->v2.value = 0;
  lswAddress(0)->v3 = 0;
  lswAddress(0)->andsw = SWTCH_NONE;
  anaSetFiltered(0, 0);

  evalMixes(1);
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));

  setTrimValue(0, 0, 32);
  evalMixes(1);
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SW1_REF));
}
#endif

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
  int sw1, sw2;
  for (sw1 = 0; sw1 < switchGetMaxAllSwitches(); sw1 += 1)
    if (g_model.getSwitchType(sw1) == SWITCH_3POS)
      break;
  for (sw2 = sw1 + 1; sw2 < switchGetMaxAllSwitches(); sw2 += 1)
    if (g_model.getSwitchType(sw2) == SWITCH_3POS)
      break;
  auto sw1Down = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw1 * 3 + 2));
  auto sw2Down = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw2 * 3 + 2));

  MODEL_RESET();
  MIXER_RESET();
  // LS1 setup: EDGE SD down (0:instant)
  // LS2 setup: (EDGE SD down (0:instant)) AND SA down
  lswAllocAt(0)->func = LS_FUNC_EDGE;
  lswAddress(0)->v1.swtch = sw2Down;
  lswAddress(0)->v2.value = -129;
  lswAddress(0)->v3 = -1;
  lswAddress(0)->andsw = SWTCH_NONE;

  lswAllocAt(1)->func = LS_FUNC_EDGE;
  lswAddress(1)->v1.swtch = sw2Down;
  lswAddress(1)->v2.value = -129;
  lswAddress(1)->v3 = -1;
  lswAddress(1)->andsw = sw1Down;

  simuSetSwitch(sw1, -1);  //SA up
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  // now trigger SD donw, LS1 should become true
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  // now release SD and LS1 should become false
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  // now reset logical switches
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  // second part with SA down

  simuSetSwitch(sw1, 1);   //SA down
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  // now trigger SD down, LS1 & LS2 should become true
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SW1_REF));
  EXPECT_TRUE(getSwitch(SW2_REF));

  // now release SA and LS1 & LS2 should stay false
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  // now reset logical switches
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  // now bug #2939
  // SD is kept down and SA is toggled
  simuSetSwitch(sw1, -1);   //SA up
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  simuSetSwitch(sw1, 1);   //SA down
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  simuSetSwitch(sw1, -1);   //SA up
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  //test what happens when EDGE condition is true and
  //logical switches are reset - the switch should fire again

  simuSetSwitch(sw1, 1);   //SA down
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));  //switch will not trigger, because SF was already up
  EXPECT_FALSE(getSwitch(SW2_REF));

  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SW1_REF));
  EXPECT_TRUE(getSwitch(SW2_REF));

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));
}

TEST(getSwitch, edgeRelease)
{
  int sw1, sw2;
  for (sw1 = 0; sw1 < switchGetMaxAllSwitches(); sw1 += 1)
    if (g_model.getSwitchType(sw1) == SWITCH_3POS)
      break;
  for (sw2 = sw1 + 1; sw2 < switchGetMaxAllSwitches(); sw2 += 1)
    if (g_model.getSwitchType(sw2) == SWITCH_3POS)
      break;
  auto sw1Down = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw1 * 3 + 2));
  auto sw2Down = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw2 * 3 + 2));

  MODEL_RESET();
  MIXER_RESET();
  // test for issue #2728
  // LS1 setup: EDGE SDup  (0:release)
  // LS2 setup: (EDGE SDup  (0:release)) AND SAup
  lswAllocAt(0)->func = LS_FUNC_EDGE;
  lswAddress(0)->v1.swtch = sw2Down;
  lswAddress(0)->v2.value = -129;
  lswAddress(0)->v3 = 0;
  lswAddress(0)->andsw = SWTCH_NONE;

  lswAllocAt(1)->func = LS_FUNC_EDGE;
  lswAddress(1)->v1.swtch = sw2Down;
  lswAddress(1)->v2.value = -129;
  lswAddress(1)->v3 = 0;
  lswAddress(1)->andsw = sw1Down;

  simuSetSwitch(sw1, -1);   //SA down
  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  simuSetSwitch(sw2, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));


  // second part with SAup
  simuSetSwitch(sw1, 1);   //SA up
  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  simuSetSwitch(sw2, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_TRUE(getSwitch(SW1_REF));
  EXPECT_TRUE(getSwitch(SW2_REF));

  // with switches reset both should remain false
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_FALSE(getSwitch(SW1_REF));
  EXPECT_FALSE(getSwitch(SW2_REF));

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
  EXPECT_EQ(-1024, getValue(SourceRef_(SOURCE_TYPE_SWITCH, sw_idx)));

  anaSetFiltered(offset, 0);
  EXPECT_EQ(0, getValue(SourceRef_(SOURCE_TYPE_SWITCH, sw_idx)));

  anaSetFiltered(offset, +1024);
  EXPECT_EQ(+1024, getValue(SourceRef_(SOURCE_TYPE_SWITCH, sw_idx)));

  g_eeGeneral.switchSetType(sw_idx, SWITCH_2POS);
  EXPECT_EQ(SWITCH_2POS, g_model.getSwitchType(sw_idx));

  anaSetFiltered(offset, -1024);
  EXPECT_EQ(-1024, getValue(SourceRef_(SOURCE_TYPE_SWITCH, sw_idx)));

  anaSetFiltered(offset, 0);
  EXPECT_EQ(+1024, getValue(SourceRef_(SOURCE_TYPE_SWITCH, sw_idx)));

  anaSetFiltered(offset, +1024);
  EXPECT_EQ(+1024, getValue(SourceRef_(SOURCE_TYPE_SWITCH, sw_idx)));
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
  auto swUp  = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw_idx * 3));
  auto swMid = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw_idx * 3 + 1));
  auto swDn  = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw_idx * 3 + 2));

  anaSetFiltered(offset, -1024);
  EXPECT_TRUE(getSwitch(swUp));
  EXPECT_FALSE(getSwitch(swMid));
  EXPECT_FALSE(getSwitch(swDn));

  anaSetFiltered(offset, 0);
  EXPECT_FALSE(getSwitch(swUp));
  EXPECT_TRUE(getSwitch(swMid));
  EXPECT_FALSE(getSwitch(swDn));

  anaSetFiltered(offset, +1024);
  EXPECT_FALSE(getSwitch(swUp));
  EXPECT_FALSE(getSwitch(swMid));
  EXPECT_TRUE(getSwitch(swDn));
}

// ---------------------------------------------------------------------------
// Tests to evidence whether per-FM logical switch state actually diverges.
// The runtime maintains lswFm[MAX_FLIGHT_MODES] — one LogicalSwitchContext
// per FM per LS. logicalSwitchesTimerTick() updates timer/sticky/edge state
// for ALL FMs every tick. These tests check whether the per-FM copies ever
// diverge from each other.
// ---------------------------------------------------------------------------

class LswPerFmTest : public EdgeTxTest {};

// Helper: tick logical switches for N cycles (eval only, no timer tick)
static void tickLogicalSwitches(int n)
{
  for (int i = 0; i < n; i++) {
    evalLogicalSwitches();
    logicalSwitchesTimerTick();
  }
}

// Helper: simulate realistic mixer timing for N 10ms ticks.
// evalLogicalSwitches runs every 10ms, logicalSwitchesTimerTick every 100ms.
static uint8_t s_test_100ms_cnt = 0;
static void mixerTickRealistic(int ticks10ms)
{
  for (int i = 0; i < ticks10ms; i++) {
    evalLogicalSwitches(true);
    s_test_100ms_cnt++;
    if (s_test_100ms_cnt >= 10) {
      s_test_100ms_cnt = 0;
      logicalSwitchesTimerTick();
    }
  }
}

// Test: Timer LS state is identical across all FMs when evaluated from reset.
// All FMs are ticked in parallel by logicalSwitchesTimerTick, so their
// timer counters should always be in lockstep.
TEST_F(LswPerFmTest, TimerStateIdenticalAcrossFMs)
{
  // LS0: TIMER with v1=0 (0.5s off), v2=0 (0.5s on)
  lswAllocAt(0)->func = LS_FUNC_TIMER;
  lswAddress(0)->v1.value = 0;  // 0.5s
  lswAddress(0)->v2.value = 0;  // 0.5s
  lswAddress(0)->andsw = SWTCH_NONE;

  logicalSwitchesReset();

  // Run 100 ticks
  for (int tick = 0; tick < 100; tick++) {
    evalLogicalSwitches();
    logicalSwitchesTimerTick();

    // Check that all FMs have the same lastValue for LS0
    int16_t fm0val = lswFm[0].lsw[0].lastValue;
    for (int fm = 1; fm < MAX_FLIGHT_MODES; fm++) {
      EXPECT_EQ(fm0val, lswFm[fm].lsw[0].lastValue)
          << "tick=" << tick << " fm=" << fm;
    }
  }
}

// Test: Sticky LS state is identical across all FMs.
// logicalSwitchesTimerTick processes sticky for all FMs with the same
// getSwitch() inputs, so they should track identically.
TEST_F(LswPerFmTest, StickyStateIdenticalAcrossFMs)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw++)
    if (g_model.getSwitchType(sw) == SWITCH_3POS) break;
  auto swUp = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3));
  auto swDn = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3 + 2));

  // LS0: STICKY(sw_up, sw_down)
  lswAllocAt(0)->func = LS_FUNC_STICKY;
  lswAddress(0)->v1.swtch = swUp;
  lswAddress(0)->v2.swtch = swDn;
  lswAddress(0)->andsw = SWTCH_NONE;

  logicalSwitchesReset();

  // Start with switch mid (neither trigger)
  simuSetSwitch(sw, 0);
  tickLogicalSwitches(5);

  // Trigger sticky ON
  simuSetSwitch(sw, -1);
  tickLogicalSwitches(5);

  // All FMs should agree
  for (int fm = 0; fm < MAX_FLIGHT_MODES; fm++) {
    EXPECT_EQ(lswFm[0].lsw[0].lastValue, lswFm[fm].lsw[0].lastValue)
        << "after ON, fm=" << fm;
  }

  // Switch to mid, then trigger OFF
  simuSetSwitch(sw, 0);
  tickLogicalSwitches(3);
  simuSetSwitch(sw, 1);
  tickLogicalSwitches(5);

  // All FMs should still agree
  for (int fm = 0; fm < MAX_FLIGHT_MODES; fm++) {
    EXPECT_EQ(lswFm[0].lsw[0].lastValue, lswFm[fm].lsw[0].lastValue)
        << "after OFF, fm=" << fm;
  }
}

// Test: Timer LS state remains identical even when the active FM changes.
// Switch FM mid-evaluation and verify the background FMs stay in sync.
TEST_F(LswPerFmTest, TimerStateSurvivesFMSwitch)
{
  lswAllocAt(0)->func = LS_FUNC_TIMER;
  lswAddress(0)->v1.value = 0;  // 0.5s
  lswAddress(0)->v2.value = 0;  // 0.5s
  lswAddress(0)->andsw = SWTCH_NONE;

  logicalSwitchesReset();

  // Run 30 ticks in FM0
  mixerCurrentFlightMode = 0;
  tickLogicalSwitches(30);

  // Switch to FM2
  mixerCurrentFlightMode = 2;
  tickLogicalSwitches(30);

  // Switch back to FM0
  mixerCurrentFlightMode = 0;
  tickLogicalSwitches(10);

  // All FMs should still be in lockstep (timer ticked for all FMs throughout)
  int16_t fm0val = lswFm[0].lsw[0].lastValue;
  for (int fm = 1; fm < MAX_FLIGHT_MODES; fm++) {
    EXPECT_EQ(fm0val, lswFm[fm].lsw[0].lastValue) << "fm=" << fm;
  }
}

// Test: The only observable effect of per-FM state is the `state` field
// written by evalLogicalSwitches (which only writes to mixerCurrentFlightMode).
// Timer lastValue is updated for all FMs, but `state` is only set for the
// current FM. This test verifies that `state` diverges across FMs.
TEST_F(LswPerFmTest, EvalStateOnlyWrittenToCurrentFM)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw++)
    if (g_model.getSwitchType(sw) == SWITCH_3POS) break;
  auto swUp = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3));

  // LS0: AND(sw_up, NONE) — simple boolean
  lswAllocAt(0)->func = LS_FUNC_AND;
  lswAddress(0)->v1.swtch = swUp;
  lswAddress(0)->v2.swtch = SWTCH_NONE;
  lswAddress(0)->andsw = SWTCH_NONE;

  logicalSwitchesReset();

  // Activate the switch
  simuSetSwitch(sw, -1);

  // Eval in FM0 only
  mixerCurrentFlightMode = 0;
  evalLogicalSwitches();

  // FM0 state should be true, FM1 state should be false (never evaluated)
  EXPECT_TRUE(lswFm[0].lsw[0].state);
  EXPECT_FALSE(lswFm[1].lsw[0].state);

  // Now eval in FM1
  mixerCurrentFlightMode = 1;
  evalLogicalSwitches();

  // Now both should be true
  EXPECT_TRUE(lswFm[0].lsw[0].state);
  EXPECT_TRUE(lswFm[1].lsw[0].state);
}

// Test: Delay/duration processing runs inside getLogicalSwitch which only
// executes for mixerCurrentFlightMode. The timerState and timer fields
// should diverge across FMs when delay or duration is configured.
TEST_F(LswPerFmTest, DelayDurationDivergesAcrossFMs)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw++)
    if (g_model.getSwitchType(sw) == SWITCH_3POS) break;
  auto swUp = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3));

  // LS0: AND(sw_up, NONE) with delay=5 (0.5s) and duration=0
  lswAllocAt(0)->func = LS_FUNC_AND;
  lswAddress(0)->v1.swtch = swUp;
  lswAddress(0)->v2.swtch = SWTCH_NONE;
  lswAddress(0)->andsw = SWTCH_NONE;
  lswAddress(0)->delay = 5;     // 0.5s delay
  lswAddress(0)->duration = 0;  // no duration limit

  logicalSwitchesReset();
  s_test_100ms_cnt = 0;

  // Activate the switch
  simuSetSwitch(sw, -1);

  // Run in FM0 for 1 second (100 × 10ms ticks)
  mixerCurrentFlightMode = 0;
  mixerTickRealistic(100);

  // FM0 should have progressed through delay and be active
  // FM1 was never evaluated, so its timerState should still be SWITCH_START(0)
  EXPECT_NE(lswFm[0].lsw[0].timerState, lswFm[1].lsw[0].timerState)
      << "timerState should diverge: FM0 evaluated, FM1 never evaluated";
}

// Test: Timer LS with realistic 10ms eval / 100ms tick timing.
// Verify timer counters still stay in lockstep across FMs even with
// the 10:1 ratio between eval and timer tick.
TEST_F(LswPerFmTest, TimerRealisticTiming)
{
  // LS0: TIMER with v1=1 (1.0s off), v2=1 (1.0s on)
  lswAllocAt(0)->func = LS_FUNC_TIMER;
  lswAddress(0)->v1.value = 1;  // 1.0s
  lswAddress(0)->v2.value = 1;  // 1.0s
  lswAddress(0)->andsw = SWTCH_NONE;

  logicalSwitchesReset();
  s_test_100ms_cnt = 0;

  // Run 500 × 10ms ticks (5 seconds) with FM switches
  mixerCurrentFlightMode = 0;
  mixerTickRealistic(150);  // 1.5s in FM0

  mixerCurrentFlightMode = 2;
  mixerTickRealistic(200);  // 2.0s in FM2

  mixerCurrentFlightMode = 0;
  mixerTickRealistic(150);  // 1.5s back in FM0

  // Timer lastValue should be identical across all FMs
  // (logicalSwitchesTimerTick updates all FMs identically)
  int16_t fm0val = lswFm[0].lsw[0].lastValue;
  for (int fm = 1; fm < MAX_FLIGHT_MODES; fm++) {
    EXPECT_EQ(fm0val, lswFm[fm].lsw[0].lastValue)
        << "Timer lastValue diverged at fm=" << fm;
  }
}

// Test: Sticky LS with realistic timing and FM switching.
TEST_F(LswPerFmTest, StickyRealisticTiming)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw++)
    if (g_model.getSwitchType(sw) == SWITCH_3POS) break;
  auto swUp = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3));
  auto swDn = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3 + 2));

  // LS0: STICKY(sw_up, sw_down)
  lswAllocAt(0)->func = LS_FUNC_STICKY;
  lswAddress(0)->v1.swtch = swUp;
  lswAddress(0)->v2.swtch = swDn;
  lswAddress(0)->andsw = SWTCH_NONE;

  logicalSwitchesReset();
  s_test_100ms_cnt = 0;

  // Trigger ON in FM0
  simuSetSwitch(sw, -1);
  mixerCurrentFlightMode = 0;
  mixerTickRealistic(50);  // 0.5s

  // Switch to FM1 and release
  simuSetSwitch(sw, 0);
  mixerCurrentFlightMode = 1;
  mixerTickRealistic(50);

  // Trigger OFF while in FM1
  simuSetSwitch(sw, 1);
  mixerTickRealistic(50);

  // Switch back to FM0
  simuSetSwitch(sw, 0);
  mixerCurrentFlightMode = 0;
  mixerTickRealistic(20);

  // Check: are sticky lastValues still identical across FMs?
  // (logicalSwitchesTimerTick processes sticky for all FMs with same inputs)
  int16_t fm0val = lswFm[0].lsw[0].lastValue;
  for (int fm = 1; fm < MAX_FLIGHT_MODES; fm++) {
    EXPECT_EQ(fm0val, lswFm[fm].lsw[0].lastValue)
        << "Sticky lastValue diverged at fm=" << fm;
  }
}

// ---------------------------------------------------------------------------
// Per-FM logical switch state was introduced in opentx/opentx#1119 (May 2014)
// to give each flight mode independent LS evaluation during fade transitions.
// One month later, opentx/opentx commit f3bc8cb9a4 (fixing #1345) gated
// evalLogicalSwitches() behind `if (tick10ms)`, which is zero for non-current
// FMs during fades.  This made per-FM LS evaluation dead code.
//
// This test proves the per-FM state was already non-functional BEFORE it was
// removed: the non-current FM's LS context was never re-evaluated during a
// fade, so it carried stale state from the moment of transition.
//
// After the flatten (lswFm[] → lswCtx[]), getSwitch() reads a single global
// context, so both FMs see the same (fresh) value.  The assertion marked
// [POST-FLATTEN] documents this corrected behavior.
// ---------------------------------------------------------------------------
TEST(getSwitch, perFmLsStateNotEvaluatedDuringFade)
{
  SYSTEM_RESET();
  MODEL_RESET();
  setModelDefaults();
  MIXER_RESET();

  // Find a 3-position switch to activate FM1
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw++)
    if (g_model.getSwitchType(sw) == SWITCH_3POS) break;
  auto swDown = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3 + 2));

  // LS0: VPOS(stick0 > 0) — true when stick is positive
  lswAllocAt(0)->func = LS_FUNC_VPOS;
  lswAddress(0)->v1.source = SourceRef_(SOURCE_TYPE_STICK, 0);
  lswAddress(0)->v2.value = 0;
  lswAddress(0)->v3 = 0;
  lswAddress(0)->andsw = SWTCH_NONE;

  // FM1: activated by switch down, fadeIn = 10 (1.0s)
  flightModeAddress(1)->swtch = swDown;
  flightModeAddress(1)->fadeIn = 10;
  flightModeAddress(0)->fadeOut = 10;

  // --- Step 1: Start in FM0 with stick positive → LS0 = TRUE ---
  simuSetSwitch(sw, -1);  // switch up → FM0 active
  anaSetFiltered(0, 512); // stick positive → LS0 condition true

  evalMixes(1);
  mixerCurrentFlightMode = 0;
  evalLogicalSwitches(true);

  EXPECT_TRUE(getSwitch(SW1_REF)) << "LS0 should be TRUE with positive stick";

  // --- Step 2: Trigger FM transition to FM1 (fade begins) ---
  simuSetSwitch(sw, 1);  // switch down → getFlightMode() returns FM1

  // First evalMixes detects FM change and starts the fade.
  // LS0 is still TRUE at this point.
  evalMixes(1);

  // --- Step 3: Change stick so LS0 should become FALSE ---
  anaSetFiltered(0, -512); // stick negative → LS0 condition false

  // --- Step 4: Run several mixer cycles during the fade ---
  // The fade loop sets mixerCurrentFlightMode to each fading FM and calls
  // evalFlightModeMixes(). For the non-current FM (FM0), tick10ms=0, so
  // evalLogicalSwitches() is NOT called. Only FM1 (current) gets evaluation.
  for (int i = 0; i < 5; i++) {
    evalMixes(1);
  }

  // --- Step 5: Verify the current FM sees the fresh (FALSE) state ---
  uint8_t savedFM = mixerCurrentFlightMode;
  mixerCurrentFlightMode = 1;  // current FM
  EXPECT_FALSE(getSwitch(SW1_REF))
    << "Current FM should see LS0=FALSE (freshly evaluated)";

  // --- Step 6: Check the fading-out FM's view ---
  // Pre-flatten: lswFm[0] was stale (TRUE) because evalLogicalSwitches was
  //   never called for FM0 during the fade (tick10ms=0 guard).
  // Post-flatten: single lswCtx[] is always fresh → returns FALSE.
  mixerCurrentFlightMode = 0;  // fading-out FM
  bool fadingOutFmResult = getSwitch(SW1_REF);

  mixerCurrentFlightMode = savedFM;

  // [POST-FLATTEN] With a single global context, both FMs see the same
  // fresh FALSE value. Before the flatten, this was TRUE (stale).
  EXPECT_FALSE(fadingOutFmResult)
    << "After flatten: fading-out FM sees fresh FALSE from single context";
}
