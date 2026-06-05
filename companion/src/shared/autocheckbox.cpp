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

#include "autocheckbox.h"

AutoCheckBox::AutoCheckBox(QWidget * parent, const QString & text):
  QCheckBox(parent),
  AutoWidget(),
  m_field(nullptr),
  m_invert(false),
  m_value(false)
{
  QCheckBox::setText(text);
  connect(this, &QCheckBox::toggled, this, &AutoCheckBox::onToggled);
}

AutoCheckBox::~AutoCheckBox()
{
}

void AutoCheckBox::onToggled(bool checked)
{
  if (!lock()) {
    m_value = m_invert ? !checked : checked;

    if (m_field)
      *m_field = m_value;

    emit currentDataChanged(m_value);
    runPostChanged();
  }
}

void AutoCheckBox::setAutoText(QString text)
{
  QCheckBox::setText(text);
}

void AutoCheckBox::setField(bool & field, AbstractPanel * panel, bool invert)
{
  m_field = &field;
  m_invert = invert;
  setPanel(panel);

  m_value = *m_field;
  updateValue();
}

void AutoCheckBox::setFieldInit(AbstractPanel * panel, bool invert)
{
  setPanel(panel);
  setInvert(invert);
  updateValue();
}

void AutoCheckBox::setInvert(bool invert)
{
  m_invert = invert;
  updateValue();
}

void AutoCheckBox::setValue(bool value, AbstractPanel * panel, bool invert)
{
  if (!m_field) {
    m_value = value;
    setFieldInit(panel, invert);
  }
}

void AutoCheckBox::updateValue()
{
  setLock(true);
  setChecked(m_invert ? !m_value : m_value);
  setLock(false);
}
