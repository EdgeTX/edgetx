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

#include "preview_window.h"

#include "libopenui.h"
#include "opentx.h"
#include "sliders.h"
#include "theme.h"
#include "themes/etx_lv_theme.h"
#include "trims.h"

extern inline tmr10ms_t getTicks() { return g_tmr10ms; }

#define PREVIEW_WINDOW_REFRESH_INTERVAL ((1000 / 10) * 1)

// class to hold a color list and apply / restore it while drawing standard
// controls
class ColorMaintainer
{
 public:
  void setColorList(std::vector<ColorEntry> colorList)
  {
    this->colorList.assign(colorList.begin(), colorList.end());
  }

  void applyColorValues()
  {
    // save old values;
    for (auto i = 0; i < LCD_COLOR_COUNT; i++) {
      oldColorVals[i] = lcdColorTable[i];
    }

    for (auto color : colorList) {
      lcdColorTable[color.colorNumber] = color.colorValue;
    }

    usePreviewStyle();
  }

  void restoreColorValues()
  {
    for (auto i = 0; i < LCD_COLOR_COUNT; i++) {
      lcdColorTable[i] = oldColorVals[i];
    }

    useMainStyle();
  }

 protected:
  uint32_t oldColorVals[LCD_COLOR_COUNT];
  std::vector<ColorEntry> colorList;
};

extern ColorMaintainer colorMaintainer;

class ThemedCheckBox : public ToggleSwitch
{
 public:
  ThemedCheckBox(Window *parent, rect_t rect, bool checked) :
      ToggleSwitch(
          parent, rect, [=]() { return checked; },
          [=](uint8_t value) { update(); }),
      checked(checked)
  {
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    setFocusHandler([](bool focus) {});
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override { return parent->onEvent(event); }
#endif

 protected:
  bool checked;
};

ColorMaintainer colorMaintainer;

class ThemedButton : public TextButton
{
 public:
  ThemedButton(Window *window, const rect_t &rect, std::string text,
               bool isChecked) :
      TextButton(window, rect, text, nullptr)
  {
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    setPressHandler([=]() { return isChecked; });
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override { parent->onEvent(event); }
#endif
};

class ThemedTextEdit : public TextEdit
{
 public:
  ThemedTextEdit(Window *parent, const rect_t &rect, const char *text,
                 bool edited) :
      TextEdit(parent, rect, editText, strlen(text))
  {
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    strcpy(editText, text);
    lv_obj_add_state(lvobj, LV_STATE_FOCUSED);
    if (edited) lv_obj_add_state(lvobj, LV_STATE_EDITED);
    update();
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override { parent->onEvent(event); }
#endif

 protected:
  char editText[50];
};

PreviewWindow::PreviewWindow(Window *window, rect_t rect,
                             std::vector<ColorEntry> colorList) :
    Window(window, rect)
{
  setWindowFlag(NO_FOCUS);

  // reset default group to avoid focus
  lv_group_t *def_group = lv_group_get_default();
  lv_group_set_default(nullptr);

  setColorList(colorList);
  colorMaintainer.applyColorValues();

  etx_solid_bg(lvobj, COLOR_THEME_SECONDARY3_INDEX);

  auto topbar = new Window(this, {0, 0, LV_PCT(100), TOPBAR_ZONE_HEIGHT});
  etx_solid_bg(topbar->getLvObj(), COLOR_THEME_SECONDARY1_INDEX);

  new StaticIcon(topbar, 5, 5, ICON_RADIO,
                 COLOR_THEME_PRIMARY2);
  new StaticIcon(topbar, 38, 5, ICON_RADIO_TOOLS,
                 COLOR_THEME_PRIMARY2);
  new StaticIcon(topbar, 71, 5, ICON_RADIO_SETUP,
                 COLOR_THEME_PRIMARY2);

  new StaticText(this, {5, 44, 100, PAGE_LINE_HEIGHT}, STR_THEME_CHECKBOX);
  new ThemedCheckBox(this, {100, 40, 40, 28}, true);
  new ThemedCheckBox(this, {150, 40, 40, 28}, false);
  (new ThemedButton(this, {210, 40, 100, 32}, STR_THEME_ACTIVE, true))
      ->check(true);
  new ThemedButton(this, {210, 75, 100, 32}, STR_THEME_REGULAR, false);
  new MainViewTrim(this, {5, 75, HORIZONTAL_SLIDERS_WIDTH, 20}, 0, false);
  new MainViewSlider(this, {5, 97, HORIZONTAL_SLIDERS_WIDTH, 20}, 0, false);
  new StaticText(this, {5, 122, 100, PAGE_LINE_HEIGHT}, STR_THEME_WARNING, 
                 COLOR_THEME_WARNING);
  new StaticText(this, {5, 144, 100, PAGE_LINE_HEIGHT}, STR_THEME_DISABLED, 
                 COLOR_THEME_DISABLED);

  new ThemedTextEdit(this, {5, 170, 100, 0}, STR_THEME_EDIT, true);
  new ThemedTextEdit(this, {110, 170, 100, 0}, STR_THEME_FOCUS, false);
  ticks = 0;

  dateTime = new HeaderDateTime(this->getLvObj(), width() - 42, 4);

  lv_group_set_default(def_group);

  colorMaintainer.restoreColorValues();
}

void PreviewWindow::setColorList(std::vector<ColorEntry> colorList)
{
  colorMaintainer.setColorList(colorList);
  // Force style update
  colorMaintainer.applyColorValues();
  colorMaintainer.restoreColorValues();
  // Update preview colors
  invalidate();
}

void PreviewWindow::checkEvents() { dateTime->update(); }
