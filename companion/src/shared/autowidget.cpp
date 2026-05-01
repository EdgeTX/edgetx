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
  m_panel(nullptr),
  m_lock(false),
  m_buddyWidgets(QList<AutoWidget *> {nullptr}),
  m_enabled(nullptr),
  m_model(nullptr),
  m_text(nullptr),
  m_visible(nullptr)
{
}

AutoWidget::~AutoWidget() = default;

void AutoWidget::addBuddyWidget(AutoWidget * wgt)
{
  if (wgt) {
    m_buddyWidgets.append(wgt);
    // just in case set as we want parent to control
    clearBuddyBinds(wgt);
  }
}

void AutoWidget::addBuddyWidgets(QList<AutoWidget *> wgts)
{
  m_buddyWidgets.append(wgts);
  // just in case set as we want parent to control
  for (const auto wgt: m_buddyWidgets) {
    if (wgt) clearBuddyBinds(wgt);
  }
}

// Apply bindings to this AutoWidget also enabled and visible to all buddy AutoWidgets
void AutoWidget::applyBindings()
{
  if (m_enabled) {
    // buddy widget has no bind so no code executed when it is updated
    // so apply parent value
    bool val = m_enabled();
    setAutoEnabled(val);

    for (const auto wgt: m_buddyWidgets) {
      if (wgt) wgt->setAutoEnabled(val);
    }
  }

  if (m_visible) {
    // buddy widget has no bind so no code executed when it is updated
    // so apply parent value
    bool val = m_visible();
    setAutoVisible(val);

    for (const auto wgt: m_buddyWidgets) {
      if (wgt) wgt->setAutoVisible(val);
    }
  }

  if (m_text) {
    // makes no sense to apply parent bind to buddies
    // leave it to each buddy to apply when it is updated
    setAutoText(m_text());
  }

  if (m_model) {
    // makes no sense to apply parent bind to buddies
    // leave it to each buddy to apply when it is updated
    setAutoModel(m_model());
  }
}

void AutoWidget::clearBindEnabled()
{
  m_enabled = nullptr;
}

void AutoWidget::clearBindVisible()
{
  m_visible = nullptr;
}

void AutoWidget::clearBuddyBinds(AutoWidget * wgt)
{
  if (wgt) {
    wgt->clearBindEnabled();
    wgt->clearBindVisible();
  }
}

void AutoWidget::clearBuddyWidget(AutoWidget * wgt)
{
  if (wgt) {
    if (!m_buddyWidgets.removeOne(wgt))
      qDebug() << "Warning: widget not removed from list";
  }
}

void AutoWidget::clearBuddyWidgets()
{
  m_buddyWidgets.clear();
}

void AutoWidget::dataChanged()
{
  if (m_panel)
    emit m_panel->modified();
}

bool AutoWidget::lock()
{
  return panelLock() | m_lock;
}

bool AutoWidget::panelLock()
{
  return m_panel ? m_panel->lock : false;
}

void AutoWidget::setAutoEnabled(bool enabled)
{
  QWidget *wgt = dynamic_cast<QWidget *>(this);
  if (wgt) wgt->setEnabled(enabled);
}

void AutoWidget::setAutoVisible(bool visible)
{
  QWidget *wgt = dynamic_cast<QWidget *>(this);
  if (wgt) wgt->setVisible(visible);
}

void AutoWidget::setBindEnabled(std::function<bool()> pred)
{
  m_enabled = std::move(pred);
}

void AutoWidget::setBindModel(std::function<QAbstractItemModel*()> fn)
{
  m_model = std::move(fn);
}

void AutoWidget::setBindText(std::function<QString()> fn)
{
  m_text = std::move(fn);
}

void AutoWidget::setBindVisible(std::function<bool()> pred)
{
  m_visible = std::move(pred);
}

void AutoWidget::setLock(bool lock)
{
  m_lock = lock;
}

void AutoWidget::setPanel(GenericPanel * panel)
{
  m_panel = panel;
}
