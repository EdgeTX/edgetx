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
#include "wizarddataair.h"

Plane::MixerChannel::Channel()
{
  clear();
}

void Plane::MixerChannel::clear()
{
  page = Vehicle::PAGE_NONE;
  prebooked = false;
  input1 = AirVehicle::NO_INPUT;
  input2 = AirVehicle::NO_INPUT;
  weight1 = 0;
  weight2 = 0;
}

Plane::WizMix::WizMix(const GeneralSettings & settings, unsigned int modelId, const ModelData & modelData):
  complete(false),
  modelId(modelId),
  settings(settings),
  originalModelData(modelData),
  vehicle(Vehicle::TYPE)
{
  memset(name, 0, sizeof(name));
  memcpy(name, originalModelData.name, sizeof(name) - 1);
  name[sizeof(name) - 1] = '\0';
}

void Plane::WizMix::maxMixSwitch(char *name, MixData &mix, int channel, int sw, int weight)
{
  memset(mix.name, 0, sizeof(mix.name));
  strncpy(mix.name, name, sizeof(mix.name)-1);
  mix.destCh = channel;
  mix.srcRaw = RawSource(SOURCE_TYPE_MAX);
  mix.swtch  = RawSwitch(SWITCH_TYPE_SWITCH, sw + 1);
  mix.weight = weight;
}

void Plane::WizMix::addMix(ModelData &model, int input, int weight, int channel, int & mixIndex)
{
  if (input != AirVehicle::NO_INPUT)  {
    if (input >= AirVehicle::RUDDER_INPUT && input <= AirVehicle::AILERONS_INPUT) {
      MixData & mix = model.mixData[mixIndex++];
      mix.destCh = channel + 1;
      mix.srcRaw = RawSource(SOURCE_TYPE_VIRTUAL_INPUT, settings.getDefaultChannel(input - 1) + 1);
      mix.weight = weight;
    }
    else if (input==AirVehicle::FLAPS_INPUT){
      // There ought to be some kind of constants for switches somewhere...
      maxMixSwitch((char *)tr("FlapUp").toLatin1().data(), model.mixData[mixIndex++], channel+1, SWITCH_SA0 - 1, weight);
      maxMixSwitch((char *)tr("FlapDn").toLatin1().data(), model.mixData[mixIndex++], channel+1, SWITCH_SA2 - 1, -weight);

    }
    else if (input==AirVehicle::AIRBRAKES_INPUT){
      maxMixSwitch((char *)tr("ArbkOf").toLatin1().data(), model.mixData[mixIndex++], channel+1, SWITCH_SE0 - 1, -weight);
      maxMixSwitch((char *)tr("ArbkOn").toLatin1().data(),  model.mixData[mixIndex++], channel+1, SWITCH_SE2 - 1, weight);
    }
  }
}

Plane::WizMix::operator ModelData()
{
  int throttleChannel = -1;

  ModelData model;
  model.labels[0] = '\0';
  model.used = true;
  model.moduleData[0].modelId = modelId;
  model.setDefaultInputs(settings);
  model.setDefaultFunctionSwitches(Boards::getCapability(getCurrentFirmware()->getBoard(), Board::FunctionSwitches));

  int mixIndex = 0;
  int timerIndex = 0;

  memset(model.name, 0, sizeof(model.name));
  memcpy(model.name, name, sizeof(model.name) - 1);
  model.name[sizeof(model.name) - 1] = '\0';

  // Add the channel mixes
  for (int i=0; i<WIZ_MAX_CHANNELS; i++ )
  {
    Plane::MixerChannel ch = channel[i];

    addMix(model, ch.input1, ch.weight1, i, mixIndex);
    addMix(model, ch.input2, ch.weight2, i, mixIndex);

    if (ch.input1 == AirVehicle::THROTTLE_INPUT || ch.input2 == AirVehicle::THROTTLE_INPUT) {
      throttleChannel++;
      if (options[AirVehicle::THROTTLE_CUT_OPTION]) {
        // Add the Throttle Cut option
        MixData & mix = model.mixData[mixIndex++];
        mix.destCh = i+1;
        mix.srcRaw = RawSource(SOURCE_TYPE_MAX);
        mix.weight = -100;
        mix.swtch.type = SWITCH_TYPE_SWITCH;
        mix.swtch.index = SWITCH_SF0;
        mix.mltpx = MLTPX_REP;
        memset(mix.name, 0, sizeof(mix.name));
        strncpy(mix.name, Plane::WizMix::tr("Cut").toLatin1().data(), MIXDATA_NAME_LEN);
      }
    }
  }

  // Add the Flight Timer option
  if (options[AirVehicle::FLIGHT_TIMER_OPTION] && throttleChannel >= 0){
    memset(model.timers[timerIndex].name, 0, sizeof(model.timers[timerIndex].name));
    strncpy(model.timers[timerIndex].name, Plane::WizMix::tr("Flt").toLatin1().data(), sizeof(model.timers[timerIndex].name)-1);
    model.timers[timerIndex].mode = TimerData::TIMERMODE_START;
    timerIndex++;
  }

  // Add the Throttle Timer option
  if (options[AirVehicle::THROTTLE_TIMER_OPTION] && throttleChannel >= 0){
    memset(model.timers[timerIndex].name, 0, sizeof(model.timers[timerIndex].name));
    strncpy(model.timers[timerIndex].name, Plane::WizMix::tr("Thr").toLatin1().data(), sizeof(model.timers[timerIndex].name)-1);
    model.timers[timerIndex].mode = TimerData::TIMERMODE_THR;
    timerIndex++;
  }

  return model;
}
