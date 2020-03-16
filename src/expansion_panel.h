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
    ExpansionPanelHeader(ExpansionPanel * parent);

    void onEvent(event_t event) override;

    bool onTouchEnd(coord_t, coord_t) override;
};


class ExpansionPanel: public FormGroup {
  friend class ExpansionPanelHeader;

  public:
    ExpansionPanel(Window * parent, const rect_t & rect):
      FormGroup(parent, rect, FORM_NO_BORDER | FORM_FORWARD_FOCUS)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "ExpansionPanel()";
    }
#endif

    void toggle()
    {
      open(!isOpen);
    }

    void open(bool state = true)
    {
      isOpen = state;
      coord_t newHeight = (state ? header->height() + body->height() : header->height());
      parent->moveWindowsTop(bottom(), newHeight - height());
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

ExpansionPanelHeader::ExpansionPanelHeader(ExpansionPanel * parent):
  FormGroup(parent, {0, 0, parent->width(), parent->height()}, FORWARD_SCROLL)
{
}

bool ExpansionPanelHeader::onTouchEnd(coord_t, coord_t)
{
  static_cast<ExpansionPanel *>(parent)->toggle();
  setFocus();
  return true;
}

void ExpansionPanelHeader::onEvent(event_t event)
{
  ExpansionPanel * panel = static_cast<ExpansionPanel *>(parent);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    panel->toggle();
  }
  else if (event == EVT_ROTARY_RIGHT && !panel->isOpen) {
    panel->getNextField()->setFocus(SET_FOCUS_FORWARD);
  }
  else {
    FormGroup::onEvent(event);
  }
}

#endif // _EXPANSION_PANEL_H_
