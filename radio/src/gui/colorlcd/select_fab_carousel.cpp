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

#include "select_fab_carousel.h"

#include "bitmaps.h"
#include "button.h"
#include "static.h"

static void etx_quick_button_constructor(const lv_obj_class_t* class_p,
                                         lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->border_transparent, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->rounded, LV_PART_MAIN);
  etx_txt_color(obj, COLOR_WHITE_INDEX, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->pad_medium, LV_PART_MAIN);

  etx_obj_add_style(obj, styles->border, LV_PART_MAIN | LV_STATE_FOCUSED);
  etx_obj_add_style(obj, styles->border_color[COLOR_WHITE_INDEX],
                    LV_PART_MAIN | LV_STATE_FOCUSED);
}

static const lv_obj_class_t etx_quick_button_class = {
    .base_class = &lv_btn_class,
    .constructor_cb = etx_quick_button_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = SelectFabCarousel::FAB_BUTTON_WIDTH,
    .height_def = SelectFabCarousel::FAB_BUTTON_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_btn_t),
};

static lv_obj_t* etx_quick_button_create(lv_obj_t* parent)
{
  return etx_create(&etx_quick_button_class, parent);
}

static void etx_quick_icon_constructor(const lv_obj_class_t* class_p,
                                       lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->border, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->border_color[COLOR_WHITE_INDEX], LV_PART_MAIN);
  etx_obj_add_style(obj, styles->outline, LV_PART_MAIN);
  lv_obj_set_style_outline_color(obj, lv_color_black(), LV_PART_MAIN);
  etx_obj_add_style(obj, styles->circle, LV_PART_MAIN);
  etx_solid_bg(obj, COLOR_BLACK_INDEX, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->pad_zero, LV_PART_MAIN);
}

static const lv_obj_class_t etx_quick_icon_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = etx_quick_icon_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = SelectFabCarousel::FAB_ICON_SIZE,
    .height_def = SelectFabCarousel::FAB_ICON_SIZE,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_obj_t),
};

static lv_obj_t* etx_quick_icon_create(lv_obj_t* parent)
{
  return etx_create(&etx_quick_icon_class, parent);
}

class SelectFabButton : public ButtonBase
{
 public:
  SelectFabButton(Window* parent, EdgeTxIcon icon, const char* title,
                  std::function<uint8_t(void)> pressHandler) :
      ButtonBase(parent, {}, pressHandler, etx_quick_button_create)
  {
    padAll(PAD_ZERO);

    auto iconLayout =
        new Window(this,
                   {(SelectFabCarousel::FAB_BUTTON_INNER_WIDTH - SelectFabCarousel::FAB_ICON_SIZE) / 2,
                    (SelectFabCarousel::FAB_BUTTON_INNER_WIDTH - SelectFabCarousel::FAB_ICON_SIZE) / 2 - 2,
                    SelectFabCarousel::FAB_ICON_SIZE, SelectFabCarousel::FAB_ICON_SIZE},
                   etx_quick_icon_create);
    iconLayout->setWindowFlag(NO_FOCUS);

    (new StaticIcon(iconLayout, 0, 0, icon, COLOR_WHITE_INDEX))
        ->center(SelectFabCarousel::FAB_ICON_SIZE - 4, SelectFabCarousel::FAB_ICON_SIZE - 4);

    new StaticText(this, {0, SelectFabCarousel::FAB_BUTTON_HEIGHT - SelectFabCarousel::FAB_TXT_YO, SelectFabCarousel::FAB_BUTTON_INNER_WIDTH, 0},
                   title, COLOR_WHITE_INDEX, CENTERED);
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SelectFabButton"; }
#endif

  void onEvent(event_t event) override
  {
#if defined(HARDWARE_KEYS)
    switch (event) {
      case EVT_KEY_BREAK(KEY_EXIT):
        parent->deleteLater();
        return;
    }
#endif
    ButtonBase::onEvent(event);
  }
};

SelectFabCarousel::SelectFabCarousel(Window* parent) : Window(parent, rect_t{})
{
  padAll(PAD_ZERO);
  setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_ZERO);
}

void SelectFabCarousel::addButton(EdgeTxIcon icon, const char* title,
                                  std::function<uint8_t(void)> pressHandler)
{
  new SelectFabButton(this, icon, title, pressHandler);
}
