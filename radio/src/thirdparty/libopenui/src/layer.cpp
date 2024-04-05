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

#include "layer.h"

std::list<Layer> Layer::stack;

Layer::Layer(Window* w, lv_group_t* g) : window(w), group(g) {}
Layer::~Layer() { lv_group_del(group); }

static void _assign_lv_group(lv_group_t* g)
{
  lv_group_set_default(g);

  // associate it with all input devices
  lv_indev_t* indev = lv_indev_get_next(NULL);
  while (indev) {
    lv_indev_set_group(indev, g);
    indev = lv_indev_get_next(indev);
  }
}

void Layer::push(Window* w)
{
  // create a new group
  auto g = lv_group_create();
  _assign_lv_group(g);
  
  // and store
  stack.emplace_back(w, g);
}

void Layer::pop(Window* w)
{
  if (stack.empty()) return;

  if (back() == w) {
    stack.pop_back();
  } else {
    for (auto layer = stack.crbegin(); layer != stack.crend(); layer++) {
      if (layer->window == w) {
        stack.erase(layer.base());
        return;
      }
    }
  }

  if (!stack.empty()) {
    lv_group_t* g = stack.back().group;
    _assign_lv_group(g);
  } else {
    lv_group_set_default(NULL);
  }
}

Window* Layer::back()
{
  if (stack.empty()) return nullptr;
  return stack.back().window;
}

Window* Layer::getFirstOpaque()
{
  for (auto layer = stack.crbegin(); layer != stack.crend(); layer++) {
    auto w = layer->window;
    if (w && w->hasWindowFlag(OPAQUE)) return w;
  }

  return nullptr;
}
