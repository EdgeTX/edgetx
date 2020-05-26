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

#include "expansion_panel.h"

void ExpansionPanel::updateHeight()
{
  coord_t newHeight = (isOpen ? header->height() + body->height() : header->height());
  parent->moveWindowsTop(bottom(), newHeight - height());
  setHeight(newHeight);
}

ExpansionPanelHeader::ExpansionPanelHeader(ExpansionPanel * parent):
  FormGroup(parent, {0, 0, parent->width(), parent->height()}, FORWARD_SCROLL)
{
}

void ExpansionPanelHeader::onEvent(event_t event)
{
  auto panel = static_cast<ExpansionPanel *>(parent);

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

#if defined(HARDWARE_TOUCH)
bool ExpansionPanelHeader::onTouchEnd(coord_t, coord_t)
{
  static_cast<ExpansionPanel *>(parent)->toggle();
  setFocus(SET_FOCUS_DEFAULT);
  return true;
}
#endif
