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
#include "opentx_types.h"

#include <list>

class InputMixGroup : public Window
{
  struct Line {
    Window*        win;
    const uint8_t* symbol;

    Line(Window* w, const uint8_t* s) :
      win(w), symbol(s)
    {}
  };

  mixsrc_t idx;
  std::list<Line> lines;

  lv_obj_t* label;
  lv_obj_t* line_container;

  static void value_changed(lv_event_t* e);
  
 public:
  InputMixGroup(Window* parent, mixsrc_t idx);

  void addMixerMonitor(uint8_t channel);
    
  mixsrc_t getMixSrc() { return idx; }
  size_t getLineCount() { return lines.size(); }
  
  void addLine(Window* line, const uint8_t* symbol = nullptr);
  bool removeLine(Window* line);
};
