/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
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
