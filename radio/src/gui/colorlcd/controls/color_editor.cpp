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

#include "button.h"
#include "etx_lv_theme.h"

static const char* const RGBChars[MAX_BARS] = {"R", "G", "B"};
static const char* const HSVChars[MAX_BARS] = {"H", "S", "V"};

typedef std::function<uint32_t(int pos)> getRGBFromPos;

class ColorBar : public FormField
{
 public:
  ColorBar(Window* parent, const rect_t& r, uint32_t value = 0,
           uint32_t maxValue = 0, bool invert = false) :
      FormField(parent, r)
  {
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_ENCODER_ACCEL);

    lv_group_add_obj((lv_group_t*)lv_group_get_default(), lvobj);

    lv_obj_add_event_cb(lvobj, ColorBar::pressing, LV_EVENT_PRESSING, nullptr);
    lv_obj_add_event_cb(lvobj, ColorBar::on_key, LV_EVENT_KEY, nullptr);
    lv_obj_add_event_cb(lvobj, ColorBar::draw_end, LV_EVENT_DRAW_PART_END,
                        nullptr);

    etx_std_style(lvobj, LV_PART_MAIN, PAD_ZERO);
    etx_obj_add_style(lvobj, styles->border_color[COLOR_THEME_EDIT_INDEX], LV_PART_MAIN | LV_STATE_EDITED);
    etx_obj_add_style(lvobj, styles->outline_color_edit, LV_PART_MAIN | LV_STATE_EDITED);
  }

  int valueToScreen(int val)
  {
    auto h = height() - 4;  // exclude border

    int scaledValue = (val * h + maxValue / 2) / maxValue;
    if (invert) scaledValue = h - scaledValue;
    return scaledValue;
  }

  uint32_t screenToValue(int pos)
  {
    auto h = height() - 4;  // exclude border

    // range check
    pos = min<int>(pos, h);
    pos = max<int>(pos, 0);

    uint32_t scaledValue = ((pos * maxValue + h / 2) / h);
    if (invert) scaledValue = maxValue - scaledValue;
    return scaledValue;
  }

  static void pressing(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    lv_indev_t* click_source = (lv_indev_t*)lv_event_get_param(e);
    if (!click_source ||
        (lv_indev_get_type(click_source) != LV_INDEV_TYPE_POINTER))
      return;

    ColorBar* bar = (ColorBar*)lv_obj_get_user_data(target);
    if (!bar) return;

    lv_area_t obj_coords;
    lv_obj_get_coords(target, &obj_coords);

    lv_point_t point_act;
    lv_indev_get_point(click_source, &point_act);

    lv_point_t rel_pos;
    rel_pos.x = point_act.x - obj_coords.x1;
    rel_pos.y = point_act.y - obj_coords.y1;

    TRACE("PRESSING [%d,%d]", rel_pos.x, rel_pos.y);

    bar->value = bar->screenToValue(rel_pos.y);
    lv_event_send(target->parent, LV_EVENT_VALUE_CHANGED, nullptr);
  }

  static void on_key(lv_event_t* e)
  {
    lv_obj_t* obj = lv_event_get_target(e);
    ColorBar* bar = (ColorBar*)lv_obj_get_user_data(obj);
    if (!bar) return;

    uint32_t key = *(uint32_t*)lv_event_get_param(e);
    if (key == LV_KEY_LEFT) {
      if (bar->value > 0) {
        uint32_t accel = rotaryEncoderGetAccel();
        bar->value--;
        if (accel > 0) {
          if (accel > bar->value)
            bar->value = 0;
          else
            bar->value -= accel;
        }
        lv_event_send(obj->parent, LV_EVENT_VALUE_CHANGED, nullptr);
      }
    } else if (key == LV_KEY_RIGHT) {
      if (bar->value < bar->maxValue) {
        uint32_t accel = rotaryEncoderGetAccel();
        bar->value++;
        if (accel > 0) {
          if (accel < bar->maxValue - bar->value)
            bar->value += accel;
          else
            bar->value = bar->maxValue;
        }
        lv_event_send(obj->parent, LV_EVENT_VALUE_CHANGED, nullptr);
      }
    }
  }

  static void draw_end(lv_event_t* e)
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
    int h = area->y2 - area->y1 - 4;

    // draw background gradient
    for (int i = 0; i <= h; i += 1) {
      p1.y = p2.y = i + area->y1 + 2;
      if (i == 0 || i == h) {
        p1.x = area->x1 + 3;
        p2.x = area->x2 - 2;
      } else {
        p1.x = area->x1 + 2;
        p2.x = area->x2 - 1;
      }
      auto c = bar->getRGB(bar->screenToValue(i));
      line_dsc.color = lv_color_make(GET_RED(c), GET_GREEN(c), GET_BLUE(c));
      lv_draw_line(dsc->draw_ctx, &line_dsc, &p1, &p2);
    }

    // draw cursor
    lv_area_t cursor_area;
    cursor_area.x1 = area->x1 + (lv_area_get_width(area) / 2) - ColorEditor::CRSR_SZ / 2;
    cursor_area.x2 = cursor_area.x1 + ColorEditor::CRSR_SZ - 1;

    auto pos = bar->valueToScreen(bar->value);
    cursor_area.y1 = area->y1 + pos - ColorEditor::CRSR_YO;
    cursor_area.y2 = cursor_area.y1 + ColorEditor::CRSR_SZ - 1;

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

  uint32_t maxValue = 0;
  uint32_t value = 0;
  bool invert = false;
  getRGBFromPos getRGB = nullptr;
};

// ColorTypes()
// A ColorType implements an editor for selecting a color. Currently we support
// HSV, RGB and SYS (choose from system defined colors)
class ColorType
{
 public:
  ColorType() {}
  virtual ~ColorType() {}

  virtual void setText() {};
  virtual uint32_t getRGB() { return 0; };

 protected:
};

// Color editor with three bars for selecting color value. Base class for HSV
// and RGB color types.
class BarColorType : public ColorType
{
 public:
  BarColorType(Window* parent)
  {
    auto spacePerBar = (parent->width() / MAX_BARS);

    int leftPos = 0;
    rect_t r;
    r.y = ColorEditor::BAR_TOP_MARGIN;
    r.w = spacePerBar - ColorEditor::BAR_MARGIN - 5;
    r.h = parent->height() - (ColorEditor::BAR_TOP_MARGIN + ColorEditor::BAR_HEIGHT_OFFSET);

    for (int i = 0; i < MAX_BARS; i++) {
      r.x = leftPos + ColorEditor::BAR_MARGIN;

      bars[i] = new ColorBar(parent, r);
      leftPos += spacePerBar;

      // bar labels
      auto bar = bars[i];
      auto x = bar->left();
      auto y = bar->bottom();

      barLabels[i] = create_bar_label(parent->getLvObj(), x, y + ColorEditor::LBL_YO);
      barValLabels[i] = create_bar_value_label(parent->getLvObj(), x + ColorEditor::VAL_XO, y + ColorEditor::VAL_YO);
    }
  }

  ~BarColorType() override
  {
    for (int i = 0; i < MAX_BARS; i++) {
      bars[i]->deleteLater();
    }
  };

  void setText() override
  {
    for (int i = 0; i < MAX_BARS; i++) {
      auto bar = bars[i];
      lv_label_set_text_static(barLabels[i], getLabelChars()[i]);
      lv_label_set_text_fmt(barValLabels[i], "%" PRIu32, bar->value);
      bar->invalidate();
    }
  }

 protected:
  ColorBar* bars[MAX_BARS];
  lv_obj_t* barLabels[MAX_BARS];
  lv_obj_t* barValLabels[MAX_BARS];

  virtual const char* const* getLabelChars() { return nullptr; };

  lv_obj_t* create_bar_label(lv_obj_t* parent, lv_coord_t x, lv_coord_t y)
  {
    lv_obj_t* obj = lv_label_create(parent);
    lv_obj_set_pos(obj, x, y);
    etx_txt_color(obj, COLOR_THEME_PRIMARY1_INDEX);
    etx_font(obj, FONT_XXS_INDEX);
    return obj;
  }

  lv_obj_t* create_bar_value_label(lv_obj_t* parent, lv_coord_t x,
                                   lv_coord_t y)
  {
    lv_obj_t* obj = lv_label_create(parent);
    lv_obj_set_pos(obj, x, y);
    etx_txt_color(obj, COLOR_THEME_PRIMARY1_INDEX);
    return obj;
  }
};

// Color editor for HSV color input
class HSVColorType : public BarColorType
{
 public:
  HSVColorType(Window* parent, uint32_t color) : BarColorType(parent)
  {
    auto rgb = COLOR_VAL(colorToRGB(color));

    auto r = GET_RED(rgb), g = GET_GREEN(rgb), b = GET_BLUE(rgb);
    float values[MAX_BARS];
    RGBtoHSV(r, g, b, values[0], values[1], values[2]);
    values[1] *= MAX_SATURATION;  // convert the proper base
    values[2] *= MAX_BRIGHTNESS;

    for (auto i = 0; i < MAX_BARS; i++) {
      bars[i]->maxValue = i == 0 ? MAX_HUE : MAX_BRIGHTNESS;
      bars[i]->invert = i != 0;
      bars[i]->value = values[i];
    }

    // hue
    bars[0]->getRGB = [=](int pos) {
      auto rgb = HSVtoRGB(pos, bars[1]->value, bars[2]->value);
      return rgb;
    };

    // saturation
    bars[1]->getRGB = [=](int pos) {
      auto rgb = HSVtoRGB(bars[0]->value, pos, bars[2]->value);
      return rgb;
    };

    // brightness
    bars[2]->getRGB = [=](int pos) {
      auto rgb = HSVtoRGB(bars[0]->value, bars[1]->value, pos);
      return rgb;
    };
  }

  uint32_t getRGB() override
  {
    return COLOR2FLAGS(HSVtoRGB(bars[0]->value, bars[1]->value, bars[2]->value)) | RGB_FLAG;
  }

 protected:
  const char* const* getLabelChars() override { return HSVChars; }
};

// Color editor for RGB color input
class RGBColorType : public BarColorType
{
 public:
  RGBColorType(Window* parent, uint32_t color) : BarColorType(parent)
  {
    auto rgb = COLOR_VAL(colorToRGB(color));

    auto r = GET_RED(rgb), g = GET_GREEN(rgb), b = GET_BLUE(rgb);
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

  uint32_t getRGB() override
  {
    return RGB2FLAGS(bars[0]->value, bars[1]->value, bars[2]->value);
  }

 protected:
  const char* const* getLabelChars() override { return RGBChars; }
};

// Color editor that shows the system theme colors as buttons
class ThemeColorType : public ColorType
{
 public:
  ThemeColorType(Window* parent, uint32_t color)
  {
    m_color = color;

    auto vbox = new Window(parent, rect_t{});
    vbox->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_MEDIUM);

    makeButtonsRow(vbox, COLOR_THEME_PRIMARY1_INDEX, COLOR_THEME_PRIMARY2_INDEX,
                  COLOR_THEME_PRIMARY3_INDEX);
    makeButtonsRow(vbox, COLOR_THEME_SECONDARY1_INDEX,
                  COLOR_THEME_SECONDARY2_INDEX, COLOR_THEME_SECONDARY3_INDEX);
    makeButtonsRow(vbox, COLOR_THEME_FOCUS_INDEX, COLOR_THEME_EDIT_INDEX,
                  COLOR_THEME_ACTIVE_INDEX);
    makeButtonsRow(vbox, COLOR_THEME_WARNING_INDEX, COLOR_THEME_DISABLED_INDEX,
                  COLOR_THEME_DISABLED_INDEX);
  }

  uint32_t getRGB() { return m_color; }

 protected:
  uint32_t m_color;

  void makeButton(Window* parent, uint16_t color)
  {
    auto btn = new TextButton(parent, rect_t{}, "       ");
    etx_bg_color(btn->getLvObj(), (LcdColorIndex)color);
    btn->setPressHandler([=]() {
      m_color = COLOR2FLAGS(color);
      lv_event_send(parent->getParent()->getParent()->getLvObj(),
                    LV_EVENT_VALUE_CHANGED, nullptr);
      return 0;
    });
  }

  void makeButtonsRow(Window* parent, uint16_t c1, uint16_t c2, uint16_t c3)
  {
    auto hbox = new Window(parent, rect_t{});
    hbox->padAll(PAD_TINY);
    hbox->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_MEDIUM);
    lv_obj_set_flex_align(hbox->getLvObj(), LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

    makeButton(hbox, c1);
    makeButton(hbox, c2);
    if (c3 != c2) makeButton(hbox, c3);
  }
};

/////////////////////////////////////////////////////////////////////////
////// ColorEditor Base class
/////////////////////////////////////////////////////////////////////////
ColorEditor::ColorEditor(Window* parent, const rect_t& rect, uint32_t color,
                         std::function<void(uint32_t rgb)> setValue) :
    Window(parent, rect), _setValue(std::move(setValue)), _color(color)
{
  _colorType = new HSVColorType(this, color);
  _colorType->setText();

  lv_obj_add_event_cb(lvobj, ColorEditor::value_changed, LV_EVENT_VALUE_CHANGED,
                      nullptr);
}

void ColorEditor::setColorEditorType(COLOR_EDITOR_TYPE colorType)
{
  if (_colorType != nullptr) {
    clear();
    delete _colorType;
  }
  if (colorType == RGB_COLOR_EDITOR) {
    _colorType = new RGBColorType(this, _color);
    setRGB();
  } else if (colorType == HSV_COLOR_EDITOR) {
    _colorType = new HSVColorType(this, _color);
    setHSV();
  } else {
    _colorType = new ThemeColorType(this, _color);
    setText();
  }
  // Update color bars
  invalidate();
}

void ColorEditor::setText()
{
  _colorType->setText();
  if (_setValue != nullptr) _setValue(_color);
}

void ColorEditor::setRGB()
{
  _color = _colorType->getRGB();
  // update bars & labels
  setText();
}

void ColorEditor::setHSV()
{
  // update bars & labels
  setText();
}

void ColorEditor::value_changed(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);
  ColorEditor* edit = (ColorEditor*)lv_obj_get_user_data(target);
  if (edit) edit->setRGB();
}
