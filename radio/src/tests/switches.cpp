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

  setLogicalSwitch(0, LS_FUNC_AND, SWSRC_FIRST_SWITCH, SWSRC_NONE);
  setLogicalSwitch(1, LS_FUNC_OR, SWSRC_SW1, SWSRC_SW2);

  simuSetSwitch(0, 0);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SA0, both switches should become true
  simuSetSwitch(0, -1);
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(0, 0);
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

#if defined(PCBFRSKY)
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
#endif

#define SWSRC_SA2 (SWSRC_FIRST_SWITCH + 2)
#define SWSRC_SD2 (SWSRC_FIRST_SWITCH + 3 * 3 + 2)

TEST(getSwitch, edgeInstant)
{
  if (switchGetMaxSwitches() < 6) return;
  
  MODEL_RESET();
  MIXER_RESET();
  // LS1 setup: EDGE SDup  (0:instant)
  // LS2 setup: (EDGE SDup  (0:instant)) AND SAup
  setLogicalSwitch(0, LS_FUNC_EDGE, SWSRC_SD2, -129, -1);
  setLogicalSwitch(1, LS_FUNC_EDGE, SWSRC_SD2, -129, -1, 0, 0, SWSRC_SA2);

  simuSetSwitch(0, -1);  //SA down
  simuSetSwitch(3, 0);   //SD down
  // EXPECT_EQ(getSwitch(SWSRC_SD2), false);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SFup, LS1 should become true
  simuSetSwitch(3, 1);    //SD up
  // EXPECT_EQ(getSwitch(SWSRC_SD2), true);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(3, 0);   //SD down
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


  // second part with SAup

  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(3, 0);   //SD down
  // EXPECT_EQ(getSwitch(SWSRC_SD2), false);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SFup, LS1 should become true
  simuSetSwitch(3, 1);    //SD up
  // EXPECT_EQ(getSwitch(SWSRC_SD2), true);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(3, 0);   //SD down
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
  // SD is kept up and SA is toggled
  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(3, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(3, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(3, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  //test what happens when EDGE condition is true and
  //logical switches are reset - the switch should fire again

  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(3, 1);    //SD up
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
  if (switchGetMaxSwitches() < 6) return;

  MODEL_RESET();
  MIXER_RESET();
  // test for issue #2728
  // LS1 setup: EDGE SDup  (0:release)
  // LS2 setup: (EDGE SDup  (0:release)) AND SAup
  setLogicalSwitch(0, LS_FUNC_EDGE, SWSRC_SD2, -129, 0);
  setLogicalSwitch(1, LS_FUNC_EDGE, SWSRC_SD2, -129, 0, 0, 0, SWSRC_SA2 );

  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(3, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(3, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(3, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);


  // second part with SAup
  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(3, 0);   //SD down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(3, 1);    //SD up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(3, 0);   //SD down
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
  auto sw_name = switchGetName(sw_idx);
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

  g_eeGeneral.switchConfig = (swconfig_t)SWITCH_3POS << (sw_idx * SW_CFG_BITS);
  EXPECT_EQ(SWITCH_3POS, SWITCH_CONFIG(sw_idx));

  auto offset = adcGetInputOffset(ADC_INPUT_FLEX);
  anaSetFiltered(offset, -1024);
  EXPECT_EQ(-1024, getValue(MIXSRC_FIRST_SWITCH + sw_idx));

  anaSetFiltered(offset, 0);
  EXPECT_EQ(0, getValue(MIXSRC_FIRST_SWITCH + sw_idx));

  anaSetFiltered(offset, +1024);
  EXPECT_EQ(+1024, getValue(MIXSRC_FIRST_SWITCH + sw_idx));

  g_eeGeneral.switchConfig = (swconfig_t)SWITCH_2POS << (sw_idx * SW_CFG_BITS);
  EXPECT_EQ(SWITCH_2POS, SWITCH_CONFIG(sw_idx));

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

  g_eeGeneral.switchConfig = (swconfig_t)SWITCH_3POS << (sw_idx * SW_CFG_BITS);
  EXPECT_EQ(SWITCH_3POS, SWITCH_CONFIG(sw_idx));

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
