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

#include <memory>

#include "edgetx.h"
#include "widget.h"

#define ETX_STATE_BG_FILL LV_STATE_USER_1

class ModelBitmapWidget : public Widget
{
 public:
  ModelBitmapWidget(const WidgetFactory* factory, Window* parent,
                    const rect_t& rect,
                    Widget::PersistentData* persistentData) :
      Widget(factory, parent, rect, persistentData)
  {
    etx_obj_add_style(lvobj, styles->bg_opacity_transparent, LV_PART_MAIN);
    etx_obj_add_style(lvobj, styles->bg_opacity_cover,
                      LV_PART_MAIN | ETX_STATE_BG_FILL);

    label = new StaticText(this, rect_t{}, g_model.header.name);
    label->hide();

    image = new StaticImage(this, rect_t{0, 0, width(), height()});
    image->hide();

    update();
  }

  void checkEvents() override
  {
    Widget::checkEvents();

    if (getHash() != deps_hash) {
      update();
      // Force bitmap redraw
      invalidate();
    }

    if (label->getText() != g_model.header.name)
      label->setText(g_model.header.name);
  }

  void update() override
  {
    isLarge = rect.h >= 96 && rect.w >= 120;

    // get font size from options[1]
    etx_font(label->getLvObj(),
             (FontIndex)persistentData->options[1].value.unsignedValue);

    // set font colour from options[0], if use theme color option off
    if (persistentData->options[4].value.boolValue) {
      etx_txt_color(label->getLvObj(), COLOR_THEME_SECONDARY1_INDEX,
                    LV_PART_MAIN);
    } else {
      etx_txt_color_from_flags(label->getLvObj(), persistentData->options[0].value.unsignedValue);
    }

    // Set label position
    if (isLarge)
      lv_obj_set_pos(label->getLvObj(), 5, 5);
    else
      lv_obj_set_pos(label->getLvObj(), 0, 0);

    // get fill color from options[3]
    etx_bg_color_from_flags(lvobj, persistentData->options[3].value.unsignedValue);

    // Set background opacity from options[2]
    if (persistentData->options[2].value.boolValue)
      lv_obj_add_state(lvobj, ETX_STATE_BG_FILL);
    else
      lv_obj_clear_state(lvobj, ETX_STATE_BG_FILL);

    if (!image->hasImage() || deps_hash != getHash()) {
      if (g_model.header.bitmap[0]) {
        char filename[LEN_BITMAP_NAME + 1];
        strAppend(filename, g_model.header.bitmap, LEN_BITMAP_NAME);
        std::string fullpath =
            std::string(BITMAPS_PATH PATH_SEPARATOR) + filename;

        image->setSource(fullpath);
      } else {
        image->clearSource();
      }
      deps_hash = getHash();
    }

    image->setRect(
        {0, isLarge ? 38 : 0, width(), height() - (isLarge ? 38 : 0)});
    image->show(image->hasImage());
    image->setZoom();

    label->show(isLarge || !image->hasImage());
  }

  static const ZoneOption options[];

 protected:
  bool isLarge = false;
  uint32_t deps_hash = 0;
  StaticText* label = nullptr;
  StaticImage* image = nullptr;

  uint32_t getHash() { return hash(g_model.header.bitmap, LEN_BITMAP_NAME); }
};

const ZoneOption ModelBitmapWidget::options[] = {
    {STR_COLOR, ZoneOption::Color, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX)},
    {STR_SIZE, ZoneOption::TextSize, OPTION_VALUE_UNSIGNED(FONT_STD_INDEX)},
    {STR_FILL_BACKGROUND, ZoneOption::Bool, OPTION_VALUE_BOOL(false)},
    {STR_BG_COLOR, ZoneOption::Color, COLOR2FLAGS(COLOR_THEME_SECONDARY3_INDEX)},
    {STR_USE_THEME_COLOR, ZoneOption::Bool, OPTION_VALUE_BOOL(true)},
    {nullptr, ZoneOption::Bool}};

BaseWidgetFactory<ModelBitmapWidget> modelBitmapWidget(
    "ModelBmp", ModelBitmapWidget::options, STR_WIDGET_MODELBMP);
