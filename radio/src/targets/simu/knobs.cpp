//
// Code from:
//   https://github.com/altschuler/imgui-knobs
//
// Copyright (c) 2022 Simon Altschuler
// 
// MIT License
//

#include "knobs.h"

#include <cmath>
#include <cstdlib>
#include <imgui.h>
#include <imgui_internal.h>

#define IMGUIKNOBS_PI 3.14159265358979323846f

namespace ImGuiKnobs
{
namespace detail
{

template <typename DataType>
struct knob {
  float radius;
  bool value_changed;
  ImVec2 center;
  bool is_active;
  bool is_hovered;
  float angle_min;
  float angle_max;
  float t;
  float angle;
  float angle_cos;
  float angle_sin;

  knob(const char *_label, ImGuiDataType data_type, DataType *p_value,
       DataType v_min, DataType v_max, float _radius,
       const char *format, ImGuiKnobFlags flags)
  {
    radius = _radius;
    t = ((float)*p_value - v_min) / (v_max - v_min);
    auto screen_pos = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton(_label, {radius * 2.0f, radius * 2.0f});

    angle_min = IMGUIKNOBS_PI * 0.75f;
    angle_max = IMGUIKNOBS_PI * 2.25f;
    center = {screen_pos.x + radius, screen_pos.y + radius};
    is_active = ImGui::IsItemActive();
    is_hovered = ImGui::IsItemHovered();
    angle = angle_min + (angle_max - angle_min) * t;
    angle_cos = cosf(angle);
    angle_sin = sinf(angle);

    if (is_active) {
      ImVec2 mp = ImGui::GetIO().MousePos;
      float angle_mid = (angle_max + angle_min) / 2.0;
      float alpha = atan2f(mp.x - center.x, center.y - mp.y) + angle_mid;
      alpha = ImClamp(alpha, angle_min, angle_max);
      float ratio = (alpha - angle_min) / (angle_max - angle_min);
      switch(data_type){
      case ImGuiDataType_Float:
        *p_value = ImGui::ScaleValueFromRatioT<ImS32, ImS32, float>(
            data_type, ratio, v_min, v_max, false, 0.0f, 0.0f);
        break;
      case ImGuiDataType_S32:
        *p_value = ImGui::ScaleValueFromRatioT<float, float, float>(
            data_type, ratio, v_min, v_max, false, 0.0f, 0.0f);
        break;
      default: break;
      }
    }
  }

  void draw_dot(float size, float radius, float angle, color_set color,
                bool filled, int segments)
  {
    auto dot_size = size * this->radius;
    auto dot_radius = radius * this->radius;

    ImGui::GetWindowDrawList()->AddCircleFilled(
        {center[0] + cosf(angle) * dot_radius,
         center[1] + sinf(angle) * dot_radius},
        dot_size,
        is_active ? color.active : (is_hovered ? color.hovered : color.base),
        segments);
  }

  void draw_tick(float start, float end, float width, float angle,
                 color_set color)
  {
    auto tick_start = start * radius;
    auto tick_end = end * radius;
    auto angle_cos = cosf(angle);
    auto angle_sin = sinf(angle);

    ImGui::GetWindowDrawList()->AddLine(
        {center[0] + angle_cos * tick_end, center[1] + angle_sin * tick_end},
        {center[0] + angle_cos * tick_start,
         center[1] + angle_sin * tick_start},
        is_active ? color.active : (is_hovered ? color.hovered : color.base),
        width * radius);
  }

  void draw_circle(float size, color_set color, bool filled, int segments)
  {
    auto circle_radius = size * radius;

    ImGui::GetWindowDrawList()->AddCircleFilled(
        center, circle_radius,
        is_active ? color.active : (is_hovered ? color.hovered : color.base));
  }
};

template <typename DataType>
knob<DataType> knob_with_title(const char *label, ImGuiDataType data_type,
                               DataType *p_value, DataType v_min, DataType v_max,
                               const char *format, float size,
                               ImGuiKnobFlags flags)
{
  ImGui::PushID(label);
  auto width = size == 0 ? ImGui::GetTextLineHeight() * 4.0f
                         : size * ImGui::GetIO().FontGlobalScale;
  ImGui::PushItemWidth(width);

  ImGui::BeginGroup();

  // There's an issue with `SameLine` and Groups, see
  // https://github.com/ocornut/imgui/issues/4190. This is probably not the best
  // solution, but seems to work for now
  ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset = 0;

  // Draw title
  if (!(flags & ImGuiKnobFlags_NoTitle)) {
    auto title_size = ImGui::CalcTextSize(label, NULL, false, width);

    // Center title
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         (width - title_size[0]) * 0.5f);

    ImGui::Text("%s", label);
  }

  // Draw knob
  knob<DataType> k(label, data_type, p_value, v_min, v_max, width * 0.5f,
                   format, flags);

  // Draw tooltip
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) ||
      ImGui::IsItemActive()) {

    if (flags & ImGuiKnobFlags_ValueTooltip && ImGui::IsItemActive()) {
      ImGui::SetTooltip(format, *p_value);
    } else if (flags & ImGuiKnobFlags_TitleTooltip) {
      ImGui::SetTooltip("%s", label);
    }
  }

  ImGui::EndGroup();
  ImGui::PopItemWidth();
  ImGui::PopID();

  return k;
}

color_set GetPrimaryColorSet()
{
  auto *colors = ImGui::GetStyle().Colors;

  return {colors[ImGuiCol_ButtonActive], colors[ImGuiCol_ButtonHovered],
          colors[ImGuiCol_ButtonHovered]};
}

color_set GetSecondaryColorSet()
{
  auto *colors = ImGui::GetStyle().Colors;
  auto active = ImVec4(colors[ImGuiCol_ButtonActive].x * 0.5f,
                       colors[ImGuiCol_ButtonActive].y * 0.5f,
                       colors[ImGuiCol_ButtonActive].z * 0.5f,
                       colors[ImGuiCol_ButtonActive].w);

  auto hovered = ImVec4(colors[ImGuiCol_ButtonHovered].x * 0.5f,
                        colors[ImGuiCol_ButtonHovered].y * 0.5f,
                        colors[ImGuiCol_ButtonHovered].z * 0.5f,
                        colors[ImGuiCol_ButtonHovered].w);

  return {active, hovered, hovered};
}

color_set GetTrackColorSet()
{
  auto *colors = ImGui::GetStyle().Colors;

  return {colors[ImGuiCol_FrameBg], colors[ImGuiCol_FrameBg],
          colors[ImGuiCol_FrameBg]};
}
}  // namespace detail

template <typename DataType>
bool BaseKnob(const char *label, ImGuiDataType data_type, DataType *p_value,
              DataType v_min, DataType v_max, const char *format,
              ImGuiKnobVariant variant, float size, ImGuiKnobFlags flags,
              int steps = 10)
{
  auto knob = detail::knob_with_title(label, data_type, p_value, v_min, v_max,
                                      format, size, flags);

  switch (variant) {
    case ImGuiKnobVariant_Tick: {
      knob.draw_circle(0.85f, detail::GetSecondaryColorSet(), true, 32);
      knob.draw_tick(0.5f, 0.85f, 0.1f, knob.angle,
                     detail::GetPrimaryColorSet());
      break;
    }
    case ImGuiKnobVariant_Dot: {
      knob.draw_circle(0.85f, detail::GetSecondaryColorSet(), true, 32);
      knob.draw_dot(0.12f, 0.6f, knob.angle, detail::GetPrimaryColorSet(), true,
                    12);
      break;
    }

    case ImGuiKnobVariant_Stepped: {
      for (auto n = 0.f; n < steps; n++) {
        auto a = n / (steps - 1);
        auto angle = knob.angle_min + (knob.angle_max - knob.angle_min) * a;
        knob.draw_tick(0.85f, 1.0f, 0.04f, angle, detail::GetPrimaryColorSet());
      }

      knob.draw_circle(0.75f, detail::GetSecondaryColorSet(), true, 32);
      knob.draw_dot(0.12f, 0.4f, knob.angle, detail::GetPrimaryColorSet(), true,
                    12);
      break;
    }
  }

  return knob.value_changed;
}

bool Knob(const char *label, float *p_value, float v_min, float v_max,
          float speed, const char *format, ImGuiKnobVariant variant, float size,
          ImGuiKnobFlags flags, int steps)
{
  const char *_format = format == NULL ? "%.3f" : format;
  return BaseKnob(label, ImGuiDataType_Float, p_value, v_min, v_max,
                  _format, variant, size, flags, steps);
}

bool KnobInt(const char *label, int *p_value, int v_min, int v_max, float speed,
             const char *format, ImGuiKnobVariant variant, float size,
             ImGuiKnobFlags flags, int steps)
{
  const char *_format = format == NULL ? "%i" : format;
  return BaseKnob(label, ImGuiDataType_S32, p_value, v_min, v_max,
                  _format, variant, size, flags, steps);
}

}  // namespace ImGuiKnobs
