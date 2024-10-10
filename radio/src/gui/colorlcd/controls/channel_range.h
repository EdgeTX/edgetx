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

#pragma once

#include "window.h"

class NumberEdit;

class ChannelRange : public Window
{
 public:
  ChannelRange(Window* parent);

  void setStart(uint8_t newValue);
  void setEnd(uint8_t newValue);

  virtual void update();
  void updateStart();
  void updateEnd();

  virtual int8_t getChannelsCount() = 0;
  void setPpmFrameLenEditObject(NumberEdit* ppmFrameLenEditObject);

 protected:
  NumberEdit* chStart;
  NumberEdit* chEnd;
  NumberEdit* ppmFrameLenEditObject = nullptr;

  void build();

  virtual uint8_t getChannelsStart() = 0;
  virtual void setChannelsStart(uint8_t val) = 0;

  virtual void setChannelsCount(int8_t val) = 0;

  virtual uint8_t getChannelsUsed() = 0;
  virtual uint8_t getChannelsMin() = 0;
  virtual uint8_t getChannelsMax() = 0;
};

class ModuleChannelRange : public ChannelRange
{
  uint8_t moduleIdx;

  void update() override;

  uint8_t getChannelsStart() override;
  void setChannelsStart(uint8_t val) override;

  void setChannelsCount(int8_t val) override;

  uint8_t getChannelsUsed() override;
  uint8_t getChannelsMin() override;
  uint8_t getChannelsMax() override;

 public:
  ModuleChannelRange(Window* parent, uint8_t moduleIdx);

  int8_t getChannelsCount() override;
};

class TrainerChannelRange : public ChannelRange
{
  uint8_t getChannelsStart() override;
  void setChannelsStart(uint8_t val) override;

  void setChannelsCount(int8_t val) override;

  uint8_t getChannelsUsed() override;
  uint8_t getChannelsMin() override;
  uint8_t getChannelsMax() override;

 public:
  TrainerChannelRange(Window* parent);

  int8_t getChannelsCount() override;
};
