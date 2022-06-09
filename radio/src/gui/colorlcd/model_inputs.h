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

#include "tabsgroup.h"

class InputMixGroup;
class InputMixButton;

class ModelInputsPage : public PageTab
{
 public:
  ModelInputsPage();

  void build(FormWindow *window) override;

 protected:
  FormGroup* form = nullptr;
  std::list<InputMixGroup*> groups;
  std::list<InputMixButton*> lines;
  InputMixButton* _copySrc = nullptr;
  uint8_t _copyMode = 0;

  InputMixGroup* getGroupBySrc(mixsrc_t src);
  virtual InputMixGroup* getGroupByIndex(uint8_t index);
  InputMixButton* getLineByIndex(uint8_t index);

  void removeGroup(InputMixGroup* g);
  void removeLine(InputMixButton* l);
  
  virtual void addLineButton(uint8_t index);
  virtual void addLineButton(mixsrc_t src, uint8_t index);
  virtual InputMixGroup* createGroup(FormGroup* form, mixsrc_t src);
  virtual InputMixButton* createLineButton(InputMixGroup *group, uint8_t index);

  void editInput(uint8_t input, uint8_t index);
  void insertInput(uint8_t input, uint8_t index);
  void deleteInput(uint8_t index);

  void pasteInput(uint8_t dst_idx, uint8_t input);
  void pasteInputBefore(uint8_t dst_idx);
  void pasteInputAfter(uint8_t dst_idx);
};
