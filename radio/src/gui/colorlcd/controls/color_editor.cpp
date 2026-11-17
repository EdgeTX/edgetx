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
#include "hal/rotary_encoder.h"

constexpr int MAX_BARS = 3;

// ColorTypes()
// A ColorType implements an editor for selecting a color. Currently we support
// HSV, RGB, Theme (editable theme colors) and Fixed (system colors)
class ColorType
{
 public:
  ColorType() {}
  virtual ~ColorType() {}

  virtual void updateUI() {}
  virtual uint32_t getRGB() = 0;

 protected:
};

class ColorBar : public FormField
{
 public:
  ColorBar(Window* parent, const rect_t& r) : FormField(parent, r)
  {
    setWindowFlag(NO_SCROLL);
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    lv_group_add_obj((lv_group_t*)lv_group_get_default(), lvobj);

    lv_obj_add_event_cb(lvobj, ColorBar::pressing, LV_EVENT_PRESSING, nullptr);
    lv_obj_add_event_cb(lvobj, ColorBar::on_key, LV_EVENT_KEY, nullptr);

    etx_std_style(lvobj, LV_PART_MAIN, PAD_ZERO);
    etx_obj_add_style(lvobj, styles->border_color[COLOR_THEME_EDIT_INDEX], LV_PART_MAIN | LV_STATE_EDITED);
    etx_obj_add_style(lvobj, styles->outline_color_edit, LV_PART_MAIN | LV_STATE_EDITED);

    gradientBitmap = new BitmapBuffer(BMP_RGB565, r.w - PAD_BORDER * 2, r.h - PAD_BORDER * 2);
    if (gradientBitmap)
      gradientCanvas = gradientBitmap->addCanvas(this);

    cursor = lv_obj_create(lvobj);
    lv_obj_set_x(cursor, width() / 2 - PAD_BORDER - CRSR_SZ);
    lv_obj_set_size(cursor, CRSR_SZ * 2, CRSR_SZ);
    lv_obj_clear_flag(cursor, LV_OBJ_FLAG_CLICKABLE);
    etx_obj_add_style(cursor, styles->rounded, LV_PART_MAIN);
    etx_obj_add_style(cursor, styles->border, LV_PART_MAIN);
    etx_bg_color(cursor, COLOR_THEME_PRIMARY1_INDEX);
    etx_solid_bg(cursor, COLOR_THEME_PRIMARY2_INDEX);

    onClosing([=]() {
      if (gradientBitmap) {
        gradientBitmap->removeCanvas();
        delete gradientBitmap;
      }
    });
  }

  void initUI()
  {
    fillGradient();
    setCursorPos();
  }

  void fillGradient()
  {
    if (gradientBitmap && gradientCanvas) {
      lv_draw_line_dsc_t line_dsc;
      lv_draw_line_dsc_init(&line_dsc);

      line_dsc.width = 1;
      line_dsc.opa = LV_OPA_100;

      lv_point_t p[2];
      p[0].x = 0; p[1].x = gradientBitmap->width();

      for (int i = 0; i < gradientBitmap->height(); i += 1) {
        p[0].y = i; p[1].y = i;

        auto c = getRGB(screenToValue(i));
        line_dsc.color = lv_color_make(GET_RED32(c), GET_GREEN32(c), GET_BLUE32(c));

        lv_canvas_draw_line(gradientCanvas, p, 2, &line_dsc);
      }
    }
  }

  void setCursorPos()
  {
    lv_obj_set_y(cursor, valueToScreen(value) - (CRSR_SZ / 2));
  }

  static void pressing(lv_event_t* e)
  {
    ColorBar* bar = (ColorBar*)lv_obj_get_user_data(lv_event_get_target(e));
    if (bar) bar->onPressing((lv_indev_t*)lv_event_get_param(e));
  }

  void onPressing(lv_indev_t* click_source)
  {
    if (click_source && lv_indev_get_type(click_source) == LV_INDEV_TYPE_POINTER) {
      lv_area_t obj_coords;
      lv_obj_get_coords(lvobj, &obj_coords);

      lv_point_t point_act;
      lv_indev_get_point(click_source, &point_act);

      lv_point_t rel_pos;
      rel_pos.x = point_act.x - obj_coords.x1;
      rel_pos.y = point_act.y - obj_coords.y1;

      value = screenToValue(rel_pos.y);
      Messaging::send(Messaging::COLOR_CHANGED);
      setCursorPos();
    }
  }

  static void on_key(lv_event_t* e)
  {
    ColorBar* bar = (ColorBar*)lv_obj_get_user_data(lv_event_get_target(e));
    if (bar) bar->onKey(*(uint32_t*)lv_event_get_param(e));
  }

  void onKey(uint32_t key)
  {
    if (key == LV_KEY_LEFT) {
      if (value > 0) {
        uint32_t accel = rotaryEncoderGetAccel();
        value--;
        if (accel > 0) {
          if (accel > value)
            value = 0;
          else
            value -= accel;
        }
        Messaging::send(Messaging::COLOR_CHANGED);
        setCursorPos();
      }
    } else if (key == LV_KEY_RIGHT) {
      if (value < maxValue) {
        uint32_t accel = rotaryEncoderGetAccel();
        value++;
        if (accel > 0) {
          if (accel < maxValue - value)
            value += accel;
          else
            value = maxValue;
        }
        Messaging::send(Messaging::COLOR_CHANGED);
        setCursorPos();
      }
    }
  }

  uint32_t maxValue = 0;
  uint32_t value = 0;
  std::function<uint32_t(int pos)> getRGB = nullptr;

  static LAYOUT_VAL_SCALED_ODD(CRSR_SZ, 11)

 protected:
  BitmapBuffer* gradientBitmap = nullptr;
  lv_obj_t* gradientCanvas = nullptr;
  lv_obj_t* cursor = nullptr;

  int valueToScreen(int val)
  {
    auto maxY = height() - PAD_BORDER * 2 - 1;  // exclude border

    int scaledValue = (val * maxY + maxValue / 2) / maxValue;
    return scaledValue;
  }

  uint32_t screenToValue(int pos)
  {
    auto maxY = height() - PAD_BORDER * 2 - 1;  // exclude border

    // range check
    pos = limit(0, pos, maxY);

    uint32_t scaledValue = ((pos * maxValue + maxY / 2) / maxY);
    return scaledValue;
  }
};

// Color editor with three bars for selecting color value. Base class for HSV
// and RGB color types.
class BarColorType : public ColorType, public Window
{
 public:
  BarColorType(Window* parent, const char* labels) :
      Window(parent, {0, 0, parent->width(), parent->height()}),
      labels(labels)
  {
    setWindowFlag(NO_SCROLL);

    auto spacePerBar = (width() - PAD_OUTLINE * (MAX_BARS + 1)) / MAX_BARS;

    rect_t r;
    r.y = PAD_OUTLINE + 1;
    r.w = spacePerBar;
    r.h = height() - (PAD_OUTLINE * 2 + EdgeTxStyles::STD_FONT_HEIGHT);

    for (int i = 0; i < MAX_BARS; i++) {
      r.x = i * (spacePerBar + PAD_OUTLINE) + PAD_OUTLINE;

      bars[i] = new ColorBar(this, r);

      // bar labels
      lv_obj_t* obj = etx_label_create(lvobj);
      lv_obj_set_pos(obj, bars[i]->left(), bars[i]->bottom() + PAD_OUTLINE);
      lv_obj_set_size(obj, r.w, EdgeTxStyles::STD_FONT_HEIGHT);
      lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
      etx_txt_color(obj, COLOR_THEME_PRIMARY1_INDEX);
      barValLabels[i] = obj;
    }
  }

  void updateUI() override
  {
    for (int i = 0; i < MAX_BARS; i++)
      lv_label_set_text_fmt(barValLabels[i], "%c:%" PRIu32, labels[i], bars[i]->value);
    updateBars();
  }

 protected:
  ColorBar* bars[MAX_BARS];
  lv_obj_t* barValLabels[MAX_BARS];
  const char* labels;

  virtual void updateBars() {}
};

// Color editor for HSV color input
class HSVColorType : public BarColorType
{
 public:
  HSVColorType(Window* parent, uint32_t color) : BarColorType(parent, "HSV")
  {
    auto rgb = color32ToRGB(color);

    auto r = GET_RED32(rgb), g = GET_GREEN32(rgb), b = GET_BLUE32(rgb);
    float values[MAX_BARS];
    RGBtoHSV(r, g, b, values[0], values[1], values[2]);
    values[1] *= MAX_SATURATION;  // convert the proper base
    values[2] *= MAX_BRIGHTNESS;

    // hue
    bars[0]->getRGB = [=](int pos) {
      return HSVtoRGB32(pos, bars[1]->value, bars[2]->value);
    };

    // saturation
    bars[1]->getRGB = [=](int pos) {
      return HSVtoRGB32(bars[0]->value, pos, bars[2]->value);
    };

    // brightness
    bars[2]->getRGB = [=](int pos) {
      return HSVtoRGB32(bars[0]->value, bars[1]->value, pos);
    };

    bars[0]->maxValue = MAX_HUE;
    bars[1]->maxValue = MAX_SATURATION;
    bars[2]->maxValue = MAX_BRIGHTNESS;

    for (auto i = 0; i < MAX_BARS; i++) {
      bars[i]->value = values[i];
      bars[i]->initUI();
    }
  }

  uint32_t getRGB() override
  {
    return HSVtoRGB32(bars[0]->value, bars[1]->value, bars[2]->value) | RGB888_FLAG;
  }

 protected:
  void updateBars() override
  {
    for (int i = 0; i < MAX_BARS; i++)
      bars[i]->fillGradient();
  }
};

// Color editor for RGB color input
class RGBColorType : public BarColorType
{
 public:
  RGBColorType(Window* parent, uint32_t color) : BarColorType(parent, "RGB")
  {
    auto rgb = color32ToRGB(color);

    auto r = GET_RED32(rgb), g = GET_GREEN32(rgb), b = GET_BLUE32(rgb);
    float values[MAX_BARS];
    values[0] = r;
    values[1] = g;
    values[2] = b;

    bars[0]->getRGB = [=](int pos) { return RGB32(pos, 0, 0); };
    bars[1]->getRGB = [=](int pos) { return RGB32(0, pos, 0); };
    bars[2]->getRGB = [=](int pos) { return RGB32(0, 0, pos); };

    for (auto i = 0; i < MAX_BARS; i++) {
      bars[i]->maxValue = 255;
      bars[i]->value = values[i];
      bars[i]->initUI();
    }
  }

  uint32_t getRGB() override
  {
    return RGB32(bars[0]->value, bars[1]->value, bars[2]->value) | RGB888_FLAG;
  }

 protected:
};

// Color editor that shows the system theme colors as buttons
class ThemeColorType : public ColorType, public Window
{
 public:
  ThemeColorType(Window* parent, uint32_t color) : Window(parent, rect_t{})
  {
    m_color = color;

    setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

    makeButtonsRow(COLOR_THEME_PRIMARY1_INDEX, COLOR_THEME_PRIMARY2_INDEX,
                  COLOR_THEME_PRIMARY3_INDEX);
    makeButtonsRow(COLOR_THEME_SECONDARY1_INDEX,
                  COLOR_THEME_SECONDARY2_INDEX, COLOR_THEME_SECONDARY3_INDEX);
    makeButtonsRow(COLOR_THEME_FOCUS_INDEX, COLOR_THEME_EDIT_INDEX,
                  COLOR_THEME_ACTIVE_INDEX);
    makeButtonsRow(COLOR_THEME_WARNING_INDEX, COLOR_THEME_DISABLED_INDEX,
                  COLOR_THEME_DISABLED_INDEX);
    makeButtonsRow(COLOR_THEME_QM_BG_INDEX, COLOR_THEME_QM_FG_INDEX,
                  COLOR_THEME_QM_FG_INDEX);
  }

  uint32_t getRGB() { return m_color; }

 protected:
  uint32_t m_color;

  void makeButton(Window* parent, uint16_t color)
  {
    auto btn = new TextButton(parent, {0, 0, BTN_W, 0}, "");
    etx_bg_color(btn->getLvObj(), (LcdColorIndex)color);
    btn->setPressHandler([=]() {
      m_color = color;
      Messaging::send(Messaging::COLOR_CHANGED);
      return 0;
    });
  }

  void makeButtonsRow(uint16_t c1, uint16_t c2, uint16_t c3)
  {
    auto hbox = new Window(this, rect_t{});
    hbox->padAll(PAD_OUTLINE);
    hbox->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_OUTLINE);
    lv_obj_set_flex_align(hbox->getLvObj(), LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

    makeButton(hbox, c1);
    if (c2 != c1) makeButton(hbox, c2);
    if (c3 != c2) makeButton(hbox, c3);
  }

  static LAYOUT_VAL_SCALED(BTN_W, 44)
};

// Color editor that shows the system fixed colors as buttons
class FixedColorType : public ColorType, public Window
{
 public:
  FixedColorType(Window* parent, uint32_t color) : Window(parent, rect_t{})
  {
    m_color = color;

    padAll(PAD_OUTLINE);
    setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_ZERO);
    lv_obj_set_flex_align(lvobj, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

    for (int c = COLOR_BLACK_INDEX; c < TOTAL_COLOR_COUNT; c += 1)
      makeButton(c);
  }

  uint32_t getRGB() { return m_color; }

 protected:
  uint32_t m_color;

  void makeButton(uint16_t color)
  {
    auto btn = new TextButton(this, {0, 0, BTN_W, 0}, "");
    etx_bg_color(btn->getLvObj(), (LcdColorIndex)color);
    btn->setPressHandler([=]() {
      m_color = color;
      Messaging::send(Messaging::COLOR_CHANGED);
      return 0;
    });
  }

  static LAYOUT_VAL_SCALED(BTN_W, 42)
};

/////////////////////////////////////////////////////////////////////////
////// ColorEditor Base class
/////////////////////////////////////////////////////////////////////////
ColorEditor::ColorEditor(Window* parent, const rect_t& rect, uint32_t color,
                         std::function<void(uint32_t rgb)> setValue,
                         COLOR_EDITOR_FMT fmt, COLOR_EDITOR_TYPE typ) :
    Window(parent, rect), _setValue(std::move(setValue)),
    format(fmt)
{
  if (format == ETX_RGB565) {
    if (color & RGB_FLAG) {
      color = COLOR_VAL(color);
      auto r = GET_RED(color);
      auto g = GET_GREEN(color);
      auto b = GET_BLUE(color);
      color = RGB32(r, g, b) | RGB888_FLAG;
    } else {
      color = COLOR_VAL(color);
    }
  }
  _color = color;
  setColorEditorType(typ);

  Messaging::send(Messaging::COLOR_PREVIEW, _color);

  colorUpdateMsg.subscribe(Messaging::COLOR_CHANGED, [=](uint32_t param) { setRGB(); });
}

void ColorEditor::setColorEditorType(COLOR_EDITOR_TYPE colorType)
{
  clear();
  if (colorType == RGB_COLOR_EDITOR)
    _colorType = new RGBColorType(this, _color);
  else if (colorType == HSV_COLOR_EDITOR)
    _colorType = new HSVColorType(this, _color);
  else if (colorType == THM_COLOR_EDITOR)
    _colorType = new ThemeColorType(this, _color);
  else
    _colorType = new FixedColorType(this, _color);
  updateUI();
}

void ColorEditor::updateUI()
{
  _colorType->updateUI();
  if (_setValue != nullptr) {
    uint32_t c = _color;
    if (format == ETX_RGB565) {
      if (c & RGB888_FLAG) {
        auto r = GET_RED32(c);
        auto g = GET_GREEN32(c);
        auto b = GET_BLUE32(c);
        c = RGB2FLAGS(r, g, b);
      } else {
        c = COLOR2FLAGS(c);
      }
    }
    _setValue(c);
  }
}

void ColorEditor::setRGB()
{
  _color = _colorType->getRGB();
  // update bars & labels
  updateUI();
  Messaging::send(Messaging::COLOR_PREVIEW, _color);
}
