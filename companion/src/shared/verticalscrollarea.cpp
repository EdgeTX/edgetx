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

#include "verticalscrollarea.h"
#include "genericpanel.h"
#include <QEvent>
#include <QScrollBar>

VerticalScrollArea::VerticalScrollArea(QWidget * parent, GenericPanel * panel):
  QScrollArea(parent),
  panel(panel)
{
  setWidgetResizable(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setWidget(panel);
  panel->installEventFilter(this);
}

bool VerticalScrollArea::eventFilter(QObject * o, QEvent * e)
{
  if (o == panel && e->type() == QEvent::Resize) {
    setMinimumWidth(panel->minimumSizeHint().width() + verticalScrollBar()->width());
  }
  return false;
}
