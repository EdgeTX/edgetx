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

#include "opentx.h"
#include "widgets_container_impl.h"

#include <memory>

class ModelBitmapWidget: public Widget
{
  public:
    ModelBitmapWidget(const WidgetFactory* factory, Window* parent, const rect_t & rect, Widget::PersistentData* persistentData):
      Widget(factory, parent, rect, persistentData)
    {
      loadBitmap();
    }

    void refresh(BitmapBuffer * dc) override
    {
      std::string filename = std::string(g_model.header.bitmap);

      // set font colour from options[0], if use theme color option off
      LcdFlags fontColor;
      if (persistentData->options[4].value.boolValue)
        fontColor = COLOR_THEME_SECONDARY1;
      else
        fontColor = COLOR2FLAGS(persistentData->options[0].value.unsignedValue);

      // get font size from options[1]
      LcdFlags fontSize = persistentData->options[1].value.unsignedValue << 8u;

      // fill bg from options[3] if options[2] set
      if (persistentData->options[2].value.boolValue) {
        LcdFlags fillColour = COLOR2FLAGS(persistentData->options[3].value.unsignedValue);
        dc->drawSolidFilledRect(0, 0, width(), height(), fillColour);
      }

      if (buffer &&
          ((buffer->width() != width()) || (buffer->height() != height()) ||
           (deps_hash != getHash()))) {

        loadBitmap();
        deps_hash = getHash();
      }

      // big space to draw
      if (rect.h >= 96 && rect.w >= 120) {

        if (!filename.empty() && buffer) {
          dc->drawBitmap(0, 38, buffer.get());
        }

        dc->drawSizedText(5, 5, g_model.header.name, LEN_MODEL_NAME, fontSize | fontColor);
      }
      // smaller space to draw
      else {
        if (!filename.empty() && buffer) {
          dc->drawBitmap(0, 0, buffer.get());
        }
        else {
          dc->drawSizedText(0, 0, g_model.header.name, LEN_MODEL_NAME, fontSize | fontColor);
        }
      }
    }

    void checkEvents() override
    {
      Widget::checkEvents();
      if (getHash() != deps_hash) {
        invalidate();
      }
    }

    static const ZoneOption options[];

  protected:
    std::unique_ptr<BitmapBuffer> buffer;
    uint32_t deps_hash = 0;

    uint32_t getHash()
    {
      return hash(g_model.header.bitmap, sizeof(g_model.header.bitmap));
    }
  
    void loadBitmap()
    {
      std::string filename = std::string(g_model.header.bitmap);
      std::string fullpath = std::string(BITMAPS_PATH PATH_SEPARATOR) + filename;

      if (!buffer || (buffer->width() != width()) || (buffer->height() != height())) {
        buffer.reset(new BitmapBuffer(BMP_ARGB4444, width(), height()));
      }

      buffer->clear();
      if (!filename.empty()) {
        std::unique_ptr<BitmapBuffer> bitmap(BitmapBuffer::loadBitmap(fullpath.c_str()));
        if (!bitmap) {
          TRACE("could not load bitmap '%s'", filename.c_str());
          return;
        }

        if (rect.h >= 96 && rect.w >= 120) {
          buffer->drawScaledBitmap(bitmap.get(), 0, 0, width(), height() - 38);
        } else {
          buffer->drawScaledBitmap(bitmap.get(), 0, 0, width(), height());
        }
      }
    }
};

const ZoneOption ModelBitmapWidget::options[] = {
    {STR_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(COLOR_THEME_SECONDARY1>>16)},
    {STR_SIZE, ZoneOption::TextSize, OPTION_VALUE_UNSIGNED(FONT_STD_INDEX)},
    {STR_FILL_BACKGROUND, ZoneOption::Bool, OPTION_VALUE_BOOL(false)},
    {STR_BG_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(COLOR_THEME_SECONDARY3>>16)},
    {STR_USE_THEME_COLOR, ZoneOption::Bool, OPTION_VALUE_BOOL(true)},
    {nullptr, ZoneOption::Bool}};

BaseWidgetFactory<ModelBitmapWidget> modelBitmapWidget("ModelBmp", ModelBitmapWidget::options, "Models");
