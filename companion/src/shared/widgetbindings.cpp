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

#include "widgetbindings.h"

void WidgetBindings::bindVisible(QWidget *widget, std::function<bool()> pred)
{
  m_visible.push_back({widget, std::move(pred)});
}

void WidgetBindings::bindEnabled(QWidget *widget, std::function<bool()> pred)
{
  m_enabled.push_back({widget, std::move(pred)});
}

void WidgetBindings::bindText(QLabel *label, std::function<QString()> fn)
{
  m_text.push_back({label, std::move(fn)});
}

void WidgetBindings::applyAll()
{
  for (const auto &b : m_visible)
    b.widget->setVisible(b.pred());

  for (const auto &b : m_enabled)
    b.widget->setEnabled(b.pred());

  for (const auto &b : m_text)
    b.label->setText(b.fn());
}
