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
  int swPos = (sw * 3) + SWSRC_FIRST_SWITCH;

  setLogicalSwitch(0, LS_FUNC_AND, swPos, SWSRC_NONE);
  setLogicalSwitch(1, LS_FUNC_OR, SWSRC_SW1, SWSRC_SW2);

  simuSetSwitch(sw, 0);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SA0, both switches should become true
  simuSetSwitch(sw, -1);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(sw, 0);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now reset logical switches
  logicalSwitchesReset();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);
}
#endif

TEST(getSwitch, nullSW)
{
  MODEL_RESET();
  EXPECT_EQ(getSwitch(0), true);
}


#if defined(PCBTARANIS)
TEST(getSwitch, inputWithTrim)
{
  MODEL_RESET();
  setModelDefaults();
  MIXER_RESET();

  setLogicalSwitch(0, LS_FUNC_VPOS, MIXSRC_FIRST_INPUT, 0, 0);
  anaSetFiltered(0, 0);

  evalMixes(1);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);

  setTrimValue(0, 0, 32);
  evalMixes(1);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
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

#define MODELNAME TR_MODEL "01"

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

  EXPECT_EQ(isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (31 << 16) + AUDIO_EVENT_ON, filename), true);
  EXPECT_EQ(isAudioFileReferenced((LOGICAL_SWITCH_AUDIO_CATEGORY << 24) + (32 << 16) + AUDIO_EVENT_ON, filename), false);

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
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SD donw, LS1 should become true
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now release SD and LS1 should become false
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now reset logical switches
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // second part with SA down

  simuSetSwitch(sw1, 1);   //SA down
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SD down, LS1 & LS2 should become true
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now release SA and LS1 & LS2 should stay false
  simuSetSwitch(sw2, 0);   //SD mid
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now reset logical switches
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now bug #2939
  // SD is kept down and SA is toggled
  simuSetSwitch(sw1, -1);   //SA up
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(sw1, 1);   //SA down
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(sw1, -1);   //SA up
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  //test what happens when EDGE condition is true and
  //logical switches are reset - the switch should fire again

  simuSetSwitch(sw1, 1);   //SA down
  simuSetSwitch(sw2, 1);    //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);  //switch will not trigger, because SF was already up
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);
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
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(sw2, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);


  // second part with SAup
  simuSetSwitch(sw1, 1);   //SA up
  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(sw2, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(sw2, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // with switches reset both should remain false
  logicalSwitchesReset();
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

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
  EXPECT_EQ(true, getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3));
  EXPECT_EQ(false, getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 1));
  EXPECT_EQ(false, getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 2));

  anaSetFiltered(offset, 0);
  EXPECT_EQ(false, getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3));
  EXPECT_EQ(true, getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 1));
  EXPECT_EQ(false, getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 2));

  anaSetFiltered(offset, +1024);
  EXPECT_EQ(false, getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3));
  EXPECT_EQ(false, getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 1));
  EXPECT_EQ(true, getSwitch(SWSRC_FIRST_SWITCH + sw_idx * 3 + 2));
}
