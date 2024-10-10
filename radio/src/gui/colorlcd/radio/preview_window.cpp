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
#include "edgetx.h"
#include "sliders.h"
#include "etx_lv_theme.h"
#include "trims.h"
#include "topbar_impl.h"

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
      TextEdit(parent, rect, editText, 0)
  {
    strcpy(editText, text);
    preview(edited, editText, strlen(editText));
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

  auto topbar = new Window(this, {0, 0, LV_PCT(100), TopBar::TOPBAR_ZONE_HEIGHT});
  etx_solid_bg(topbar->getLvObj(), COLOR_THEME_SECONDARY1_INDEX);

  new StaticIcon(topbar, ICON_X1, ICON_Y, ICON_RADIO,
                 COLOR_THEME_PRIMARY2_INDEX);
  new StaticIcon(topbar, ICON_X2, ICON_Y, ICON_RADIO_TOOLS,
                 COLOR_THEME_PRIMARY2_INDEX);
  new StaticIcon(topbar, ICON_X3, ICON_Y, ICON_RADIO_SETUP,
                 COLOR_THEME_PRIMARY2_INDEX);

  new StaticText(this, {ICON_X1, CBT_Y, CBT_W, EdgeTxStyles::PAGE_LINE_HEIGHT}, STR_THEME_CHECKBOX);
  new ThemedCheckBox(this, {CB1_X, CB_Y, CB_W, 0}, true);
  new ThemedCheckBox(this, {CB2_X, CB_Y, CB_W, 0}, false);
  (new ThemedButton(this, {BTN_X, BTN1_Y, BTN_W, 0}, STR_THEME_ACTIVE, true))
      ->check(true);
  new ThemedButton(this, {BTN_X, BTN2_Y, BTN_W, 0}, STR_THEME_REGULAR, false);
  new MainViewTrim(this, {ICON_X1, TRIM_Y, MainViewSlider::HORIZONTAL_SLIDERS_WIDTH, TRIM_H}, 0, false);
  new MainViewSlider(this, {ICON_X1, SLIDER_Y, MainViewSlider::HORIZONTAL_SLIDERS_WIDTH, TRIM_H}, 0, false);
  new StaticText(this, {ICON_X1, TXT1_Y, TXT_W, EdgeTxStyles::PAGE_LINE_HEIGHT}, STR_THEME_WARNING, 
                 COLOR_THEME_WARNING_INDEX);
  new StaticText(this, {ICON_X1, TXT2_Y, TXT_W, EdgeTxStyles::PAGE_LINE_HEIGHT}, STR_THEME_DISABLED, 
                 COLOR_THEME_DISABLED_INDEX);

  new ThemedTextEdit(this, {ICON_X1, EDT_Y, TXT_W, 0}, STR_THEME_EDIT, true);
  new ThemedTextEdit(this, {EDT2_X, EDT_Y, TXT_W, 0}, STR_THEME_FOCUS, false);
  ticks = 0;

  new HeaderDateTime(this, width() - DATE_XO, PAD_SMALL);

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
