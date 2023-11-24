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
#include "opentx_types.h"

#include <list>

class MixerChannelBar;

class InputMixGroupBase : public Window
{
 public:
  InputMixGroupBase(Window* parent, mixsrc_t idx,
                    const lv_coord_t* gridCols = nullptr);
    
  mixsrc_t getMixSrc() { return idx; }
  size_t getLineCount() { return lines.size(); }
  
  void addLine(Window* line);
  bool removeLine(Window* line);
  
  void refresh();

 protected:
  mixsrc_t idx;
  std::list<Window*> lines;

  lv_obj_t* label;
  lv_obj_t* line_container;
};

class InputMixPageBase : public PageTab
{
 public:
  InputMixPageBase(std::string title, EdgeTxIcon icon) :
    PageTab(title, icon) {}

 protected:
  Window* form = nullptr;
  uint8_t _copyMode = 0;
  std::list<InputMixGroupBase*> groups;

  InputMixGroupBase* getGroupBySrc(mixsrc_t src);

  void removeGroup(InputMixGroupBase* g);
};
