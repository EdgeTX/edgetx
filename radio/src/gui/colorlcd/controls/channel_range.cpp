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

#include "channel_range.h"

#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

inline int16_t ppmFrameLen(int8_t chCount)
{
#define PPM_PULSE_LEN_MAX \
  (4 * PPM_STEP_SIZE)  // let's assume roughly 2ms max pulse length

  if (chCount > 0)
    return ((PPM_PULSE_LEN_MAX * chCount) +
            PPM_DEF_PERIOD);  // for more than 8 channels update frame length
  else
    return (PPM_DEF_PERIOD);  // else leave frame Length at default (22.5ms)
}

ChannelRange::ChannelRange(Window* parent) : Window(parent, rect_t{})
{
  padAll(PAD_TINY);
  setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL, LV_SIZE_CONTENT);
}

void ChannelRange::build()
{
  chStart = new NumberEdit(this, rect_t{0, 0, 80, 0}, 1, 1,
                           GET_DEFAULT(1 + getChannelsStart()));
  chStart->setSetValueHandler([=](int newValue) { setStart(newValue); });
  chStart->setPrefix(STR_CH);

  chEnd =
      new NumberEdit(this, rect_t{0, 0, 80, 0}, 8, 8,
                     GET_DEFAULT(getChannelsStart() + 8 + getChannelsCount()));

  chEnd->setPrefix(STR_CH);
  chEnd->setSetValueHandler([=](int newValue) {
    setEnd(newValue);

    if (ppmFrameLenEditObject)
      ppmFrameLenEditObject->setValue(ppmFrameLen(getChannelsCount()));
  });
}

void ChannelRange::setStart(uint8_t newValue)
{
  setChannelsStart(newValue - 1);
  updateEnd();
  SET_DIRTY();
}

void ChannelRange::setEnd(uint8_t newValue)
{
  setChannelsCount(newValue - getChannelsStart() - 8);
  updateStart();
  SET_DIRTY();
}

void ChannelRange::updateEnd()
{
  auto min_ch = getChannelsStart() + getChannelsMin();
  chEnd->setMin(min_ch);

  auto max_ch = getChannelsStart() + getChannelsMax();
  max_ch = min<int8_t>(MAX_OUTPUT_CHANNELS, max_ch);
  chEnd->setMax(max_ch);
  chEnd->setDefault(max_ch);

  // Force value to fit between new min and max
  chEnd->setValue(chEnd->getValue());
}

void ChannelRange::updateStart()
{
  chStart->setMax(MAX_OUTPUT_CHANNELS - getChannelsUsed() + 1);
}

void ChannelRange::update()
{
  updateStart();
  updateEnd();
}

void ChannelRange::setPpmFrameLenEditObject(NumberEdit* ppmFrameLenEditObject)
{
  this->ppmFrameLenEditObject = ppmFrameLenEditObject;
}

ModuleChannelRange::ModuleChannelRange(Window* parent, uint8_t moduleIdx) :
    ChannelRange(parent), moduleIdx(moduleIdx)
{
  build();
  update();
}

void ModuleChannelRange::update()
{
  ChannelRange::update();

  auto min_mod_ch = minModuleChannels(moduleIdx);
  auto max_mod_ch = maxModuleChannels(moduleIdx);
  chEnd->enable(min_mod_ch < max_mod_ch);

  if (chEnd->getValue() > chEnd->getMax()) chEnd->setValue(chEnd->getMax());

  if (!isModulePXX2(moduleIdx)) {
    chEnd->setAvailableHandler(nullptr);
  }
#if defined(PXX2)
  else {
    chEnd->setAvailableHandler(
        [=](int value) { return isPxx2IsrmChannelsCountAllowed(value - 8); });
  }
#endif
}

uint8_t ModuleChannelRange::getChannelsStart()
{
  ModuleData* md = &g_model.moduleData[moduleIdx];
  return md->channelsStart;
}

void ModuleChannelRange::setChannelsStart(uint8_t val)
{
  ModuleData* md = &g_model.moduleData[moduleIdx];
  md->channelsStart = val;
}

int8_t ModuleChannelRange::getChannelsCount()
{
  ModuleData* md = &g_model.moduleData[moduleIdx];
  return md->channelsCount;
}

void ModuleChannelRange::setChannelsCount(int8_t val)
{
  ModuleData* md = &g_model.moduleData[moduleIdx];
  md->channelsCount = val;
}

uint8_t ModuleChannelRange::getChannelsUsed()
{
  return sentModuleChannels(moduleIdx);
}

uint8_t ModuleChannelRange::getChannelsMin()
{
  return minModuleChannels(moduleIdx);
}

uint8_t ModuleChannelRange::getChannelsMax()
{
  return maxModuleChannels(moduleIdx);
}

TrainerChannelRange::TrainerChannelRange(Window* parent) : ChannelRange(parent)
{
  build();
  update();
}

uint8_t TrainerChannelRange::getChannelsStart()
{
  return g_model.trainerData.channelsStart;
}

void TrainerChannelRange::setChannelsStart(uint8_t val)
{
  g_model.trainerData.channelsStart = val;
}

int8_t TrainerChannelRange::getChannelsCount()
{
  return g_model.trainerData.channelsCount;
}

void TrainerChannelRange::setChannelsCount(int8_t val)
{
  g_model.trainerData.channelsCount = val;
}

uint8_t TrainerChannelRange::getChannelsUsed()
{
  return 8 + getChannelsCount();
}

uint8_t TrainerChannelRange::getChannelsMin() { return MIN_TRAINER_CHANNELS; }

uint8_t TrainerChannelRange::getChannelsMax() { return MAX_TRAINER_CHANNELS; }
