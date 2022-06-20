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
#include "color_editor_popup.h"

#define SET_DIRTY() setDirty()

////////////////////////////////////////////////////////////////////////////////////////////////
/////                     ColorEditorContent
////////////////////////////////////////////////////////////////////////////////////////////////
constexpr int GRID_TOP_MARGIN    = 60;
 
constexpr int HUE_BAR_TOP_MARGIN = 35;
constexpr int HUE_BAR_HEIGHT     = 20;
 
constexpr int COLOR_BOX_TOP      = 130;
constexpr int COLOR_BOX_WIDTH    = 90;
constexpr int COLOR_BOX_HEIGHT   = 32;

#if (LCD_W > LCD_H)
constexpr int COLOR_BOX_LEFT     = 240;
constexpr int SCREEN_LEFT_MARGIN = 10;
constexpr rect_t contentRect     = { 50, 50, 360 + 20, 170 };
#else
constexpr int COLOR_BOX_LEFT     = 210;
constexpr rect_t contentRect     = { 5, 50, LCD_W - 10, 170 };
constexpr int SCREEN_LEFT_MARGIN = 0;
#endif

ColorEditorContent::ColorEditorContent(ModalWindow *window, const rect_t rect, uint32_t color, std::function<void (uint32_t rgb)> setValue) :
  ModalWindowContent(window, rect),
  setValue(std::move(setValue))
{
  r = GET_RED(color); g = GET_GREEN(color); b = GET_BLUE(color);
  float h, s, v;
  RGBtoHSV(r, g, b, h, s, v);
  hue = (int) h;
  this->s = 100 * s;
  this->v = 100 * v;

  rect_t rText = {COLOR_BOX_LEFT, 60, 50, 20};
  rect_t rNumber = {COLOR_BOX_LEFT + 60, 60, 40, 20};

  new StaticText(this, rText, STR_RED, 0, COLOR_THEME_PRIMARY1);
  rText.y += 23;
  rEdit = new NumberEdit(this, rNumber, 0, 255, [=] () { return r; }, [=] (int value) { r = value; });
  rNumber.y += 23;
  new StaticText(this, rText, STR_GREEN, 0, COLOR_THEME_PRIMARY1);
  rText.y += 23;
  gEdit = new NumberEdit(this, rNumber, 0, 255, [=] () { return g; }, [=] (int value) { g = value; });
  rNumber.y += 23;
  new StaticText(this, rText, STR_BLUE, 0, COLOR_THEME_PRIMARY1);
  bEdit = new NumberEdit(this, rNumber, 0, 255, [=] () { return b; }, [=] (int value) { b = value; });

  invalidate();
}

void ColorEditorContent::setRGB()
{
  int rgb = HSVtoRGB(hue, s, v);
  r = GET_RED(rgb);
  g = GET_GREEN(rgb);
  b = GET_BLUE(rgb);

  rEdit->setValue(r);
  gEdit->setValue(g);
  bEdit->setValue(b);
  invalidate();

  if (setValue != nullptr)
    setValue(rgb);
}

#if defined(HARDWARE_TOUCH)
bool ColorEditorContent::onTouchEnd(coord_t x, coord_t y)
{
  sliding = false;
  colorPicking = false;
  invalidate();
  return true;
}

bool ColorEditorContent::onTouchStart(coord_t x, coord_t y)
{
  if (!sliding &&
      (y > HUE_BAR_TOP_MARGIN && y < HUE_BAR_TOP_MARGIN + HUE_BAR_HEIGHT)) {
    hue = x - SCREEN_LEFT_MARGIN;
    hue = max(hue, 0);
    hue = min(hue, MAX_HUE);
    sliding = true;

    invalidate();
  } else if (x >= SCREEN_LEFT_MARGIN &&
             x <= SCREEN_LEFT_MARGIN + (MAX_SATURATION * 2) &&
             y >= GRID_TOP_MARGIN && y <= GRID_TOP_MARGIN + MAX_BRIGHTNESS) {
    colorPicking = true;
    v = min(MAX_BRIGHTNESS - (y - GRID_TOP_MARGIN), MAX_BRIGHTNESS);
    s = min((x - SCREEN_LEFT_MARGIN) / 2, MAX_SATURATION);

    setRGB();
  }
  return true;
}
#endif

void ColorEditorContent::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  int oldSliderVal = hue;
  if (event == EVT_ROTARY_LEFT) {
    hue -= 2;
    killAllEvents();
  }
  else if (event == EVT_ROTARY_RIGHT) {
    hue += 2;
    killAllEvents();
  }

  hue = max(hue, 0);
  hue = min(hue, MAX_HUE);
  if (hue != oldSliderVal) {
    invalidate();
  }
#endif
}

void ColorEditorContent::drawHueBar(BitmapBuffer *dc)
{
  for (int i = 0; i < MAX_HUE; i++) {
    auto rgb = HSVtoRGB(i, MAX_SATURATION, MAX_BRIGHTNESS);
    auto r = GET_RED(rgb);
    auto g = GET_GREEN(rgb);
    auto b = GET_BLUE(rgb);
    dc->drawSolidVerticalLine(i + SCREEN_LEFT_MARGIN, HUE_BAR_TOP_MARGIN,
                              HUE_BAR_HEIGHT, COLOR2FLAGS(RGB(r, g, b)));
  }

  dc->drawFilledCircle(hue + SCREEN_LEFT_MARGIN,
                       HUE_BAR_TOP_MARGIN + (HUE_BAR_HEIGHT / 2), 5,
                       COLOR2FLAGS(WHITE));
  if (sliding) {
    dc->drawText(hue + (SCREEN_LEFT_MARGIN / 2), 25,
                 std::to_string(hue).c_str(), FONT(XXS) | COLOR2FLAGS(WHITE));
  }
}

void ColorEditorContent::drawGrid(BitmapBuffer *dc) 
{
  for (int s = 0; s <= MAX_SATURATION; s++) {
    for (int v = MAX_BRIGHTNESS; v >= 0; v--) {
      uint32_t rgbVal = HSVtoRGB(hue, s, v);
      dc->drawPixel((s * 2) + SCREEN_LEFT_MARGIN,
                    ((MAX_BRIGHTNESS - v)) + GRID_TOP_MARGIN, rgbVal);
      dc->drawPixel((s * 2) + SCREEN_LEFT_MARGIN + 1,
                    ((MAX_BRIGHTNESS - v)) + GRID_TOP_MARGIN, rgbVal);
    }
  }

  dc->drawFilledCircle((this->s * 2) + SCREEN_LEFT_MARGIN,
                       (MAX_BRIGHTNESS - this->v) + GRID_TOP_MARGIN, 3,
                       COLOR2FLAGS(WHITE));
}

void ColorEditorContent::drawColorBox(BitmapBuffer *dc)
{
  int32_t rgb = RGB(r, g, b);

  dc->drawSolidFilledRect(COLOR_BOX_LEFT, COLOR_BOX_TOP, COLOR_BOX_WIDTH,
                          COLOR_BOX_HEIGHT, COLOR2FLAGS(rgb));
  dc->drawSolidRect(COLOR_BOX_LEFT, COLOR_BOX_TOP, COLOR_BOX_WIDTH,
                    COLOR_BOX_HEIGHT, 1, COLOR2FLAGS(BLACK));
}

void ColorEditorContent::paint(BitmapBuffer *dc)
{
  ModalWindowContent::paint(dc);
  drawHueBar(dc);
  drawGrid(dc);
  drawColorBox(dc);
}


///////////////////////////////////////////////////////////////////////////
/////                     ColorEditorPopup
///////////////////////////////////////////////////////////////////////////
ColorEditorPopup::ColorEditorPopup(
    Window *window, std::function<uint32_t()> getValue,
    std::function<void(uint32_t value)> setValue) :
    ModalWindow(window, true), _getValue(getValue), _setValue(setValue)
{
  color = _getValue();
  content = new ColorEditorContent(this, contentRect, color, [=](uint32_t rgb) {
    if (_setValue != nullptr) _setValue(rgb);
  });

  bringToTop();
  content->setTitle(STR_COLOR_PICKER);
  killAllEvents();
}

#if defined(HARDWARE_TOUCH)
bool ColorEditorPopup::onTouchEnd(coord_t x, coord_t y)
{
  TRACE("ColorEditorPopup::onTouchEnd");
  if (!Window::onTouchEnd(x, y) && closeWhenClickOutside) {
    onKeyPress();
    deleteLater();
  }
  return true;
}
#endif

void ColorEditorPopup::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;
  Layer::pop(this);
  Window::deleteLater(detach, trash);
}
