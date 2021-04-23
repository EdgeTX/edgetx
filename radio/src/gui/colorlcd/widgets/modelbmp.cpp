/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
    ModelBitmapWidget(const WidgetFactory * factory, FormGroup * parent, const rect_t & rect, Widget::PersistentData * persistentData):
      Widget(factory, parent, rect, persistentData)
    {
    }

    void paint(BitmapBuffer * dc) override
    {
      // big space to draw
      if (rect.h >= 96 && rect.w >= 120) {

        dc->drawFilledRect(0, 0, rect.w, rect.h, SOLID, MAINVIEW_PANES_COLOR, OPACITY(5));
      
        auto iconMask = theme->getIconMask(ICON_MODEL);
        if (iconMask) {
          dc->drawMask(6, 4, iconMask, MAINVIEW_GRAPHICS_COLOR);
        }

        dc->drawSizedText(45, 10, g_model.header.name, LEN_MODEL_NAME, FONT(XS));
        dc->drawSolidFilledRect(39, 27, rect.w - 48, 2, MAINVIEW_GRAPHICS_COLOR);

        if (buffer) {
          dc->drawScaledBitmap(buffer.get(), 0, 38, rect.w, rect.h - 38);
        }
      }
      // smaller space to draw
      else if (buffer) {
        dc->drawScaledBitmap(buffer.get(), 0, 0, rect.w, rect.h);
      }
    }

    void checkEvents() override
    {
      Widget::checkEvents();
      
      uint32_t new_hash = hash(g_model.header.bitmap, sizeof(g_model.header.bitmap));
      new_hash ^= hash(g_model.header.name, sizeof(g_model.header.name));
      new_hash ^= hash(g_eeGeneral.themeName, sizeof(g_eeGeneral.themeName));

      if (new_hash != deps_hash) {
        deps_hash = new_hash;
        loadBitmap();
      }
    }

  protected:
    std::unique_ptr<BitmapBuffer> buffer;
    uint32_t deps_hash = 0;

    void loadBitmap()
    {
      std::string filename = std::string(g_model.header.bitmap);
      std::string fullpath = std::string(BITMAPS_PATH PATH_SEPARATOR) + filename;

      buffer.reset(BitmapBuffer::loadBitmap(fullpath.c_str()));
      if (!buffer) {
        TRACE("could not load bitmap '%s'", filename.c_str());
        return;
      }
    }
};

BaseWidgetFactory<ModelBitmapWidget> modelBitmapWidget("ModelBmp", nullptr);
const WidgetFactory * defaultWidget = &modelBitmapWidget;
