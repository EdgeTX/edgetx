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

#ifndef PREVIEW_WINDOW_INCLUDE
#define PREVIEW_WINDOW_INCLUDE
#include <algorithm>

#include "libopenui.h"
#include "opentx.h"
#include "sliders.h"
#include "theme_manager.h"
#include "themes/etx_lv_theme.h"

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
    for (auto i = 0; i < COLOR_COUNT; i++) {
      oldColorVals[i] = lcdColorTable[i];
    }

    for (auto color : colorList) {
      lcdColorTable[color.colorNumber] = color.colorValue;
    }

    usePreviewStyle();
  }

  void restoreColorValues()
  {
    for (auto i = 0; i < COLOR_COUNT; i++) {
      lcdColorTable[i] = oldColorVals[i];
    }

    useMainStyle();
  }

 protected:
  uint32_t oldColorVals[COLOR_COUNT];
  std::vector<ColorEntry> colorList;
};

extern ColorMaintainer colorMaintainer;

// override of controls to draw them using a selected theme
class ThemedStaticText : public StaticText
{
 public:
  ThemedStaticText(FormWindow *window, const rect_t &rect, std::string text,
                   LcdColorIndex colorIndex) :
      StaticText(window, rect, text, 0, COLOR(colorIndex)),
      _colorIndex(colorIndex)
  {
  }

  void paint(BitmapBuffer *dc) override
  {
    colorMaintainer.applyColorValues();
    setTextFlags(COLOR(_colorIndex));
    StaticText::paint(dc);
    colorMaintainer.restoreColorValues();
  }

 protected:
  LcdColorIndex _colorIndex;
};

class ThemedCheckBox : public ToggleSwitch
{
 public:
  ThemedCheckBox(Window *parent, rect_t rect, bool checked) :
      ToggleSwitch(
          parent, rect, [=]() { return checked; },
          [=](uint8_t value) { update(); }, NO_FOCUS),
      checked(checked)
  {
    setFocusHandler([](bool focus) {});
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override { return parent->onEvent(event); }
#endif

 protected:
  bool checked;
};

class ThemedMainViewHorizontalTrim : public MainViewHorizontalTrim
{
 public:
  using MainViewHorizontalTrim::MainViewHorizontalTrim;
  void paint(BitmapBuffer *dc) override
  {
    colorMaintainer.applyColorValues();
    MainViewHorizontalTrim::paint(dc);
    colorMaintainer.restoreColorValues();
  }
};

class ThemedMainViewHorizontalSlider : public MainViewHorizontalSlider
{
 public:
  using MainViewHorizontalSlider::MainViewHorizontalSlider;
  void paint(BitmapBuffer *dc) override
  {
    colorMaintainer.applyColorValues();
    MainViewHorizontalSlider::paint(dc);
    colorMaintainer.restoreColorValues();
  }
};

class ThemedButton : public TextButton
{
 public:
  ThemedButton(FormWindow *window, const rect_t &rect, std::string text,
               bool isChecked, WindowFlags windowFlags) :
      TextButton(window, rect, text, nullptr, windowFlags | NO_FOCUS)
  {
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
      TextEdit(parent, rect, editText, strlen(text), NO_FOCUS)
  {
    strcpy(editText, text);
    lv_obj_add_state(lvobj, LV_STATE_FOCUSED);
    if (edited) lv_obj_add_state(lvobj, LV_STATE_EDITED);
    update();
  }

#if defined(HARDWARE_TOUCH)
  bool onTouchEnd(coord_t x, coord_t y) override { return true; }
#endif

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override { parent->onEvent(event); }
#endif

 protected:
  char editText[50];
};

// display controls using the appropriate theme.
class PreviewWindow : public FormWindow
{
 public:
  PreviewWindow(Window *window, rect_t rect,
                std::vector<ColorEntry> colorList) :
      FormWindow(window, rect, NO_FOCUS)
  {
    setColorList(colorList);
    colorMaintainer.applyColorValues();

    // reset default group to avoid focus
    lv_group_t *def_group = lv_group_get_default();
    lv_group_set_default(nullptr);

    new ThemedStaticText(this, {5, 44, 100, PAGE_LINE_HEIGHT},
                         STR_THEME_CHECKBOX, COLOR_THEME_PRIMARY1_INDEX);
    new ThemedCheckBox(this, {100, 40, 40, 28}, true);
    new ThemedCheckBox(this, {150, 40, 40, 28}, false);
    new ThemedButton(this, {210, 40, 100, 32}, STR_THEME_ACTIVE, true,
                     BUTTON_CHECKED);
    new ThemedButton(this, {210, 75, 100, 32}, STR_THEME_REGULAR, false, 0);
    new ThemedMainViewHorizontalTrim(this,
                                     {5, 75, HORIZONTAL_SLIDERS_WIDTH, 20}, 0);
    new ThemedMainViewHorizontalSlider(
        this, {5, 97, HORIZONTAL_SLIDERS_WIDTH, 20}, 0);
    new ThemedStaticText(this, {5, 122, 100, PAGE_LINE_HEIGHT},
                         STR_THEME_WARNING, COLOR_THEME_WARNING_INDEX);
    new ThemedStaticText(this, {5, 144, 100, PAGE_LINE_HEIGHT},
                         STR_THEME_DISABLED, COLOR_THEME_DISABLED_INDEX);

    new ThemedTextEdit(this, {5, 170, 100, 0}, STR_THEME_EDIT, true);
    new ThemedTextEdit(this, {110, 170, 100, 0}, STR_THEME_FOCUS, false);
    ticks = 0;

    lv_group_set_default(def_group);

    colorMaintainer.restoreColorValues();
  }

  void setColorList(std::vector<ColorEntry> colorList)
  {
    colorMaintainer.setColorList(colorList);
    // Force style update
    colorMaintainer.applyColorValues();
    colorMaintainer.restoreColorValues();
    invalidate();
  }

  BitmapBuffer *getBitmap(const uint8_t *maskData, uint32_t bgColor,
                          uint32_t fgColor, int *width)
  {
    auto mask = BitmapBuffer::load8bitMaskLZ4(maskData);
    BitmapBuffer *newBm =
        new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
    newBm->clear(bgColor);
    newBm->drawMask(0, 0, mask, fgColor);
    delete mask;
    return newBm;
  }

  void drawTime(BitmapBuffer *dc)
  {
    // time on top bar
    struct gtm t;
    char str[16];

    gettime(&t);
    int s =
        snprintf(str, sizeof(str), "%d %s\n", t.tm_mday, STR_MONTHS[t.tm_mon]);
    if (s > 0 && (size_t)s < sizeof(str) - 6 /* 00:00\0 */) {
      getTimerString(str + s, getValue(MIXSRC_TX_TIME));
    }
    dc->drawText(rect.w - 40, 5, str, COLOR_THEME_PRIMARY2 | FONT(XS));
  }

  void paint(BitmapBuffer *dc) override
  {
    colorMaintainer.applyColorValues();

    // background
    dc->clear(COLOR_THEME_SECONDARY3);

    // top bar background
    dc->drawSolidFilledRect(0, 0, rect.w, TOPBAR_ZONE_HEIGHT,
                            COLOR_THEME_SECONDARY1);

    int width;
    int x = 5;
    // topbar icons
    auto mask_menu_radio = getBuiltinIcon(ICON_RADIO);
    auto bm = getBitmap(mask_menu_radio, COLOR_THEME_SECONDARY1,
                        COLOR_THEME_PRIMARY2, &width);
    dc->drawBitmap(x, 5, bm);
    x += MENU_HEADER_BUTTON_WIDTH + 2;
    delete bm;

    dc->drawSolidFilledRect(x - 2, 0, MENU_HEADER_BUTTON_WIDTH + 2,
                            TOPBAR_ZONE_HEIGHT, COLOR_THEME_FOCUS);
    auto mask_radio_tools = getBuiltinIcon(ICON_RADIO_TOOLS);
    bm = getBitmap(mask_radio_tools, COLOR_THEME_FOCUS, COLOR_THEME_PRIMARY2,
                   &width);
    dc->drawBitmap(x, 5, bm);
    x += MENU_HEADER_BUTTON_WIDTH + 2;
    delete bm;

    auto mask_radio_setup = getBuiltinIcon(ICON_RADIO_SETUP);
    bm = getBitmap(mask_radio_setup, COLOR_THEME_SECONDARY1,
                   COLOR_THEME_PRIMARY2, &width);
    dc->drawBitmap(x, 5, bm);
    delete bm;

    drawTime(dc);

    colorMaintainer.restoreColorValues();
  }

 protected:
  tmr10ms_t ticks;
};

#endif
