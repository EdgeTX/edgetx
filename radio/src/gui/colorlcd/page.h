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

#ifndef _PAGE_H_
#define _PAGE_H_

#include "window.h"
#include "button.h"

class Page;

class PageHeader: public FormGroup
{
  public:
    PageHeader(Page * parent, uint8_t icon);

#if defined(HARDWARE_TOUCH)
    void deleteLater(bool detach = true, bool trash = true) override
    {
      if (_deleted)
        return;

      back.deleteLater(true, false);
      FormGroup::deleteLater(detach, trash);
    }
#endif

    void onEvent(event_t event) override
    {
      if (event == EVT_KEY_BREAK(KEY_PGUP)) {
        onKeyPress();
        FormField *focus = dynamic_cast<FormField *>(getFocus());
        if (focus != nullptr && focus->getPreviousField()) {
          focus->getPreviousField()->setFocus(SET_FOCUS_BACKWARD, focus);
        }
      } else if (event == EVT_KEY_BREAK(KEY_PGDN)) {
        onKeyPress();
        FormField *focus = dynamic_cast<FormField *>(getFocus());
        if (focus != nullptr && focus->getNextField()) {
          focus->getNextField()->setFocus(SET_FOCUS_FORWARD, focus);
        }
      } else if (event == EVT_KEY_FIRST(KEY_EXIT)) {
        killEvents(event);
        parent->deleteLater();  // destroy the Page
      } 
      else {
        FormGroup::onEvent(event);
      }
    }

    void paint(BitmapBuffer * dc) override;

    uint8_t getIcon() const
    {
      return icon;
    }

  protected:
    uint8_t icon;
#if defined(HARDWARE_TOUCH)
    Button back;
#endif
};

class Page: public Window
{
  public:
    explicit Page(unsigned icon);

    void deleteLater(bool detach = true, bool trash = true) override;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Page";
    }
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override
    {
      Window::onTouchStart(x, y);
      return true;
    }

    bool onTouchEnd(coord_t x, coord_t y) override;

    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override
    {
      Window::onTouchSlide(x, y, startX, startY, slideX, slideY);
      return true;
    }
#endif

    void setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr) override;

    void paint(BitmapBuffer * dc) override;

  protected:
    PageHeader header;
    FormWindow body;
};

#endif // _PAGE_H_
