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

//#if defined(EEPROM)
#define MODELNAME TR_MODEL "01"
/*#else
#define MODELNAME "Model00"
#endif*/

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

#if defined(PCBTARANIS) && NUM_SWITCHES >= 8 && !defined(PCBX7)

#define SWSRC_SA2 (SWSRC_FIRST_SWITCH + 2)
#define SWSRC_SF2 (SWSRC_FIRST_SWITCH + 5 * 3 + 2)

TEST(getSwitch, edgeInstant)
{
  MODEL_RESET();
  MIXER_RESET();
  // LS1 setup: EDGE SFup  (0:instant)
  // LS2 setup: (EDGE SFup  (0:instant)) AND SAup
  setLogicalSwitch(0, LS_FUNC_EDGE, SWSRC_SF2, -129, -1);
  setLogicalSwitch(1, LS_FUNC_EDGE, SWSRC_SF2, -129, -1, 0, 0, SWSRC_SA2);

  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(5, 0);   //SF down
  // EXPECT_EQ(getSwitch(SWSRC_SF2), false);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SFup, LS1 should become true
  simuSetSwitch(5, 1);    //SF up
  // EXPECT_EQ(getSwitch(SWSRC_SF2), true);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(5, 0);   //SF down
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
  simuSetSwitch(5, 0);   //SF down
  // EXPECT_EQ(getSwitch(SWSRC_SF2), false);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  // now trigger SFup, LS1 should become true
  simuSetSwitch(5, 1);    //SF up
  // EXPECT_EQ(getSwitch(SWSRC_SF2), true);
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), true);

  // now release SA0 and SW2 should stay true
  simuSetSwitch(5, 0);   //SF down
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
  // SF is kept up and SA is toggled
  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  //test what happens when EDGE condition is true and
  //logical switches are reset - the switch should fire again

  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(5, 1);    //SF up
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
  MODEL_RESET();
  MIXER_RESET();
  // test for issue #2728
  // LS1 setup: EDGE SFup  (0:release)
  // LS2 setup: (EDGE SFup  (0:release)) AND SAup
  setLogicalSwitch(0, LS_FUNC_EDGE, SWSRC_SF2, -129, 0);
  setLogicalSwitch(1, LS_FUNC_EDGE, SWSRC_SF2, -129, 0, 0, 0, SWSRC_SA2 );

  simuSetSwitch(0, -1);   //SA down
  simuSetSwitch(5, 0);   //SF down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(5, 0);   //SF down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), true);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);


  // second part with SAup
  simuSetSwitch(0, 1);   //SA up
  simuSetSwitch(5, 0);   //SF down
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(5, 1);    //SF up
  logicalSwitchesTimerTick();
  evalLogicalSwitches();
  EXPECT_EQ(getSwitch(SWSRC_SW1), false);
  EXPECT_EQ(getSwitch(SWSRC_SW2), false);

  simuSetSwitch(5, 0);   //SF down
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
#endif // defined(PCBTARANIS)
