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

#include "widgets.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>

#define _PI_ 3.14159265358979323846f

static inline float _clamp(float f)
{
  return (f < 0.0) ? 0.0 : (f > 1.0) ? 1.0 : f;
}

static inline void adjust_gimbal(float& pos)
{
  if (pos != 0.5f) {
    auto diff = 0.5 - pos;
    if (std::abs(diff) > 0.01) {
      pos += diff / 10;
    } else {
      pos = 0.5;
    }
  }
}

static void draw_tick(const ImVec2& center, ImU32 col, float angle,
                      float start, float end, float thickness)
{
  auto angle_cos = std::cos(angle);
  auto angle_sin = std::sin(angle);

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddLine({center.x + angle_cos * end, center.y + angle_sin * end},
                     {center.x + angle_cos * start, center.y + angle_sin * start},
                     col, thickness);
}

static void draw_gimbal_frame(const ImVec2& p0, const ImVec2& p1, float rounding, float thickness)
{
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  auto bg_col = ImGui::GetColorU32(ImGuiCol_FrameBg);
  draw_list->AddRectFilled(p0, p1, bg_col, rounding,
                           ImDrawFlags_RoundCornersAll);

  auto frame_col = ImGui::GetColorU32(ImGuiCol_Border);
  draw_list->AddRect(p0, p1, frame_col, rounding,
                     ImDrawFlags_RoundCornersAll, thickness);

  float gimbal_mid = (p1.x - p0.x) / 2.0;
  float circle_radius = gimbal_mid * 0.75;
  ImVec2 center = {p0.x + gimbal_mid, p0.y + gimbal_mid};

  float circle_thickness = thickness * 1.8f;
  float inner_radius = circle_radius - circle_thickness / 2.0;

  draw_list->AddCircle(center, circle_radius, frame_col, 0, thickness * 1.8f);

  for (float deg = 0.0f; deg < 360.0f; deg += 10.0f) {
    float angle = _PI_ * deg / 180.0f;
    draw_tick(center, IM_COL32_WHITE, angle,
              circle_radius - circle_thickness * 0.3f,
              circle_radius + circle_thickness * 0.3f,
              circle_thickness * 0.2f);
  }

  for (float deg = 0.0f; deg < 360.0f; deg += 90.0f) {
    float angle = _PI_ * deg / 180.0f;
    draw_tick(center, frame_col, angle,
              inner_radius, circle_thickness, circle_thickness * 0.2f);
  }
}

void SingleGimbal(const char* name, GimbalState& gs)
{
  const auto gimbal_width = 120.0f;
  const auto border_rounding = 12.0f;
  const auto border_thickness = 6.0f;
  const auto default_dot_radius = 12.0f;

  ImGui::InvisibleButton(name, ImVec2(gimbal_width, gimbal_width));

  auto p0 = ImGui::GetItemRectMin();
  p0.x += 6.0f;
  p0.y += 6.0f;

  auto p1 = ImGui::GetItemRectMax();
  p1.x -= 6.0f;
  p1.y -= 6.0f;

  draw_gimbal_frame(p0, p1, border_rounding, border_thickness);
  
  float dot_radius = default_dot_radius;
  ImU32 col = ImGui::GetColorU32(ImGuiCol_Button);
  if (ImGui::IsItemActive()) {
    dot_radius += 1.5;
    col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
  } else if (ImGui::IsItemHovered()) {
    col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
  }

  float eff_width = gimbal_width - 12.0f - 2.0 * default_dot_radius;
  float half_width = (gimbal_width - 12.0f) / 2.0;

  if (ImGui::IsItemActive()) {
    const auto& io = ImGui::GetIO();
    ImVec2 mouse_pos(io.MousePos.x - p0.x, io.MousePos.y - p0.y);
    gs.pos.x = _clamp((mouse_pos.x - half_width) / eff_width + 0.5);
    gs.pos.y = _clamp((mouse_pos.y - half_width) / eff_width + 0.5);
  } else {
    adjust_gimbal(gs.pos.x);
    if (!gs.lock_y) adjust_gimbal(gs.pos.y);
  }

  ImVec2 dot_center(p0.x + eff_width * (gs.pos.x - 0.5) + half_width,
                    p0.y + eff_width * (gs.pos.y - 0.5) + half_width);

  // Make color opaque
  col |= IM_COL32_BLACK;
  ImGui::GetWindowDrawList()->AddCircle(dot_center, dot_radius, col, 0, dot_radius - 1.0);
}

void GimbalPair(const char* str_id, GimbalState& left, GimbalState& right)
{
  ImGui::PushID(str_id);
  ImGui::BeginGroup();

  SingleGimbal("#g-left", left);
  ImGui::SameLine();
  SingleGimbal("#g-right", right);

  ImGui::EndGroup();
  ImGui::PopID();
}

void SimuScreen(const ScreenDesc& desc, ImTextureID screen_img, ImVec2 size,
                ImU32 bg_col, ImU32 overlay_col)
{
  ImGui::InvisibleButton("#simu-screen", size);
  if (!ImGui::IsItemVisible()) return;

  ImVec2 p_min = ImGui::GetItemRectMin();
  ImVec2 p_max = ImGui::GetItemRectMax();

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  if (bg_col != IM_COL32_BLACK_TRANS) {
    draw_list->AddRectFilled(p_min, p_max, bg_col);
  }
  draw_list->AddImage(screen_img, p_min, p_max);

  if (desc.is_dot_matrix) {
    float dx = size.x / float(desc.width);
    float thickness = dx / 50.0;
    auto col = bg_col & 0x80FFFFFF;
    for (int x = 1; x < desc.width; x++) {
      ImVec2 p1 = { p_min.x + x * size.x / float(desc.width), p_min.y };
      ImVec2 p2 = { p1.x, p_min.y + size.y - 1 };
      draw_list->AddLine(p1, p2, col, thickness);
    }
    for (int y = 1; y < desc.height; y++) {
      ImVec2 p1 = { p_min.x, p_min.y + y * size.y / float(desc.height) };
      ImVec2 p2 = { p_min.x + size.x - 1, p1.y };
      draw_list->AddLine(p1, p2, col, thickness);
    }
  }

  if (overlay_col != IM_COL32_BLACK_TRANS) {
    draw_list->AddRectFilled(p_min, p_max, overlay_col);
  }
}

bool SimuScreenMouseEvent(const ScreenDesc& desc, ScreenMouseEvent& event)
{
  bool active = ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
  bool deactivated = ImGui::IsItemDeactivated() && ImGui::IsMouseReleased(ImGuiMouseButton_Left);

  if (active || deactivated) {

    auto size = ImGui::GetItemRectSize();
    float scale_x = float(desc.width) / size.x;
    float scale_y = float(desc.height) / size.y;
    
    ImGuiIO& io = ImGui::GetIO();
    auto p0 = ImGui::GetItemRectMin();

    const ImVec2 mouse_pos(io.MousePos.x - p0.x, io.MousePos.y - p0.y);
    const ImVec2 scaled_pos(mouse_pos.x * scale_x, mouse_pos.y * scale_y);

    if (active) {
      event.type = ScreenMouseEventType::MouseDown;
    } else {
      event.type = ScreenMouseEventType::MouseUp;
    }
 
    event.pos_x = (int)std::round(scaled_pos.x);
    event.pos_y = (int)std::round(scaled_pos.y);

    return true;
  }

  return false;
}
