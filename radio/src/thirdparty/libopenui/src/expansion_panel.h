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

#ifndef _EXPANSION_PANEL_H_
#define _EXPANSION_PANEL_H_

#include "form.h"

class ExpansionPanel;

class ExpansionPanelHeader: virtual public FormGroup {
  public:
    explicit ExpansionPanelHeader(ExpansionPanel * parent);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ExpansionPanelHeader";
    }
#endif

    void onEvent(event_t event) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t, coord_t) override;
#endif
};


class ExpansionPanel: public FormGroup {
  friend class ExpansionPanelHeader;

  public:
    ExpansionPanel(Window * parent, const rect_t & rect):
      FormGroup(parent, rect, FORM_NO_BORDER | FORM_FORWARD_FOCUS | FORWARD_SCROLL)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ExpansionPanel";
    }
#endif

    void toggle()
    {
      open(!isOpen);
    }

    void open(bool state = true)
    {
      isOpen = state;
      updateHeight();
    }

    void updateHeight()
    {
      coord_t previousHeight = height();
      coord_t previousBottom = bottom();
      body->adjustInnerHeight();
      coord_t newHeight = (isOpen ? header->height() + body->height() : header->height());
      parent->moveWindowsTop(previousBottom, newHeight - previousHeight);
      setHeight(newHeight);
    }

    void setFocus(uint8_t flag) override
    {
      if (!isOpen)
        header->setFocus(flag);
      else
        FormGroup::setFocus(flag);
    }

    FormGroup * body = nullptr;

  protected:
    Window * header = nullptr;
    bool isOpen = false;
};

#endif // _EXPANSION_PANEL_H_
