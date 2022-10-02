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

#include "menu.h"
#include "font.h"
#include "theme.h"

#include <lvgl/lvgl.h>

void MenuBody::onDrawBegin(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc)
{
  lv_canvas_t* icon = (lv_canvas_t*)lines[row].getIcon();
  if (!icon) return;

  lv_img_t* img = &icon->img;
  lv_coord_t cell_left = lv_obj_get_style_pad_left(lvobj, LV_PART_ITEMS);
  dsc->label_dsc->ofs_x = img->w + cell_left;
}

void MenuBody::onDrawEnd(uint16_t row, uint16_t col, lv_obj_draw_part_dsc_t* dsc)
{
  if(row >= lines.size())  return;

  lv_obj_t* icon = lines[row].getIcon();
  if (icon) {
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);
    
    lv_img_dsc_t* img = lv_canvas_get_img(icon);
    lv_area_t coords;

    lv_coord_t area_h = lv_area_get_height(dsc->draw_area);

    lv_coord_t cell_left = lv_obj_get_style_pad_left(lvobj, LV_PART_ITEMS);
    coords.x1 = dsc->draw_area->x1 + cell_left;
    coords.x2 = coords.x1 + img->header.w - 1;
    coords.y1 = dsc->draw_area->y1 + (area_h - img->header.h) / 2;
    coords.y2 = coords.y1 + img->header.h - 1;
    
    lv_draw_img(dsc->draw_ctx, &img_dsc, &coords, img);
  }

  if(lines[row].isChecked != nullptr && 
     lines[row].isChecked()) {    
    lv_area_t coords;
    lv_coord_t area_h = lv_area_get_height(dsc->draw_area);
    lv_coord_t cell_right = lv_obj_get_style_pad_right(lvobj, LV_PART_ITEMS);
    lv_coord_t font_h = getFontHeight(FONT(STD));
    coords.x1 = dsc->draw_area->x2 - cell_right - font_h;
    coords.x2 = coords.x1 + font_h;
    coords.y1 = dsc->draw_area->y1 + (area_h - font_h) / 2;
    coords.y2 = coords.y1 + font_h - 1;
    lv_draw_label(dsc->draw_ctx, dsc->label_dsc, &coords, LV_SYMBOL_OK, nullptr);
  }
}

static void _force_editing(lv_group_t* g)
{
  lv_group_set_editing(g, true);
}

MenuBody::MenuBody(Window * parent, const rect_t & rect):
  TableField(parent, rect)
{
  // Allow encoder acceleration
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_ENCODER_ACCEL);

  setColumnCount(1);
  setColumnWidth(0, rect.w);

  lv_group_t* g = (lv_group_t*)lv_obj_get_group(lvobj);
  if (g) {
    setFocusHandler([=](bool focus) {
      if (focus) {
        lv_group_set_focus_cb(g, _force_editing);
      } else {
        lv_group_set_focus_cb(g, nullptr);
      }
    });
    lv_group_set_editing(g, true);
  }
}

void MenuBody::addLine(const std::string &text, std::function<void()> onPress,
                       std::function<bool()> isChecked)
{
  lines.emplace_back(std::move(onPress), std::move(isChecked), nullptr);

  auto idx = lines.size() - 1;
  lv_table_set_cell_value(lvobj, idx, 0, text.c_str());
}

void MenuBody::addLine(const uint8_t *icon_mask, const std::string &text,
                       std::function<void()> onPress,
                       std::function<bool()> isChecked)
{
  lv_obj_t* canvas = lv_canvas_create(nullptr);
  lines.emplace_back(std::move(onPress), std::move(isChecked), canvas);

  lv_coord_t w = *((uint16_t *)icon_mask);
  lv_coord_t h = *(((uint16_t *)icon_mask)+1);
  void* buf = (void*)(icon_mask + 4);
  lv_canvas_set_buffer(canvas, buf, w, h, LV_IMG_CF_ALPHA_8BIT);

  auto idx = lines.size() - 1;
  lv_table_set_cell_value_fmt(lvobj, idx, 0, text.c_str());
}

void MenuBody::removeLines()
{
  lines.clear();
  setRowCount(0);

  // reset vertical scroll
  lv_obj_scroll_to_y(lvobj, 0, LV_ANIM_OFF);
}

coord_t MenuBody::getContentHeight()
{
  return lv_obj_get_self_height(lvobj);
}

void MenuBody::onPress(size_t index)
{
  Menu *menu = getParentMenu();
  if (index < lines.size()) {
    if (menu->multiple) {
      if (selectedIndex == (int)index)
        lines[index].onPress();
      else {
        setIndex(index);
        lines[index].onPress();
      }
    } else {
      // delete menu first to avoid
      // focus issues with onPress()
      menu->deleteLater();
      lines[index].onPress();
    }
  }
}

// ensure index is in range and also handle wrapping index
int MenuBody::rangeCheck(int index)
{
  if (index < 0)
    index = lines.size() - 1;
  else if (index > (signed)lines.size() - 1)
    index = 0;

  return index;
}

void MenuBody::setIndex(int index)
{
  if (index < (int)lines.size()) {
    if (index == selectedIndex) return;
    selectedIndex = index;

    lv_obj_invalidate(lvobj);
    lv_table_t* table = (lv_table_t*)lvobj;
    
    table->row_act = index;
    table->col_act = 0;

    lv_coord_t h_before = 0;
    for (uint16_t i = 0; i < table->row_act; i++)
      h_before += table->row_h[i];

    lv_coord_t row_h = table->row_h[table->row_act];    
    lv_coord_t scroll_y = lv_obj_get_scroll_y(lvobj);

    lv_obj_update_layout(lvobj);
    lv_coord_t h = lv_obj_get_height(lvobj);

    lv_coord_t diff_y = 0;
    if (h_before < scroll_y) {
      diff_y = scroll_y - h_before;
    } else if (scroll_y + h < h_before + row_h) {
      diff_y = scroll_y + h - h_before - row_h;
    } else {
      return;
    }

    lv_obj_scroll_by_bounded(lvobj, 0, diff_y, LV_ANIM_OFF);
  }
}

void MenuBody::onPress(uint16_t row, uint16_t col)
{
  onPress(row);
}

void MenuBody::selectNext(MENU_DIRECTION direction)
{
  // look for the next non separator line
  int index = selectedIndex + direction;
  setIndex(rangeCheck(index));
}


void MenuBody::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    onCancel();
  }
  else {
    TableField::onEvent(event);
  }
#endif
}

void MenuBody::onCancel()
{
  if(_onCancel) _onCancel();
  TableField::onCancel();
}

MenuWindowContent::MenuWindowContent(Menu* parent) :
    ModalWindowContent(parent, {(LCD_W - MENUS_WIDTH) / 2,
                                (LCD_H - MENUS_WIDTH) / 2, MENUS_WIDTH, 0}),
    body(this, {0, 0, width(), height()})
{
  // body.setFocus(SET_FOCUS_DEFAULT);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);
}

coord_t MenuWindowContent::getHeaderHeight() const
{
  if (title) return lv_obj_get_height(title);
  return 0;
}

void MenuWindowContent::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;
  body.deleteLater(true, false);
  ModalWindowContent::deleteLater(detach, trash);
}

Menu::Menu(Window * parent, bool multiple):
  ModalWindow(parent, true),
  content(createMenuWindow(this)),
  multiple(multiple)
{
}

void Menu::setToolbar(Window * window)
{
  toolbar = window;
  updatePosition();
  toolbar->setLeft(content->left() - toolbar->width());
  toolbar->setTop(content->top());
  toolbar->setHeight(content->height());
}

void Menu::setOutline(Window* obj)
{
  lv_obj_set_style_outline_width(obj->getLvObj(), 4, LV_PART_MAIN);
  lv_obj_set_style_outline_color(obj->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY2), LV_PART_MAIN);
  lv_obj_set_style_outline_opa(obj->getLvObj(),  LV_OPA_100, LV_PART_MAIN);
  lv_obj_set_style_outline_pad(obj->getLvObj(),  1, LV_PART_MAIN);

}

void Menu::updatePosition()
{
  coord_t height = content->body.getContentHeight();

  if (toolbar) {
    content->setLeft((LCD_W - content->width() + toolbar->width())/2);
    setOutline(toolbar);
  } else {
    content->setLeft((LCD_W - content->width())/2);
    auto headerHeight = content->getHeaderHeight();
    auto bodyHeight = min<coord_t>(height, MENUS_MAX_HEIGHT - headerHeight);
    content->setTop((LCD_H - headerHeight - bodyHeight) / 2);
    content->setHeight(headerHeight + bodyHeight);
    content->body.setTop(headerHeight);
    content->body.setHeight(bodyHeight);
  }
  setOutline(content);
}

void Menu::setTitle(std::string text)
{
  content->setTitle(std::move(text));
  updatePosition();
}

void Menu::addLine(const std::string &text, std::function<void()> onPress,
                   std::function<bool()> isChecked)
{
  content->body.addLine(text, std::move(onPress), std::move(isChecked));
  updatePosition();
}

void Menu::addLine(const uint8_t *icon_mask, const std::string &text,
                   std::function<void()> onPress,
                   std::function<bool()> isChecked)
{
  content->body.addLine(icon_mask, text, std::move(onPress), std::move(isChecked));
  updatePosition();
}

void Menu::removeLines()
{
  content->body.removeLines();
  updatePosition();
}

void Menu::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  if (toolbar &&
      (event == EVT_KEY_BREAK(KEY_PGDN) || event == EVT_KEY_LONG(KEY_PGDN) ||
       event == EVT_KEY_BREAK(KEY_PGUP))) {
    toolbar->onEvent(event);
  }
#endif
}

void Menu::onCancel()
{
  deleteLater();
}
