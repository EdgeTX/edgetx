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

#include <string.h>
#include "eeprominterface.h"
#include "wizarddata.h"

Channel::Channel()
{
  clear();
}

void Channel::clear()
{
  page = Page_None;
  prebooked = false;
  input1 = NO_INPUT;
  input2 = NO_INPUT;
  weight1 = 0;
  weight2 = 0;
}

WizMix::WizMix(const GeneralSettings & settings, unsigned int modelId, const ModelData & modelData):
  complete(false),
  modelId(modelId),
  settings(settings),
  originalModelData(modelData),
  vehicle(NOVEHICLE)
{
  memset(name, 0, sizeof(name));
  strncpy(name, originalModelData.name, sizeof(name)-1);
}

void WizMix::maxMixSwitch(char *name, MixData &mix, int channel, int sw, int weight)
{
  memset(mix.name, 0, sizeof(mix.name));
  strncpy(mix.name, name, sizeof(mix.name)-1);
  mix.destCh = channel;
  mix.srcRaw = RawSource(SOURCE_TYPE_MAX);
  mix.swtch  = RawSwitch(SWITCH_TYPE_SWITCH, sw + 1);
  mix.weight = weight;
}

void WizMix::addMix(ModelData &model, Input input, int weight, int channel, int & mixIndex)
{
  if (input != NO_INPUT)  {
    if (input >= RUDDER_INPUT && input <= AILERONS_INPUT) {
      MixData & mix = model.mixData[mixIndex++];
      mix.destCh = channel + 1;
      if (IS_SKY9X(getCurrentBoard())) {
        mix.srcRaw = RawSource(SOURCE_TYPE_INPUT, input + 1);
      }
      else {
        int channel = settings.getDefaultChannel(input - 1);
        mix.srcRaw = RawSource(SOURCE_TYPE_VIRTUAL_INPUT, channel + 1);
      }

      mix.weight = weight;
    }
    else if (input==FLAPS_INPUT){
      // There ought to be some kind of constants for switches somewhere...
      maxMixSwitch((char *)tr("Flaps Up").toLatin1().data(), model.mixData[mixIndex++], channel+1, IS_SKY9X(getCurrentBoard()) ? -SWITCH_ELE : SWITCH_SA0, weight); //Taranis-Horus SA-UP, 9X ELE-UP
      maxMixSwitch((char *)tr("Flaps Dn").toLatin1().data(), model.mixData[mixIndex++], channel+1, IS_SKY9X(getCurrentBoard()) ? SWITCH_ELE : SWITCH_SA2, -weight); //Taranis-Horus SA-DOWN, 9X ELE-DOWN

    }
    else if (input==AIRBRAKES_INPUT){
      maxMixSwitch((char *)tr("AirbkOff").toLatin1().data(), model.mixData[mixIndex++], channel+1, IS_SKY9X(getCurrentBoard()) ? -SWITCH_RUD : SWITCH_SE0, -weight); //Taranis-Horus SE-UP, 9X RUD-UP
      maxMixSwitch((char *)tr("AirbkOn").toLatin1().data(),  model.mixData[mixIndex++], channel+1, IS_SKY9X(getCurrentBoard()) ? SWITCH_RUD : SWITCH_SE2, weight); //Tatanis-Horus SE-DOWN, 9X RUD-DOWN
    }
  }
}

WizMix::operator ModelData()
{
  int throttleChannel = -1;

  ModelData model;
  model.labels[0] = '\0';
  model.used = true;
  model.moduleData[0].modelId = modelId;
  model.setDefaultInputs(settings);

  int mixIndex = 0;
  int timerIndex = 0;

  // Safe copy model name
  memset(model.name, 0, sizeof(model.name));
  strncpy(model.name, name, sizeof(model.name)-1);

  // Add the channel mixes
  for (int i=0; i<WIZ_MAX_CHANNELS; i++ )
  {
    Channel ch = channel[i];

    addMix(model, ch.input1, ch.weight1, i, mixIndex);
    addMix(model, ch.input2, ch.weight2, i, mixIndex);

    if (ch.input1 == THROTTLE_INPUT || ch.input2 == THROTTLE_INPUT) {
      throttleChannel++;
      if (options[THROTTLE_CUT_OPTION]) {
        // Add the Throttle Cut option
        MixData & mix = model.mixData[mixIndex++];
        mix.destCh = i+1;
        mix.srcRaw = RawSource(SOURCE_TYPE_MAX);
        mix.weight = -100;
        mix.swtch.type = SWITCH_TYPE_SWITCH;
        mix.swtch.index = IS_SKY9X(getCurrentBoard()) ? SWITCH_THR : SWITCH_SF0;
        mix.mltpx = MLTPX_REP;
        memset(mix.name, 0, sizeof(mix.name));
        strncpy(mix.name, tr("Cut").toLatin1().data(), MIXDATA_NAME_LEN);
      }
    }
  }

  // Add the Flight Timer option
  if (options[FLIGHT_TIMER_OPTION] && throttleChannel >= 0){
    memset(model.timers[timerIndex].name, 0, sizeof(model.timers[timerIndex].name));
    strncpy(model.timers[timerIndex].name, tr("Flt").toLatin1().data(), sizeof(model.timers[timerIndex].name)-1);
    model.timers[timerIndex].mode = TimerData::TIMERMODE_START;
    timerIndex++;
  }

  // Add the Throttle Timer option
  if (options[THROTTLE_TIMER_OPTION] && throttleChannel >= 0){
    memset(model.timers[timerIndex].name, 0, sizeof(model.timers[timerIndex].name));
    strncpy(model.timers[timerIndex].name, tr("Thr").toLatin1().data(), sizeof(model.timers[timerIndex].name)-1);
    model.timers[timerIndex].mode = TimerData::TIMERMODE_THR;
    timerIndex++;
  }

  return model;
}
