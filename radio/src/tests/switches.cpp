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
  lswAddress(0)->func = LS_FUNC_AND;
  lswAddress(0)->v1.swtch = swRef;
  lswAddress(0)->v2.swtch = SWTCH_NONE;
  lswAddress(0)->andsw = SWTCH_NONE;

  // LS1: OR(SW1, SW2)
  lswAddress(1)->func = LS_FUNC_OR;
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
  lswAddress(0)->func = LS_FUNC_VPOS;
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
  lswAddress(0)->func = LS_FUNC_EDGE;
  lswAddress(0)->v1.swtch = sw2Down;
  lswAddress(0)->v2.value = -129;
  lswAddress(0)->v3 = -1;
  lswAddress(0)->andsw = SWTCH_NONE;

  lswAddress(1)->func = LS_FUNC_EDGE;
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
  lswAddress(0)->func = LS_FUNC_EDGE;
  lswAddress(0)->v1.swtch = sw2Down;
  lswAddress(0)->v2.value = -129;
  lswAddress(0)->v3 = 0;
  lswAddress(0)->andsw = SWTCH_NONE;

  lswAddress(1)->func = LS_FUNC_EDGE;
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
