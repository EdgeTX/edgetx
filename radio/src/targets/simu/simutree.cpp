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

// LVGL UI tree snapshot: serializes the live widget tree as JSON so the
// host can inspect the screen structurally (automated UI tests, tooling).
// See simulib.h for the protocol and the schema description.

#include "simulib.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(COLORLCD)

#include <atomic>
#include <string>

#include <lvgl/lvgl.h>

// EdgeTX window container class (gui/colorlcd/libui/window.cpp)
extern const lv_obj_class_t window_base_class;

namespace {

constexpr uint32_t MAX_DEPTH = 64;
constexpr uint32_t MAX_TABLE_CELLS = 4096;

// Minimal JSON writer. Commas are inserted lazily based on the last
// emitted character, so callers never track "first item" state.
class JsonWriter
{
 public:
  explicit JsonWriter(std::string& out) : out(out) {}

  void beginObject()
  {
    sep();
    out += '{';
  }
  void endObject() { out += '}'; }
  void beginArray()
  {
    sep();
    out += '[';
  }
  void endArray() { out += ']'; }

  // Keys are literal identifiers: no escaping needed.
  void key(const char* k)
  {
    sep();
    out += '"';
    out += k;
    out += "\":";
  }

  void str(const char* s)
  {
    sep();
    out += '"';
    if (s) escape(s);
    out += '"';
  }

  void num(long long v)
  {
    sep();
    out += std::to_string(v);
  }

  void boolean(bool v)
  {
    sep();
    out += v ? "true" : "false";
  }

  void keyStr(const char* k, const char* s)
  {
    key(k);
    str(s);
  }
  void keyNum(const char* k, long long v)
  {
    key(k);
    num(v);
  }
  void keyBool(const char* k, bool v)
  {
    key(k);
    boolean(v);
  }
  // Boolean node attributes are only emitted when true (compact dumps).
  void keyFlag(const char* k, bool v)
  {
    if (v) keyBool(k, true);
  }
  void keyColor(const char* k, lv_color_t c)
  {
    char buf[12];
    snprintf(buf, sizeof(buf), "#%06x",
             (unsigned)(lv_color_to32(c) & 0xFFFFFF));
    keyStr(k, buf);
  }

 private:
  void sep()
  {
    if (out.empty()) return;
    char c = out.back();
    if (c != '{' && c != '[' && c != ':') out += ',';
  }

  void escape(const char* s)
  {
    for (const unsigned char* p = (const unsigned char*)s; *p; p++) {
      unsigned char c = *p;
      switch (c) {
        case '"':
          out += "\\\"";
          break;
        case '\\':
          out += "\\\\";
          break;
        case '\n':
          out += "\\n";
          break;
        case '\r':
          out += "\\r";
          break;
        case '\t':
          out += "\\t";
          break;
        default:
          if (c < 0x20) {
            char buf[8];
            snprintf(buf, sizeof(buf), "\\u%04x", c);
            out += buf;
          } else {
            out += (char)c;
          }
      }
    }
  }

  std::string& out;
};

// Object identity: the lv_obj_t address, stable while the object lives.
long long objId(const lv_obj_t* obj) { return (long long)(uintptr_t)obj; }

struct ClassName {
  const lv_obj_class_t* cls;
  const char* name;
};

// Walking an object's class chain from the most derived class upwards,
// the first entry found here names the node. EdgeTX's own LVGL classes
// are static, so they resolve to the LVGL class they derive from.
const ClassName CLASS_NAMES[] = {
    {&window_base_class, "window"},
#if LV_USE_KEYBOARD
    {&lv_keyboard_class, "keyboard"},
#endif
#if LV_USE_BTNMATRIX
    {&lv_btnmatrix_class, "btnmatrix"},
#endif
#if LV_USE_TEXTAREA
    {&lv_textarea_class, "textarea"},
#endif
#if LV_USE_LABEL
    {&lv_label_class, "label"},
#endif
#if LV_USE_BTN
    {&lv_btn_class, "btn"},
#endif
#if LV_USE_CHECKBOX
    {&lv_checkbox_class, "checkbox"},
#endif
#if LV_USE_SWITCH
    {&lv_switch_class, "switch"},
#endif
#if LV_USE_SLIDER
    {&lv_slider_class, "slider"},
#endif
#if LV_USE_BAR
    {&lv_bar_class, "bar"},
#endif
#if LV_USE_ARC
    {&lv_arc_class, "arc"},
#endif
#if LV_USE_TABLE
    {&lv_table_class, "table"},
#endif
#if LV_USE_CANVAS
    {&lv_canvas_class, "canvas"},
#endif
#if LV_USE_IMG
    {&lv_img_class, "img"},
#endif
#if LV_USE_LINE
    {&lv_line_class, "line"},
#endif
#if LV_USE_TILEVIEW
    {&lv_tileview_class, "tileview"},
    {&lv_tileview_tile_class, "tile"},
#endif
#if LV_USE_DROPDOWN
    {&lv_dropdown_class, "dropdown"},
    {&lv_dropdownlist_class, "dropdownlist"},
#endif
#if LV_USE_ROLLER
    {&lv_roller_class, "roller"},
#endif
    {&lv_obj_class, "obj"},
};

const char* className(const lv_obj_t* obj)
{
  for (const lv_obj_class_t* cls = lv_obj_get_class(obj); cls;
       cls = cls->base_class) {
    for (const auto& e : CLASS_NAMES) {
      if (e.cls == cls) return e.name;
    }
  }
  return "obj";
}

// Widget-specific attributes (text, values, ...).
void emitWidget(JsonWriter& w, lv_obj_t* obj)
{
#if LV_USE_LABEL
  if (lv_obj_has_class(obj, &lv_label_class)) {
    w.keyStr("text", lv_label_get_text(obj));
    return;
  }
#endif

#if LV_USE_TEXTAREA
  if (lv_obj_has_class(obj, &lv_textarea_class)) {
    w.keyStr("text", lv_textarea_get_text(obj));
    w.keyNum("cursor", lv_textarea_get_cursor_pos(obj));
    w.keyFlag("password", lv_textarea_get_password_mode(obj));
    return;
  }
#endif

#if LV_USE_CHECKBOX
  if (lv_obj_has_class(obj, &lv_checkbox_class)) {
    w.keyStr("text", lv_checkbox_get_text(obj));
    return;
  }
#endif

#if LV_USE_KEYBOARD
  if (lv_obj_has_class(obj, &lv_keyboard_class)) {
    lv_obj_t* ta = lv_keyboard_get_textarea(obj);
    if (ta) w.keyNum("textarea", objId(ta));
    w.keyNum("mode", lv_keyboard_get_mode(obj));
    // fall through: a keyboard is a button matrix
  }
#endif

#if LV_USE_BTNMATRIX
  if (lv_obj_has_class(obj, &lv_btnmatrix_class)) {
    // The map is terminated by an empty string; "\n" entries break rows.
    const char** map = lv_btnmatrix_get_map(obj);
    w.key("buttons");
    w.beginArray();
    for (const char** p = map; p && *p && **p; p++) w.str(*p);
    w.endArray();
    uint16_t sel = lv_btnmatrix_get_selected_btn(obj);
    if (sel != LV_BTNMATRIX_BTN_NONE) {
      w.keyNum("selected", sel);
      const char* txt = lv_btnmatrix_get_btn_text(obj, sel);
      if (txt) w.keyStr("selected_text", txt);
    }
    return;
  }
#endif

#if LV_USE_BAR
  // Sliders derive from bars and share the value accessors.
  if (lv_obj_has_class(obj, &lv_bar_class)) {
    w.keyNum("value", lv_bar_get_value(obj));
    w.keyNum("min", lv_bar_get_min_value(obj));
    w.keyNum("max", lv_bar_get_max_value(obj));
    return;
  }
#endif

#if LV_USE_ARC
  if (lv_obj_has_class(obj, &lv_arc_class)) {
    w.keyNum("value", lv_arc_get_value(obj));
    w.keyNum("min", lv_arc_get_min_value(obj));
    w.keyNum("max", lv_arc_get_max_value(obj));
    return;
  }
#endif

#if LV_USE_TABLE
  if (lv_obj_has_class(obj, &lv_table_class)) {
    uint16_t rows = lv_table_get_row_cnt(obj);
    uint16_t cols = lv_table_get_col_cnt(obj);
    w.keyNum("rows", rows);
    w.keyNum("cols", cols);
    uint16_t row, col;
    lv_table_get_selected_cell(obj, &row, &col);
    if (row != LV_TABLE_CELL_NONE && col != LV_TABLE_CELL_NONE) {
      w.keyNum("row", row);
      w.keyNum("col", col);
    }
    uint32_t budget = MAX_TABLE_CELLS;
    w.key("cells");
    w.beginArray();
    for (uint16_t r = 0; r < rows && budget; r++) {
      w.beginArray();
      for (uint16_t c = 0; c < cols && budget; c++, budget--) {
        w.str(lv_table_get_cell_value(obj, r, c));
      }
      w.endArray();
    }
    w.endArray();
    if ((uint32_t)rows * cols > MAX_TABLE_CELLS) w.keyBool("truncated", true);
    return;
  }
#endif

#if LV_USE_IMG
  // Canvases derive from images; their source is pixel data, not a name.
  if (lv_obj_has_class(obj, &lv_img_class)) {
    const void* src = lv_img_get_src(obj);
    if (src) {
      lv_img_src_t type = lv_img_src_get_type(src);
      if (type == LV_IMG_SRC_FILE || type == LV_IMG_SRC_SYMBOL) {
        w.keyStr("src", (const char*)src);
      }
    }
    return;
  }
#endif

#if LV_USE_TILEVIEW
  if (lv_obj_has_class(obj, &lv_tileview_class)) {
    lv_obj_t* tile = lv_tileview_get_tile_act(obj);
    if (tile) w.keyNum("tile", objId(tile));
    return;
  }
#endif
}

// Resolved main-part style of a node (SIMU_UI_TREE_STYLES).
void emitStyle(JsonWriter& w, lv_obj_t* obj)
{
  w.key("style");
  w.beginObject();

  lv_opa_t bg_opa = lv_obj_get_style_bg_opa(obj, LV_PART_MAIN);
  if (bg_opa) {
    w.keyColor("bg", lv_obj_get_style_bg_color(obj, LV_PART_MAIN));
    w.keyNum("bg_opa", bg_opa);
  }
  w.keyColor("text", lv_obj_get_style_text_color(obj, LV_PART_MAIN));

  lv_opa_t opa = lv_obj_get_style_opa(obj, LV_PART_MAIN);
  if (opa != LV_OPA_COVER) w.keyNum("opa", opa);

  lv_coord_t border = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
  if (border) {
    w.keyNum("border", border);
    w.keyColor("border_color",
               lv_obj_get_style_border_color(obj, LV_PART_MAIN));
  }

  lv_coord_t radius = lv_obj_get_style_radius(obj, LV_PART_MAIN);
  if (radius) w.keyNum("radius", radius);

  const lv_font_t* font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
  if (font) w.keyNum("font_h", lv_font_get_line_height(font));

  w.key("pad");
  w.beginArray();
  w.num(lv_obj_get_style_pad_top(obj, LV_PART_MAIN));
  w.num(lv_obj_get_style_pad_right(obj, LV_PART_MAIN));
  w.num(lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN));
  w.num(lv_obj_get_style_pad_left(obj, LV_PART_MAIN));
  w.endArray();

  w.endObject();
}

struct TreeDump {
  JsonWriter& w;
  uint32_t flags;
  lv_coord_t screenW;
  lv_coord_t screenH;

  void node(lv_obj_t* obj, uint32_t depth, const char* layer = nullptr)
  {
    bool hidden = lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
    if (hidden && (flags & SIMU_UI_TREE_SKIP_HIDDEN)) return;

    w.beginObject();
    w.keyNum("id", objId(obj));
    w.keyStr("type", className(obj));
    if (layer) w.keyStr("layer", layer);

    lv_area_t a;
    lv_obj_get_coords(obj, &a);
    w.keyNum("x", a.x1);
    w.keyNum("y", a.y1);
    w.keyNum("w", lv_area_get_width(&a));
    w.keyNum("h", lv_area_get_height(&a));

    // Visibility: not hidden (nor any ancestor), inside the ancestors' clip
    // areas, and intersecting the display. lv_obj_is_visible() alone does not
    // check the display bounds, so parked/off-screen objects (top bar at
    // y=-45, inactive tiles at x=+-480) would count as visible.
    bool onScreen = a.x2 >= 0 && a.y2 >= 0 && a.x1 < screenW && a.y1 < screenH;
    w.keyFlag("visible", onScreen && lv_obj_is_visible(obj));
    w.keyFlag("hidden", hidden);
    w.keyFlag("clickable", lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE));
    w.keyFlag("scrollable", lv_obj_has_flag(obj, LV_OBJ_FLAG_SCROLLABLE));
    w.keyFlag("checkable", lv_obj_has_flag(obj, LV_OBJ_FLAG_CHECKABLE));
    w.keyFlag("floating", lv_obj_has_flag(obj, LV_OBJ_FLAG_FLOATING));

    lv_state_t st = lv_obj_get_state(obj);
    w.keyFlag("focused", st & LV_STATE_FOCUSED);
    w.keyFlag("focus_key", st & LV_STATE_FOCUS_KEY);
    w.keyFlag("edited", st & LV_STATE_EDITED);
    w.keyFlag("checked", st & LV_STATE_CHECKED);
    w.keyFlag("pressed", st & LV_STATE_PRESSED);
    w.keyFlag("disabled", st & LV_STATE_DISABLED);
    w.keyFlag("scrolled", st & LV_STATE_SCROLLED);

    lv_coord_t sx = lv_obj_get_scroll_x(obj);
    lv_coord_t sy = lv_obj_get_scroll_y(obj);
    if (sx || sy) {
      w.key("scroll");
      w.beginObject();
      w.keyNum("x", sx);
      w.keyNum("y", sy);
      w.endObject();
    }

    emitWidget(w, obj);

    if (flags & SIMU_UI_TREE_STYLES) emitStyle(w, obj);

    uint32_t count = lv_obj_get_child_cnt(obj);
    if (count && depth < MAX_DEPTH) {
      w.key("children");
      w.beginArray();
      for (uint32_t i = 0; i < count; i++) {
        node(lv_obj_get_child(obj, i), depth + 1);
      }
      w.endArray();
    }

    w.endObject();
  }
};

void buildTree(std::string& out, uint32_t flags)
{
  out.clear();
  out.reserve(64 * 1024);

  JsonWriter w(out);
  TreeDump dump{w, flags, lv_disp_get_hor_res(nullptr),
                lv_disp_get_ver_res(nullptr)};

  lv_group_t* group = lv_group_get_default();
  lv_obj_t* focused = group ? lv_group_get_focused(group) : nullptr;

  char version[24];
  snprintf(version, sizeof(version), "%d.%d.%d", LVGL_VERSION_MAJOR,
           LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);

  w.beginObject();

  w.key("meta");
  w.beginObject();
  w.keyStr("lvgl", version);
  w.keyNum("tick", lv_tick_get());
  w.keyNum("flags", flags);
  if (focused) w.keyNum("focus", objId(focused));
  if (group && lv_group_get_editing(group)) w.keyBool("editing", true);
  w.endObject();

  // The root is a pseudo node so generic tree walkers need no special case.
  w.keyNum("id", 0);
  w.keyStr("type", "root");
  w.keyNum("x", 0);
  w.keyNum("y", 0);
  w.keyNum("w", lv_disp_get_hor_res(nullptr));
  w.keyNum("h", lv_disp_get_ver_res(nullptr));
  w.keyBool("visible", true);
  w.key("children");
  w.beginArray();
  dump.node(lv_scr_act(), 1, "screen");
  dump.node(lv_layer_top(), 1, "top");
  dump.node(lv_layer_sys(), 1, "sys");
  w.endArray();

  w.endObject();
}

enum TreeState : int { TREE_IDLE = 0, TREE_REQUESTED = 1, TREE_READY = 2 };

std::atomic<int> treeState{TREE_IDLE};
std::atomic<uint32_t> treeFlags{0};
std::string treeScratch;    // GUI task only
std::string treePublished;  // swapped in by the GUI task, read by the host

}  // namespace

// Called from simuGuiHook() (GUI task, LvglWrapper::run): serve a pending
// snapshot request between two LVGL cycles, when no UI code is running.
void simuUiTreePoll()
{
  if (treeState.load(std::memory_order_acquire) != TREE_REQUESTED) return;

  buildTree(treeScratch, treeFlags.load(std::memory_order_relaxed));
  treePublished.swap(treeScratch);
  treeState.store(TREE_READY, std::memory_order_release);
}

bool simuUiTreeRequest(uint32_t flags)
{
  treeFlags.store(flags, std::memory_order_relaxed);
  treeState.store(TREE_REQUESTED, std::memory_order_release);
  return true;
}

bool simuUiTreeReady()
{
  return treeState.load(std::memory_order_acquire) == TREE_READY;
}

const char* simuUiTreeData() { return treePublished.c_str(); }

uint32_t simuUiTreeSize() { return (uint32_t)treePublished.size(); }

#else  // !COLORLCD: B&W radios draw directly into the framebuffer

void simuUiTreePoll() {}
bool simuUiTreeRequest(uint32_t) { return false; }
bool simuUiTreeReady() { return false; }
const char* simuUiTreeData() { return ""; }
uint32_t simuUiTreeSize() { return 0; }

#endif
