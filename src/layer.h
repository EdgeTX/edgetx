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
extern lv_obj_t * canvas;

class Layer
{
  public:
    explicit Layer(Window * main):
      main(main)
    {
    }

    Window * main;
    Window * focus = nullptr;

    static std::list<Layer> stack;

    static void push(Window * window)
    {
      if (window->getLvObj()->parent != nullptr) {
        TRACE("Layers must have lvgl parent to be null");
      }

      if (!stack.empty()) {
        Layer & parentLayer = stack.back();
        parentLayer.focus = Window::getFocus();
      }

      // layers are screens in lvgl
      lv_obj_set_parent(canvas, window->getLvObj());
      lv_obj_move_background(canvas);
      lv_scr_load(window->getLvObj());
      lv_obj_invalidate(canvas);

      stack.emplace_back(Layer(window));

#if defined(DEBUG_WINDOWS)
      TRACE_WINDOWS("New layer added for %s (%d layers)", window->getWindowDebugString().c_str(), stack.size());
      for (auto & layer: stack) {
        TRACE_WINDOWS(" - %s (focus=%s)", layer.main->getWindowDebugString().c_str(), layer.focus ? layer.focus->getWindowDebugString().c_str() : "---");
      }
#endif
    }

    static void pop(Window * window)
    {
      if (stack.back().main == window) {
        stack.pop_back();
        const auto & back = stack.back();
        lv_obj_set_parent(canvas, back.main->getLvObj());
        lv_obj_move_background(canvas);
        lv_scr_load(back.main->getLvObj());  // load the screen.
        if (back.focus) {
          back.focus->setFocus(SET_FOCUS_DEFAULT);
        }
      }
      else {
        // TODO it would be better to do it in reverse order
        for (auto it = stack.begin(); it != stack.end(); it++) {
          if (it->main == window) {
            lv_obj_set_parent(canvas, it->main->getLvObj());
            lv_scr_load(it->main->getLvObj());
            stack.erase(it);
            break;
          }
        }
      }

#if defined(DEBUG_WINDOWS)
      TRACE_WINDOWS("Layer removed for %s (%d layers)", window->getWindowDebugString().c_str(), stack.size());
      for (auto & layer: stack) {
        TRACE_WINDOWS(" - %s (focus=%s)", layer.main->getWindowDebugString().c_str(), layer.focus ? layer.focus->getWindowDebugString().c_str() : "---");
      }
#endif
    }
};
