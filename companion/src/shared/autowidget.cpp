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

#include "autowidget.h"
#include "genericpanel.h"

#include <memory>

AutoWidget::AutoWidget():
  m_enabled(nullptr),
  m_visible(nullptr),
  m_text(nullptr),
  m_panel(nullptr),
  m_lock(false)
{
}

AutoWidget::~AutoWidget() = default;

bool AutoWidget::lock()
{
  return panelLock() | m_lock;
}

void AutoWidget::setLock(bool lock)
{
  m_lock = lock;
}

void AutoWidget::setPanel(GenericPanel * panel)
{
  m_panel = panel;
}

void AutoWidget::dataChanged()
{
  if (m_panel)
    emit m_panel->modified();
}

bool AutoWidget::panelLock()
{
  if (m_panel)
    return m_panel->lock;
  else
    return false;
}

void AutoWidget::setBindVisible(std::function<bool()> pred)
{
  m_visible = std::move(pred);
}

void AutoWidget::setBindEnabled(std::function<bool()> pred)
{
  m_enabled = std::move(pred);
}

void AutoWidget::setBindText(std::function<QString()> fn)
{
  m_text = std::move(fn);
}

void AutoWidget::applyBindings()
{
  if (m_visible)
    setAutoVisible();

  if (m_enabled)
    setAutoEnabled();

  if (m_text)
    setAutoText();
}
