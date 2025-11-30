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

#include "color_list.h"
#include "dialog.h"
#include "etx_lv_theme.h"

#if LANDSCAPE
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

class ColorEditorPopup : public BaseDialog
{
  ColorSwatch* colorPad = nullptr;
  StaticText* hexStr = nullptr;
  uint32_t m_color;
  uint32_t origColor;
  std::function<void(uint32_t)> setValue;
  COLOR_EDITOR_FMT format;

  void updateColor(uint32_t c)
  {
    m_color = c;

    uint8_t r, g, b;

    if (format == ETX_RGB565) {
      auto rgb = COLOR_VAL(colorToRGB(m_color));
      r = GET_RED(rgb); g = GET_GREEN(rgb); b = GET_BLUE(rgb);
    } else {
      auto rgb = color32ToRGB(m_color);
      r = GET_RED32(rgb); g = GET_GREEN32(rgb); b = GET_BLUE32(rgb);
    }

    if (colorPad)
      colorPad->setColor(r, g, b);

    char s[10];
    sprintf(s, "%02X%02X%02X", r, g, b);
    if (hexStr)
      hexStr->setText(s);
  }

  void onCancel() override
  {
    if (setValue) setValue(origColor);
    this->deleteLater();
  }

 public:
  ColorEditorPopup(uint32_t color,
                   std::function<void(uint32_t)> _setValue,
                   std::function<void(uint32_t)> _preview,
                   COLOR_EDITOR_FMT fmt) :
      BaseDialog(STR_COLOR_PICKER, false, COLOR_EDIT_WIDTH, COLOR_EDIT_HEIGHT),
      origColor(color), setValue(std::move(_setValue)), format(fmt)
  {
    FlexGridLayout grid(col_dsc, row_dsc);
    auto line = form->newLine(grid);

    rect_t r{0, 0, CE_SZ, CE_SZ};
    auto cedit = new ColorEditor(line, r, color, [=](uint32_t c) { updateColor(c); }, _preview, format, THM_COLOR_EDITOR);
    lv_obj_set_style_grid_cell_x_align(cedit->getLvObj(), LV_GRID_ALIGN_CENTER, 0);

    auto vbox = new Window(line, rect_t{});
    lv_obj_set_style_grid_cell_x_align(vbox->getLvObj(), LV_GRID_ALIGN_CENTER, 0);
    vbox->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_MEDIUM, r.w, r.h);

    auto hbox = new Window(vbox, rect_t{});
    hbox->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_MEDIUM);
    auto hbox_obj = hbox->getLvObj();
    lv_obj_set_flex_align(hbox_obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_SPACE_AROUND);

    colorPad = new ColorSwatch(hbox, {0, 0, COLOR_PAD_WIDTH, EdgeTxStyles::UI_ELEMENT_HEIGHT},
                               COLOR_THEME_PRIMARY1);

    hexStr = new StaticText(hbox, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH, 0}, "", COLOR_THEME_PRIMARY1_INDEX, FONT(L));

    updateColor(color);

    hbox = new Window(vbox, rect_t{});
    hbox->padAll(PAD_TINY);
    hbox->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_MEDIUM);
    lv_obj_set_flex_align(hbox->getLvObj(), LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

    auto thmBtn = new TextButton(hbox, {0, 0, BTN_W, 0}, STR_THEME);
    auto fxdBtn = new TextButton(hbox, {0, 0, BTN_W, 0}, STR_FIXED);
    auto hsvBtn = new TextButton(hbox, {0, 0, BTN_W, 0}, "HSV");
    auto rgbBtn = new TextButton(hbox, {0, 0, BTN_W, 0}, "RGB");

    rgbBtn->setPressHandler([=]() {
      cedit->setColorEditorType(RGB_COLOR_EDITOR);
      hsvBtn->check(false);
      thmBtn->check(false);
      fxdBtn->check(false);
      return 1;
    });

    hsvBtn->setPressHandler([=]() {
      cedit->setColorEditorType(HSV_COLOR_EDITOR);
      rgbBtn->check(false);
      thmBtn->check(false);
      fxdBtn->check(false);
      return 1;
    });

    thmBtn->setPressHandler([=]() {
      cedit->setColorEditorType(THM_COLOR_EDITOR);
      rgbBtn->check(false);
      hsvBtn->check(false);
      fxdBtn->check(false);
      return 1;
    });

    fxdBtn->setPressHandler([=]() {
      cedit->setColorEditorType(FXD_COLOR_EDITOR);
      rgbBtn->check(false);
      hsvBtn->check(false);
      thmBtn->check(false);
      return 1;
    });

    // color editor defaults to HSV
    thmBtn->check(true);
    lv_group_focus_obj(thmBtn->getLvObj());

    hbox = new Window(vbox, {0, height() - EdgeTxStyles::UI_ELEMENT_HEIGHT - PAD_SMALL, 0, 0});
    hbox->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_MEDIUM);
    lv_obj_set_flex_align(hbox->getLvObj(), LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_obj_set_flex_grow(hbox->getLvObj(), 1);

    new TextButton(hbox, rect_t{0, 0, BTN_W, 0}, STR_CANCEL, [=]() -> int8_t {
      onCancel();
      return 0;
    });

    new TextButton(hbox, rect_t{0, 0, BTN_W, 0}, STR_SAVE, [=]() -> int8_t {
      if (setValue) setValue(m_color);
      this->deleteLater();
      return 0;
    });
  }

  static LAYOUT_VAL_SCALED(CE_SZ, 182)
#if NARROW_LAYOUT
  static LAYOUT_ORIENTATION(COLOR_EDIT_WIDTH, LCD_W * 0.95, LCD_W * 0.7)
#else
  static LAYOUT_ORIENTATION(COLOR_EDIT_WIDTH, LCD_W * 0.8, LCD_W * 0.7)
#endif
  static LAYOUT_ORIENTATION(COLOR_EDIT_HEIGHT, LCD_H * 0.9, LV_SIZE_CONTENT)
  static LAYOUT_VAL_SCALED(COLOR_PAD_WIDTH, 52)
  static LAYOUT_VAL_SCALED(BTN_W, 80)
  static LAYOUT_VAL_SCALED(BTN_PAD_TOP, 60)
};

ColorPicker::ColorPicker(Window* parent, const rect_t& rect,
                         std::function<uint32_t()> getValue,
                         std::function<void(uint32_t)> setValue,
                         std::function<void(uint32_t)> preview,
                         COLOR_EDITOR_FMT fmt) :
    Button(parent, {rect.x, rect.y, rect.w == 0 ? ColorEditorPopup::COLOR_PAD_WIDTH : rect.w, EdgeTxStyles::UI_ELEMENT_HEIGHT}),
    setValue(std::move(setValue)), preview(std::move(preview)), format(fmt)
{
  updateColor(getValue());
}

void ColorPicker::onClicked()
{
  new ColorEditorPopup(getColor(), [=](uint32_t c) { setColor(c); }, preview, format);
}

void ColorPicker::setColor(uint32_t c)
{
  setValue(c);
  updateColor(c);
}

void ColorPicker::updateColor(uint32_t c)
{
  color = c;

  uint8_t r, g, b;

  if (format == ETX_RGB565) {
    auto rgb = COLOR_VAL(colorToRGB(color));
    r = GET_RED(rgb); g = GET_GREEN(rgb); b = GET_BLUE(rgb);
  } else {
    auto rgb = color32ToRGB(color);
    r = GET_RED32(rgb); g = GET_GREEN32(rgb); b = GET_BLUE32(rgb);
  }

  auto lvcolor = lv_color_make(r, g, b);
  lv_obj_set_style_bg_color(lvobj, lvcolor, LV_PART_MAIN);
}
