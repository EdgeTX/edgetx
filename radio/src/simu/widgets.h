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

#include <imgui.h>

struct GimbalState {
  ImVec2 pos;
  bool   lock_y;
};

struct ScreenDesc {
  int width;
  int height;
  bool is_dot_matrix;
};

enum class ScreenMouseEventType {
  MouseDown,
  MouseUp,
};

struct ScreenMouseEvent {
  ScreenMouseEventType type;
  int pos_x;
  int pos_y;
};

void GimbalPair(const char* str_id, GimbalState& left, GimbalState& right);
void SimuScreen(const ScreenDesc& desc, ImTextureID screen_img, ImVec2 size,
                ImU32 bg_col, ImU32 overlay_col);
bool SimuScreenMouseEvent(const ScreenDesc& desc, ScreenMouseEvent& event);
