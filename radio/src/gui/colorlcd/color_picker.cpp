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

#include "color_picker.h"
#include "color_editor.h"

// based on LVGL default switch size
constexpr lv_coord_t COLOR_PAD_WIDTH = (4 * LV_DPI_DEF) / 10;
constexpr lv_coord_t COLOR_PAD_HEIGHT = (4 * LV_DPI_DEF) / 17;

#if LCD_W > LCD_H
// Landscape
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#else
// Portrait
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

class ColorEditorPopup : public Dialog
{
  lv_obj_t* colorPad = nullptr;
  lv_obj_t* hexStr = nullptr;
  uint32_t m_color;

  void updateColor(uint32_t c)
  {
    m_color = c;
    auto r = GET_RED(c), g = GET_GREEN(c), b = GET_BLUE(c);

    auto lvcolor = lv_color_make(r, g, b);
    lv_obj_set_style_bg_color(colorPad, lvcolor, LV_PART_MAIN);
    lv_label_set_text_fmt(hexStr, "%02X%02X%02X",
                          (uint16_t)r, (uint16_t)g, (uint16_t)b);
  }
  
 public:
  ColorEditorPopup(Window* parent, uint32_t color,
                   std::function<void(uint32_t)> _setValue) :
    Dialog(parent, STR_COLOR_PICKER, rect_t{})
  {
    lv_obj_set_style_bg_color(content->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY3), 0);

    auto form = &content->form;

    FlexGridLayout grid(col_dsc, row_dsc);
    auto line = form->newLine(&grid);

    rect_t r{ 0, 0, 7 * LV_DPI_DEF / 5, 7 * LV_DPI_DEF / 5 };
    auto cedit = new ColorEditor(line, r, color, [=](uint32_t c) { updateColor(c); });
    lv_obj_set_style_grid_cell_x_align(cedit->getLvObj(), LV_GRID_ALIGN_CENTER, 0);

    auto vbox = new FormWindow(line, rect_t{});
    lv_obj_set_style_grid_cell_x_align(vbox->getLvObj(), LV_GRID_ALIGN_CENTER, 0);
    vbox->setFlexLayout(LV_FLEX_FLOW_COLUMN, lv_dpx(8));
    vbox->setWidth(r.w);
    vbox->setHeight(r.h);

    auto hbox = new FormWindow(vbox, rect_t{});
    hbox->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));
    auto hbox_obj = hbox->getLvObj();
    lv_obj_set_flex_align(hbox_obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_AROUND);

    colorPad = window_create(hbox_obj);
    lv_obj_set_style_bg_opa(colorPad, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_size(colorPad, COLOR_PAD_WIDTH, COLOR_PAD_HEIGHT);

    hexStr = lv_label_create(hbox_obj);
    lv_obj_set_style_text_font(hexStr, getFont(FONT(L)), LV_PART_MAIN);
    lv_obj_set_style_text_color(hexStr, makeLvColor(COLOR_THEME_PRIMARY1), LV_PART_MAIN);

    updateColor(color);
    
    hbox = new FormWindow(vbox, rect_t{});
    hbox->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));
    lv_obj_set_flex_align(hbox->getLvObj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

    auto rgbBtn = new TextButton(hbox, rect_t{}, "RGB");
    auto hsvBtn = new TextButton(hbox, rect_t{}, "HSV");
    auto thmBtn = new TextButton(hbox, rect_t{}, "SYS");

    rgbBtn->setPressHandler([=]() {
      cedit->setColorEditorType(RGB_COLOR_EDITOR);
      hsvBtn->check(false);
      thmBtn->check(false);
      return 1;
    });
    rgbBtn->padAll(lv_dpx(8));

    hsvBtn->setPressHandler([=]() {
      cedit->setColorEditorType(HSV_COLOR_EDITOR);
      rgbBtn->check(false);
      thmBtn->check(false);
      return 1;
    });
    hsvBtn->padAll(lv_dpx(8));

    thmBtn->setPressHandler([=]() {
      cedit->setColorEditorType(THM_COLOR_EDITOR);
      rgbBtn->check(false);
      hsvBtn->check(false);
      return 1;
    });
    thmBtn->padAll(lv_dpx(8));

    // color editor defaults to HSV
    hsvBtn->check(true);
    
    hbox = new FormWindow(vbox, rect_t{});
    hbox->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));
    lv_obj_set_flex_align(hbox->getLvObj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_obj_set_flex_grow(hbox->getLvObj(), 1);

    auto cancelBtn = new TextButton(hbox, rect_t{}, STR_CANCEL, [=]() -> int8_t {
      this->deleteLater();
      return 0;
    });
    cancelBtn->padTop(lv_dpx(12));
    cancelBtn->padBottom(lv_dpx(12));
    lv_obj_set_flex_grow(cancelBtn->getLvObj(), 1);

    auto okBtn = new TextButton(hbox, rect_t{}, STR_SAVE, [=]() -> int8_t {
      if (_setValue) _setValue(m_color);
      this->deleteLater();
      return 0;
    });
    okBtn->padTop(lv_dpx(12));
    okBtn->padBottom(lv_dpx(12));
    lv_obj_set_flex_grow(okBtn->getLvObj(), 1);

#if LCD_W > LCD_H
    content->setWidth(LCD_W * 0.9);
#else
    content->setWidth(LCD_W * 0.8);
#endif
    content->updateSize();
  }
};

static void color_editor_popup(lv_event_t* e)
{
  auto picker = (ColorPicker*)lv_event_get_user_data(e);
  if (picker) {
    new ColorEditorPopup(picker, picker->getColor(),
                         [=](uint32_t c) { picker->setColor(c); });
  }
}

ColorPicker::ColorPicker(Window* parent, const rect_t& rect,
                         std::function<uint16_t()> getValue,
                         std::function<void(uint16_t)> setValue) :
    FormField(parent, rect, 0, 0, etx_button_create),
    setValue(std::move(setValue))
{
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_STATE_FOCUS_KEY);

  lv_obj_set_size(lvobj, COLOR_PAD_WIDTH, COLOR_PAD_HEIGHT);
  lv_obj_add_event_cb(lvobj, color_editor_popup, LV_EVENT_CLICKED, this);

  updateColor(getValue());
}

void ColorPicker::setColor(uint32_t c)
{
  setValue(c);
  updateColor(c);
}

void ColorPicker::updateColor(uint32_t c)
{
  color = c;

  auto lvcolor = lv_color_make(GET_RED(color), GET_GREEN(color), GET_BLUE(color));
  lv_obj_set_style_bg_color(lvobj, lvcolor, LV_PART_MAIN);
  lv_obj_set_style_bg_color(lvobj, lvcolor, LV_STATE_FOCUS_KEY);
}
