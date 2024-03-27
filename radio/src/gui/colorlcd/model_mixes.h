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

#include "input_mix_group.h"

class MixLineButton;

class MixGroup : public InputMixGroupBase
{
 public:
  MixGroup(Window* parent, mixsrc_t idx);

  void enableMixerMonitor();
  void disableMixerMonitor();

  void addMPlex(Window* mplex);

 protected:
  MixerChannelBar* monitor = nullptr;
  lv_obj_t* mplex_container;
};

class ModelMixesPage : public InputMixPageBase
{
 public:
  ModelMixesPage();

  void build(Window* window) override;

 protected:
  std::list<MixLineButton*> lines;
  MixLineButton* _copySrc = nullptr;

  bool showMonitors = false;

  MixGroup* getGroupByIndex(uint8_t index);
  MixLineButton* getLineByIndex(uint8_t index);

  void removeLine(MixLineButton* l);

  void addLineButton(uint8_t index);
  void addLineButton(mixsrc_t src, uint8_t index);
  MixGroup* createGroup(Window* form, mixsrc_t src);
  MixLineButton* createLineButton(MixGroup* group, uint8_t index);

  void newMix();
  void editMix(uint8_t input, uint8_t index);
  void insertMix(uint8_t input, uint8_t index);
  void deleteMix(uint8_t index);

  void pasteMix(uint8_t dst_idx, uint8_t channel);
  void pasteMixBefore(uint8_t dst_idx);
  void pasteMixAfter(uint8_t dst_idx);

  void enableMonitors(bool enabled);

  bool reachMixesLimit();
};
