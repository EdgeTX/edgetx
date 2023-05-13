/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

class Layer
{
  static std::list<Layer> stack;

  Window*     window;
  lv_group_t* group;

 public:
  explicit Layer(Window* w, lv_group_t* g);
  ~Layer();

  static void push(Window* window);
  static void pop(Window* window);

  static Window* back();
  static Window* getFirstOpaque();
};
