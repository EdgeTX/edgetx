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
#include "themes/480_bitmaps.h"

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
  }

  void restoreColorValues()
  {
    for (auto i = 0; i < COLOR_COUNT; i++) {
      lcdColorTable[i] = oldColorVals[i];
    }
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
  ThemedStaticText(FormGroup *window, const rect_t &rect, std::string text, LcdColorIndex colorIndex) :
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

class ThemedCheckBox : public CheckBox
{
 public:
  ThemedCheckBox(Window *parent, rect_t rect, bool checked) :
      CheckBox(parent, rect, [=]() { return checked; }, [](uint8_t value) {}, NO_FOCUS),
      checked(checked)
  {
    enable(false);
    setFocusHandler([] (bool focus) {
    });
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override
  {
    return parent->onEvent(event);
  }
#endif

  void paint(BitmapBuffer *dc) override
  {
    colorMaintainer.applyColorValues();
    CheckBox::paint(dc);
    colorMaintainer.restoreColorValues();
  }

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
    ThemedButton(FormGroup *window, const rect_t &rect, std::string text, bool isChecked, WindowFlags windowFlags, 
                 LcdColorIndex colorIndex) :
      TextButton(window, rect, text, nullptr, windowFlags | NO_FOCUS, COLOR(colorIndex)),
      _colorIndex(colorIndex),
      _isChecked(isChecked)
    {
      setPressHandler([=] () { return _isChecked; });
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override
    {
      parent->onEvent(event);
    }
#endif

    void paint(BitmapBuffer *dc) override
    {
      colorMaintainer.applyColorValues();
      setTextFlags(COLOR(_colorIndex));
      TextButton::paint(dc);
      colorMaintainer.restoreColorValues();
    }
  protected:
    LcdColorIndex _colorIndex;
    bool _isChecked = true;
};

class ThemedTextEdit : public TextEdit
{
  public:
    ThemedTextEdit(Window *parent, const rect_t &rect, char *text, 
                   int colorBackgroundIndex, int colorTextIndex) :
      TextEdit(parent, rect, text, strlen(text)),
      _colorBackgroundIndex(colorBackgroundIndex),
      _colorTextIndex(colorTextIndex)
    {
      setBackgroundHandler([=] (FormField *field) {
        return COLOR(_colorBackgroundIndex);
      });
    }

    void paint(BitmapBuffer *dc) override
    {
      colorMaintainer.applyColorValues();
      FormField::paint(dc);

      dc->drawSizedText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, length, COLOR(_colorTextIndex));
      colorMaintainer.restoreColorValues();
    }

#if defined(SOFTWARE_KEYBOARD)
    bool onTouchEnd(coord_t x, coord_t y) override
    {
      return true;
    }
#endif


#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override
    {
      parent->onEvent(event);
    }
#endif


  protected:
    int _colorBackgroundIndex;
    int _colorTextIndex;
};


// display controls using the appropriate theme.
class PreviewWindow : public FormGroup
{
 public:
  PreviewWindow(Window *window, rect_t rect,
                std::vector<ColorEntry> colorList) :
      FormGroup(window, rect, NO_FOCUS), _colorList(colorList)
  {
    new ThemedStaticText(this, {5, 40, 100, LINE_HEIGHT}, "Checkbox", COLOR_THEME_PRIMARY1_INDEX);
    new ThemedCheckBox(this, {100 + 15, 40, 20, LINE_HEIGHT}, true);
    new ThemedCheckBox(this, {140 + 15, 40, 20, LINE_HEIGHT}, false);
    new ThemedButton(this, {190, 40, 100, LINE_HEIGHT + 10}, "Active", true, BUTTON_CHECKED, COLOR_THEME_PRIMARY1_INDEX);
    new ThemedButton(this, {190, 75, 100, LINE_HEIGHT + 10}, "Regular", false, 0, COLOR_THEME_PRIMARY1_INDEX);
    new ThemedMainViewHorizontalTrim(this, {5, 65, HORIZONTAL_SLIDERS_WIDTH, 20}, 0);
    new ThemedMainViewHorizontalSlider(this, {5, 87, HORIZONTAL_SLIDERS_WIDTH, 20}, 0);
    new ThemedStaticText(this, {5, 115, 100, LINE_HEIGHT}, "Warning Text", COLOR_THEME_WARNING_INDEX);
    new ThemedStaticText(this, {5, 140, 100, LINE_HEIGHT}, "Disabled Text", COLOR_THEME_DISABLED_INDEX);

    static char EditText[128];
    strcpy(EditText, "Edit");
    new ThemedTextEdit(this, {5, 160, 100, LINE_HEIGHT + 1}, EditText, 
                       COLOR_THEME_EDIT_INDEX, COLOR_THEME_PRIMARY2_INDEX);
    static char FocusText[128];
    strcpy(FocusText, "Focus");
    new ThemedTextEdit(this, {110, 160, 100, LINE_HEIGHT + 1}, FocusText, 
                       COLOR_THEME_FOCUS_INDEX, COLOR_THEME_PRIMARY2_INDEX);
    ticks = getTicks();
  }

  void setColorList(std::vector<ColorEntry> colorList)
  {
    _colorList = colorList;
  }

  void checkEvents() override
  {
    Window::checkEvents();

    tmr10ms_t newTicks = getTicks();
    if (newTicks - ticks > PREVIEW_WINDOW_REFRESH_INTERVAL) {
      invalidate();
      ticks = newTicks;
    }
  }

  BitmapBuffer *getBitmap(const uint8_t *maskData, uint32_t bgColor,
                          uint32_t fgColor, int *width)
  {
    auto mask = BitmapBuffer::load8bitMask(maskData);
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
    const char *const STR_MONTHS[] = TR_MONTHS;

    gettime(&t);
    int s = snprintf(str, sizeof(str), "%d %s\n", t.tm_mday, STR_MONTHS[t.tm_mon]);
    if (s > 0 && s < sizeof(str) - 6 /* 00:00\0 */) {
      getTimerString(str + s, getValue(MIXSRC_TX_TIME));
    }
    dc->drawText(rect.w - 40, 5, str, COLOR_THEME_PRIMARY2 | FONT(XS));
  }

  void paint(BitmapBuffer *dc) override
  {
    colorMaintainer.setColorList(_colorList);
    colorMaintainer.applyColorValues();

    // background
    dc->clear(COLOR_THEME_SECONDARY3);

    // top bar background
    dc->drawSolidFilledRect(0, 0, rect.w, TOPBAR_HEIGHT, COLOR_THEME_SECONDARY1);

    int width;
    int x = 5;
    // topbar icons
    auto bm = getBitmap(mask_menu_radio, COLOR_THEME_SECONDARY1, COLOR_THEME_PRIMARY2, &width);
    dc->drawBitmap(x, 5, bm);
    x += MENU_HEADER_BUTTON_WIDTH + 2;
    delete bm;

    dc->drawSolidFilledRect(x - 2, 0, MENU_HEADER_BUTTON_WIDTH + 2, TOPBAR_HEIGHT, COLOR_THEME_FOCUS);
    bm = getBitmap(mask_radio_tools, COLOR_THEME_FOCUS, COLOR_THEME_PRIMARY2, &width);
    dc->drawBitmap(x, 5, bm);
    x += MENU_HEADER_BUTTON_WIDTH + 2;
    delete bm;

    bm = getBitmap(mask_radio_setup, COLOR_THEME_SECONDARY1, COLOR_THEME_PRIMARY2, &width);
    dc->drawBitmap(x, 5, bm);
    delete bm;

    drawTime(dc);

    colorMaintainer.restoreColorValues();
  }

 protected:
  std::vector<ColorEntry> _colorList;
  CheckBox *checkBox;
  tmr10ms_t ticks;
};

#endif
