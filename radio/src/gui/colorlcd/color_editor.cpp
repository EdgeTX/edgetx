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
#include "font.h"

constexpr int BAR_MARGIN = 5;

constexpr int BAR_TOP_MARGIN = 5;
constexpr int BAR_HEIGHT_OFFSET = BAR_TOP_MARGIN + 25;

static const char *RGBChars[MAX_BARS] = { "R", "G", "B" };
static const char *HSVChars[MAX_BARS] = { "H", "S", "V" };

// ColorTypes()
// A ColorType implements a three bar color editor.  Currently we support HSV and RGB
enum HSV_BAR_TYPE
{
  HUE = 0,
  SATURATION = 1,
  BRIGHTNESS = 2
};

void ColorBar::pressing(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);
  lv_indev_t* click_source = (lv_indev_t*)lv_event_get_param(e);
  if (!click_source ||
      (lv_indev_get_type(click_source) != LV_INDEV_TYPE_POINTER))
    return;

  lv_area_t obj_coords;
  lv_obj_get_coords(target, &obj_coords);

  lv_point_t point_act;
  lv_indev_get_point(click_source, &point_act);

  lv_point_t rel_pos;
  rel_pos.x = point_act.x - obj_coords.x1;
  rel_pos.y = point_act.y - obj_coords.y1;

  TRACE("PRESSING [%d,%d]", rel_pos.x, rel_pos.y);

  ColorBar* bar = (ColorBar*)lv_obj_get_user_data(target);
  if (!bar) return;

  bar->value = bar->screenToValue(rel_pos.y);
  lv_event_send(target->parent, LV_EVENT_VALUE_CHANGED, nullptr);
}

void ColorBar::on_key(lv_event_t* e)
{
  lv_obj_t* obj = lv_event_get_target(e);
  ColorBar* bar = (ColorBar*)lv_obj_get_user_data(obj);
  if (!bar) return;

  uint32_t key = *(uint32_t*)lv_event_get_param(e);
  if (key == LV_KEY_LEFT) {
    if (bar->value > 0) {
      bar->value--;
      lv_event_send(obj->parent, LV_EVENT_VALUE_CHANGED, nullptr);
    }
  } else if (key == LV_KEY_RIGHT) {
    if (bar->value < bar->maxValue) {
      bar->value++;
      lv_event_send(obj->parent, LV_EVENT_VALUE_CHANGED, nullptr);
    }
  }
}

void ColorBar::draw_end(lv_event_t* e)
{
  lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
  if (dsc->type != LV_OBJ_DRAW_PART_RECTANGLE) return;
  
  lv_obj_t* obj = lv_event_get_target(e);
  ColorBar* bar = (ColorBar*)lv_obj_get_user_data(obj);
  if (!bar) return;
  
  lv_draw_line_dsc_t line_dsc;
  lv_draw_line_dsc_init(&line_dsc);

  line_dsc.width = 1;
  line_dsc.opa = LV_OPA_100;
  
  auto area = dsc->draw_area;
  lv_point_t p1, p2;
  p1.x = area->x1;
  p2.x = area->x2 + 1;

  // draw background gradient
  for (auto y = area->y1; y <= area->y2; y++) {
    p1.y = y;
    p2.y = y;
    auto c = bar->getRGB(bar->screenToValue(y - area->y1));
    line_dsc.color = lv_color_make(GET_RED(c), GET_GREEN(c), GET_BLUE(c));
    lv_draw_line(dsc->draw_ctx, &line_dsc, &p1, &p2);
  }

  // draw cursor
  lv_area_t cursor_area;
  cursor_area.x1 = area->x1 + (lv_area_get_width(area) / 2) - 5;
  cursor_area.x2 = cursor_area.x1 + 10 - 1;

  auto pos = bar->valueToScreen(bar->value);
  cursor_area.y1 = area->y1 + pos - 5;
  cursor_area.y2 = cursor_area.y1 + 10 - 1;

  lv_draw_rect_dsc_t cursor_dsc;
  lv_draw_rect_dsc_init(&cursor_dsc);

  cursor_dsc.radius = LV_RADIUS_CIRCLE;
  cursor_dsc.bg_opa = LV_OPA_100;
  cursor_dsc.bg_color = makeLvColor(COLOR_THEME_PRIMARY2);
  cursor_dsc.border_opa = LV_OPA_100;
  cursor_dsc.border_color = makeLvColor(COLOR_THEME_PRIMARY1);
  cursor_dsc.border_width = 1;

  lv_draw_rect(dsc->draw_ctx, &cursor_dsc, &cursor_area);
}
  
ColorBar::ColorBar(Window* parent, const rect_t& r, uint32_t value,
                   uint32_t maxValue, bool invert) :
  FormField(parent, r)
{
  lv_group_add_obj((lv_group_t*)lv_group_get_default(), lvobj);
  lv_obj_add_event_cb(lvobj, ColorBar::pressing, LV_EVENT_PRESSING, nullptr);
  lv_obj_add_event_cb(lvobj, ColorBar::on_key, LV_EVENT_KEY, nullptr);
  lv_obj_add_event_cb(lvobj, ColorBar::draw_end, LV_EVENT_DRAW_PART_END, nullptr);

  // normal
  lv_obj_set_style_border_width(lvobj, 1, 0);
  lv_obj_set_style_border_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY1), 0);
  lv_obj_set_style_border_opa(lvobj, LV_OPA_100, 0);
  lv_obj_set_style_border_post(lvobj, true, 0);

  // focused
  lv_obj_set_style_border_width(lvobj, 2, LV_STATE_FOCUSED);
  lv_obj_set_style_border_color(lvobj, makeLvColor(COLOR_THEME_FOCUS), LV_STATE_FOCUSED);
}

int ColorBar::valueToScreen(int val)
{
  int scaledValue = (((float)val / maxValue) * height());
  if (invert) scaledValue = height() - scaledValue;
  return scaledValue;
}

uint32_t ColorBar::screenToValue(int pos)
{
  // range check
  pos = min<int>(pos, height());
  pos = max<int>(pos, 0);

  uint32_t scaledValue = (((float)pos / height()) * maxValue);
  if (invert) scaledValue = maxValue - scaledValue;
  return scaledValue;
}

ColorType::ColorType(Window* parent, coord_t screenHeight) :
  screenHeight(screenHeight)
{
  auto spacePerBar = (parent->width() / MAX_BARS);

  int leftPos = 0;
  for ( int i=0; i < MAX_BARS; i++){
    rect_t r;
    r.x = leftPos + BAR_MARGIN;
    r.y = BAR_TOP_MARGIN;
    r.w = spacePerBar - BAR_MARGIN - 5;
    r.h = screenHeight;
    
    bars[i] = new ColorBar(parent, r);
    leftPos += spacePerBar;
  }
}

ColorType::~ColorType()
{
  for ( int i=0; i < MAX_BARS; i++) {
    bars[i]->deleteLater();
  }  
};

HSVColorType::HSVColorType(Window* parent, uint32_t color) :
  ColorType(parent, parent->height() - BAR_HEIGHT_OFFSET)
{
  auto r = GET_RED(color), g = GET_GREEN(color), b = GET_BLUE(color);
  float values[MAX_BARS];
  RGBtoHSV(r, g, b, values[0], values[1], values[2]);
  values[1] *= MAX_SATURATION; // convert the proper base
  values[2] *= MAX_BRIGHTNESS;

  for (auto i = 0; i < MAX_BARS; i++) {
    bars[i]->maxValue = i == 0 ? MAX_HUE : MAX_BRIGHTNESS;
    bars[i]->invert = i != 0;
    bars[i]->value = values[i];
  }

  // hue
  bars[0]->getRGB = [=] (int pos) {
    auto rgb = HSVtoRGB(pos, bars[1]->value, bars[2]->value);
    return rgb;
  };

  // saturation
  bars[1]->getRGB = [=] (int pos) {
    auto rgb = HSVtoRGB(bars[0]->value, pos, bars[2]->value);
    return rgb;
  };

  // brightness
  bars[2]->getRGB = [=] (int pos) {
    auto rgb = HSVtoRGB(bars[0]->value, bars[1]->value, pos);
    return rgb;
  };
}

uint32_t HSVColorType::getRGB()
{
  return HSVtoRGB(bars[0]->value, bars[1]->value, bars[2]->value);
}

const char** HSVColorType::getLabelChars()
{
  return HSVChars;
}

RGBColorType::RGBColorType(Window* parent, uint32_t color) :
    ColorType(parent, parent->height() - BAR_HEIGHT_OFFSET)
{
  auto r = GET_RED(color), g = GET_GREEN(color), b = GET_BLUE(color);
  float values[MAX_BARS];
  values[0] = r;
  values[1] = g;
  values[2] = b;

  for (auto i = 0; i < MAX_BARS; i++) {
    bars[i]->maxValue = 255;
    bars[i]->value = values[i];
    bars[i]->invert = true;
  }

  bars[0]->getRGB = [=](int pos) { return RGB(pos, 0, 0); };
  bars[1]->getRGB = [=](int pos) { return RGB(0, pos, 0); };
  bars[2]->getRGB = [=](int pos) { return RGB(0, 0, pos); };
}

uint32_t RGBColorType::getRGB()
{
  return RGB(bars[0]->value, bars[1]->value, bars[2]->value);
}

const char** RGBColorType::getLabelChars()
{
  return RGBChars;
}

static lv_obj_t* create_bar_label(lv_obj_t* parent, lv_coord_t x, lv_coord_t y)
{
  lv_obj_t* obj = lv_label_create(parent);
  lv_obj_set_pos(obj, x, y);
  lv_obj_set_style_text_color(obj, makeLvColor(COLOR_THEME_PRIMARY1), 0);
  lv_obj_set_style_text_font(obj, getFont(FONT(XXS)), 0);
  return obj;
}

static lv_obj_t* create_bar_value_label(lv_obj_t* parent, lv_coord_t x, lv_coord_t y)
{
  lv_obj_t* obj = lv_label_create(parent);
  lv_obj_set_pos(obj, x, y);
  lv_obj_set_style_text_color(obj, makeLvColor(COLOR_THEME_PRIMARY1), 0);
  return obj;
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

  // bar labels
  for (int i = 0; i < MAX_BARS; i++) {
    auto bar = _colorType->bars[i];
    auto x = bar->left();
    auto y = bar->bottom();

    barLabels[i] = create_bar_label(lvobj, x, y + 9);
    lv_label_set_text_static(barLabels[i], _colorType->getLabelChars()[i]);
    
    barValLabels[i] = create_bar_value_label(lvobj, x + 10, y + 3);
    lv_label_set_text_fmt(barValLabels[i], "%d", bar->value);
  }

  lv_obj_add_event_cb(lvobj, ColorEditor::value_changed, LV_EVENT_VALUE_CHANGED, nullptr);
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

  // update bars & labels
  for (int i = 0; i < MAX_BARS; i++) {
    auto bar = _colorType->bars[i];
    lv_label_set_text_fmt(barValLabels[i], "%d", bar->value);
    bar->invalidate();
  }

  if (_setValue != nullptr) _setValue(_color);
}

void ColorEditor::value_changed(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);
  ColorEditor* edit = (ColorEditor*)lv_obj_get_user_data(target);
  if (edit) edit->setRGB();
}

