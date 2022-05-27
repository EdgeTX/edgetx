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
#include "color_editor.h"

constexpr int BAR_MARGIN = 5;

constexpr int BAR_TOP_MARGIN = 5;
constexpr int BAR_HEIGHT_OFFSET = BAR_TOP_MARGIN + 25;

const char *RGBChars[MAX_BARS] = { "R", "G", "B" };
const char *HSVChars[MAX_BARS] = { "H", "S", "V" };

// ColorTypes()
// A ColorType implements a three bar color editor.  Currently we support HSV and RGB
enum HSV_BAR_TYPE
{
  HUE = 0,
  SATURATION = 1,
  BRIGHTNESS = 2
};


HSVColorType::HSVColorType(FormGroup *window, uint32_t color) :
  ColorType(window->height() - BAR_HEIGHT_OFFSET)
{
  auto r = GET_RED(color), g = GET_GREEN(color), b = GET_BLUE(color);
  float values[MAX_BARS];
  RGBtoHSV(r, g, b, values[0], values[1], values[2]);
  values[1] *= MAX_SATURATION; // convert the proper base
  values[2] *= MAX_BRIGHTNESS;

  auto spacePerBar = (window->width() / MAX_BARS);

  int leftPos = 0;

  for (auto i = 0; i < MAX_BARS; i++) {
    barInfo[i].leftPos = leftPos + BAR_MARGIN;
    barInfo[i].barWidth = spacePerBar - BAR_MARGIN - 5;
    barInfo[i].maxValue = i == 0 ? MAX_HUE : MAX_BRIGHTNESS;
    barInfo[i].invert = i != 0;
    barInfo[i].sliding = false;
    barInfo[i].value = values[i];

    leftPos += spacePerBar;
  }
}

uint32_t HSVColorType::getRGB()
{
  return HSVtoRGB(barInfo[0].value, barInfo[1].value, barInfo[2].value);
}

void HSVColorType::drawBarValue(BitmapBuffer *dc, int bar)
{
  dc->drawText(barInfo[bar].leftPos, BAR_TOP_MARGIN + screenHeight + 9, HSVChars[bar], COLOR_THEME_PRIMARY1 | FONT(XXS));
  dc->drawText(barInfo[bar].leftPos + 10, BAR_TOP_MARGIN + screenHeight + 3, std::to_string(barInfo[bar].value).c_str(), COLOR_THEME_PRIMARY1);
}

void HSVColorType::drawBar(BitmapBuffer* dc, int bar, getRGBFromPos getRGB)
{
  int maxRange = screenHeight;
  for (auto i = 0; i < maxRange; i++) {
    auto rgb = getRGB(bar, i);
    auto r = GET_RED(rgb);
    auto g = GET_GREEN(rgb);
    auto b = GET_BLUE(rgb);

    dc->drawSolidHorizontalLine(barInfo[bar].leftPos, i + BAR_TOP_MARGIN, barInfo[bar].barWidth, COLOR2FLAGS(RGB(r, g, b)));
  }

  dc->drawSolidRect(barInfo[bar].leftPos, BAR_TOP_MARGIN, barInfo[bar].barWidth, screenHeight, 1, COLOR2FLAGS(BLACK));
  dc->drawFilledCircle(barInfo[bar].leftPos + (barInfo[bar].barWidth / 2), BAR_TOP_MARGIN + valueToScreen(bar, barInfo[bar].value), 5, COLOR2FLAGS(WHITE));\

  drawBarValue(dc, bar);
}

void HSVColorType::paint(BitmapBuffer *dc)
{
  drawBar(dc, HUE, [=] (int bar, int pos) {
    int hue = screenToValue(0, pos);
    auto rgb = HSVtoRGB(hue, barInfo[1].value, barInfo[2].value);
    return rgb;
  });
  drawBar(dc, SATURATION, [=] (int bar, int pos) {
    int saturation = screenToValue(1, pos);
    auto rgb = HSVtoRGB(barInfo[0].value, saturation, barInfo[2].value);
    return rgb;
  });
  drawBar(dc, BRIGHTNESS, [=] (int bar, int pos) {
    int brightness = screenToValue(2, pos);
    auto rgb = HSVtoRGB(barInfo[0].value, barInfo[1].value, brightness);
    return rgb;
  });
}

RGBColorType::RGBColorType(FormGroup *window, uint32_t color) :
  ColorType(window->height() - BAR_HEIGHT_OFFSET)
{
  screenHeight = screenHeight;
  auto r = GET_RED(color), g = GET_GREEN(color), b = GET_BLUE(color);
  float values[MAX_BARS];
  values[0] = r; values[1] = g; values[2] = b;

  auto spacePerBar = (window->width() / MAX_BARS);
  int leftPos = 0;
  for (auto i = 0; i < MAX_BARS; i++) {
    barInfo[i].leftPos = leftPos + BAR_MARGIN;
    barInfo[i].barWidth = spacePerBar - BAR_MARGIN - 5;
    barInfo[i].maxValue = 255;
    barInfo[i].sliding = false;
    barInfo[i].value = values[i];
    barInfo[i].invert = true;
    leftPos += spacePerBar;
  }
}

uint32_t RGBColorType::getRGB()
{
  return RGB(barInfo[0].value, barInfo[1].value, barInfo[2].value);
}

void RGBColorType::drawBarValue(BitmapBuffer *dc, int bar)
{
  dc->drawText(barInfo[bar].leftPos, BAR_TOP_MARGIN + screenHeight + 9, RGBChars[bar], COLOR_THEME_PRIMARY1 | FONT(XXS));
  dc->drawText(barInfo[bar].leftPos + 10, BAR_TOP_MARGIN + screenHeight + 3, std::to_string(barInfo[bar].value).c_str(), COLOR_THEME_PRIMARY1);
}

void RGBColorType::paint(BitmapBuffer *dc)
{
  for (int bar = 0; bar < MAX_BARS; bar++) {
    for (uint32_t pos = 0; pos < screenHeight; pos++) {
      uint32_t value = screenToValue(bar, pos);
      uint32_t color;
      if (bar == 0)
        color = RGB(value, 0, 0);
      else if (bar == 1)
        color = RGB(0, value, 0);
      else
        color = RGB(0, 0, value);

      dc->drawSolidHorizontalLine(barInfo[bar].leftPos, pos + BAR_TOP_MARGIN, barInfo[bar].barWidth, 
                                  COLOR2FLAGS(color));
      dc->drawSolidRect(barInfo[bar].leftPos, BAR_TOP_MARGIN, barInfo[bar].barWidth, screenHeight, 1, 
                        COLOR2FLAGS(BLACK));
    }

    dc->drawFilledCircle(barInfo[bar].leftPos + (barInfo[bar].barWidth / 2), BAR_TOP_MARGIN + valueToScreen(bar, barInfo[bar].value), 5, 
                        COLOR2FLAGS(WHITE));

    drawBarValue(dc, bar);
  }
}

/////////////////////////////////////////////////////////////////////////
////// ColorEditor Base class
/////////////////////////////////////////////////////////////////////////
ColorEditor::ColorEditor(FormGroup *window, const rect_t rect, uint32_t color,
                         std::function<void (uint32_t rgb)> setValue) :
  FormGroup(window, rect),
  _setValue(std::move(setValue)),
  _color(color)
{
  _colorType = new HSVColorType(this, color);
  setFocusHandler([=](bool focus) {
    _focused = focus;
  });
}

void ColorEditor::setNextFocusBar()
{
  _focusBar = (_focusBar + 1) % MAX_BARS;
  // setFocus();
  invalidate();
}

void ColorEditor::setColorEditorType(COLOR_EDITOR_TYPE colorType)
{
  if (_colorType != nullptr) {
    delete _colorType;
  }
  if (colorType == RGB_COLOR_EDITOR) 
    _colorType = new RGBColorType(this, _color);
  else
    _colorType = new HSVColorType(this, _color);
  invalidate();
}

void ColorEditor::setRGB()
{
  _color = _colorType->getRGB();
  invalidate();
  if (_setValue != nullptr)
    _setValue(_color);
}

#if defined(HARDWARE_TOUCH)
bool ColorEditor::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  if (touchState.event == TE_SLIDE_END) {
    onTouchEnd(0,0);
    return true;
  }

  for (auto i = 0; i < MAX_BARS; i++) {
    if (_colorType->barInfo[i].sliding) {
      y -= BAR_TOP_MARGIN;
      auto value = _colorType->screenToValue(i, y);
      if (value != _colorType->barInfo[i].value) {
        _colorType->barInfo[i].value = value;    
        setRGB(); 
      } 
    }
  }

  return true;
}

bool ColorEditor::onTouchEnd(coord_t x, coord_t y)
{
  bool bSliding = false;
  for (auto i = 0; i < MAX_BARS; i++) {
    if (_colorType->barInfo[i].sliding) {
      bSliding = true;
    }
    _colorType->barInfo[i].sliding = false;
  }

  if (bSliding) {
    invalidate();
  }

  return FormGroup::onTouchEnd(x,y);
}

bool ColorEditor::onTouchStart(coord_t x, coord_t y)
{
  bool bFound = false;
  for (auto i = 0; i < MAX_BARS; i++) {
    if (y >= BAR_TOP_MARGIN && y < (coord_t)(BAR_TOP_MARGIN + _colorType->screenHeight) &&
        x >= _colorType->barInfo[i].leftPos && x < _colorType->barInfo[i].leftPos + _colorType->barInfo[i].barWidth) {
      _colorType->barInfo[i].sliding = true;
      y -= BAR_TOP_MARGIN;
      int value = _colorType->screenToValue(i, y);
      _colorType->barInfo[i].value = value;
      bFound = true;
      _focusBar = i;
      // setFocus();
    } else {
      _colorType->barInfo[i].sliding = false;
    }
  }
  if (bFound) {
    setRGB();
    return true;
  } else {
    return FormGroup::onTouchStart(x,y);
  }
}
#endif

#if defined(HARDWARE_KEYS)
void ColorEditor::onEvent(event_t event)
{
  switch(event)
  {
    case EVT_ROTARY_RIGHT:
    case EVT_ROTARY_LEFT:
      {
        int direction = event == EVT_ROTARY_RIGHT ? 1 : -1;
        auto bar = &_colorType->barInfo[_focusBar];
        int newValue = (int) bar->value;
        newValue += direction;

        newValue = min(newValue, (int)bar->maxValue);
        newValue = max(newValue, 0);
        bar->value = newValue;
        setRGB();
        onKeyPress();
      }
    break;
    default:
      FormGroup::onEvent(event);
      break;
  }
}
#endif

void ColorEditor::drawFocusBox(BitmapBuffer *dc)
{
  auto bar = _colorType->barInfo[_focusBar];
  dc->drawSolidRect(bar.leftPos, BAR_TOP_MARGIN, bar.barWidth, _colorType->screenHeight, 2, COLOR_THEME_FOCUS);
}

void ColorEditor::paint(BitmapBuffer *dc)
{
  dc->clear(COLOR_THEME_SECONDARY3);
  _colorType->paint(dc);
  drawFocusBox(dc);
}

