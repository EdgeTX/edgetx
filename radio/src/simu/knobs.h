//
// Code from:
//   https://github.com/altschuler/imgui-knobs
//
// Copyright (c) 2022 Simon Altschuler
// 
// MIT License
//

#include <imgui.h>

typedef int ImGuiKnobFlags;

enum ImGuiKnobFlags_ {
    ImGuiKnobFlags_NoTitle = 1 << 0,
    ImGuiKnobFlags_ValueTooltip = 1 << 1,
    ImGuiKnobFlags_TitleTooltip = 1 << 2,
};

typedef int ImGuiKnobVariant;

enum ImGuiKnobVariant_ {
    ImGuiKnobVariant_Tick = 1,
    ImGuiKnobVariant_Dot,
    ImGuiKnobVariant_Stepped,
};

namespace ImGuiKnobs {

    struct color_set {
        ImColor base;
        ImColor hovered;
        ImColor active;

        color_set(ImColor base, ImColor hovered, ImColor active) :
            base(base), hovered(hovered), active(active)
        {
        }

        color_set(ImColor color) {
            base = color;
            hovered = color;
            active = color;
        }
    };

    bool Knob(const char* label, float* p_value, float v_min, float v_max,
              float speed = 0, const char* format = NULL,
              ImGuiKnobVariant variant = ImGuiKnobVariant_Tick, float size = 0,
              ImGuiKnobFlags flags = 0, int steps = 10);

    bool KnobInt(const char* label, int* p_value, int v_min, int v_max,
                 float speed = 0, const char* format = NULL,
                 ImGuiKnobVariant variant = ImGuiKnobVariant_Tick,
                 float size = 0, ImGuiKnobFlags flags = 0, int steps = 10);

}  // namespace ImGuiKnobs
