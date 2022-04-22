/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "coloredit.h"
#include "numberedit.h"
#include "bitfield.h"
#include "dialog.h"

#define PICKER_SIZE lv_colorwheel_class.width_def

struct ColorWheelPopup : public Dialog {
  ColorWheelPopup(Window* parent, lv_color_t color,
                  std::function<void(lv_color_t)> setValue) :
      Dialog(parent, std::string(),
             rect_t{0, 0, PICKER_SIZE,
                    PICKER_SIZE + POPUP_HEADER_HEIGHT})
  {
    auto lv_content = content->getLvObj();
    auto form = content->form.getLvObj();
    lv_obj_set_size(form, PICKER_SIZE, PICKER_SIZE);

    lv_obj_center(lv_content);
    lv_obj_center(form);

    auto cw = lv_colorwheel_create(form, true);
    lv_colorwheel_set_rgb(cw, color);

    auto wheel_size = 3 * PICKER_SIZE / 4;
    lv_obj_set_style_arc_width(cw, wheel_size / 5, 0);
    lv_obj_set_size(cw, wheel_size, wheel_size);
    lv_obj_center(cw);

    content->updateSize();

    setCloseWhenClickOutside(true);
    setCloseHandler([=]() {
      lv_color_t lv_color = lv_colorwheel_get_rgb(cw);
      setValue(lv_color);
    });
  }
};

static void colorwheel_popup(lv_event_t* e)
{
  auto edit = (ColorEdit*)lv_event_get_user_data(e);
  if (edit) {
    new ColorWheelPopup(edit, edit->getColor(),
                        [=](lv_color_t c) { edit->setColor(c); });
  }
}

// based on LVGL default switch size
constexpr lv_coord_t COLOR_PAD_WIDTH = (4 * LV_DPI_DEF) / 10;
constexpr lv_coord_t COLOR_PAD_HEIGHT = (4 * LV_DPI_DEF) / 17;

ColorEdit::ColorEdit(Window* parent, const rect_t& rect,
                     std::function<uint16_t()> getValue,
                     std::function<void(uint16_t)> setValue) :
  FormField(parent, rect, 0, 0, lv_btn_create),
  setValue(std::move(setValue))
{
  auto value = getValue();
  color = lv_color_make(GET_RED(value), GET_GREEN(value), GET_BLUE(value));

  lv_obj_set_style_bg_color(lvobj, color, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);

  lv_obj_set_style_bg_color(lvobj, color, LV_STATE_FOCUS_KEY);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_STATE_FOCUS_KEY);

  lv_obj_set_size(lvobj, COLOR_PAD_WIDTH, COLOR_PAD_HEIGHT);
  lv_obj_add_event_cb(lvobj, colorwheel_popup, LV_EVENT_CLICKED, this);
}

void ColorEdit::setColor(lv_color_t c)
{
  color = c;
  setValue(RGB_JOIN(c.ch.red, c.ch.green, c.ch.blue));

  lv_obj_set_style_bg_color(lvobj, color, LV_PART_MAIN);
  lv_obj_set_style_bg_color(lvobj, color, LV_STATE_FOCUS_KEY);
  
  // TRACE("### color: %d %d %d ###", c.ch.red, c.ch.green, c.ch.blue);
  invalidate();
}
