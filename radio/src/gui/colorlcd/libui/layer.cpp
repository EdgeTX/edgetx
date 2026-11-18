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

Layer::Layer(Window* w, lv_group_t* g, lv_group_t* pg) : window(w), group(g), prevGroup(pg) {}
Layer::~Layer() { lv_group_del(group); }

void Layer::push(Window* w)
{
  // save prev group
  auto pg = lv_group_get_default();

  // create a new group
  auto g = lv_group_create();
  w->assignLvGroup(g, true);
  
  // and store
  stack.emplace_back(w, g, pg);
}

void Layer::pop(Window* w)
{
  if (stack.empty()) return;

  if (back() == w) {
    lv_group_t* prevGroup = stack.back().prevGroup;
    if (prevGroup) {
      w->assignLvGroup(prevGroup, true);
    } else if (!stack.empty()) {
      w->assignLvGroup(stack.back().group, true);
    } else {
      lv_group_set_default(NULL);
    }
    stack.pop_back();
  } else {
    for (auto layer = stack.crbegin(); layer != stack.crend(); layer++) {
      if (layer->window == w) {
        stack.erase(layer.base());
        return;
      }
    }
    return;
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

Window* Layer::walk(std::function<bool(Window* w)> check)
{
  for (auto layer = stack.crbegin(); layer != stack.crend(); layer++) {
    if (layer->window && check(layer->window))
      return layer->window;
  }

  return nullptr;
}

Window* Layer::getPageGroup()
{
  Window* w = Layer::walk([=](Window *w) mutable -> bool {
    return w->isPageGroup();
  });
  return w;
}

void Layer::resetGroup(Window* window)
{
  for (auto layer = stack.crbegin(); layer != stack.crend(); layer++) {
    if (layer->window == window) {
      window->assignLvGroup(layer->group, true);
    }
  }
}
