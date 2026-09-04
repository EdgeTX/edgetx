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

#include "autospinbox.h"

AutoSpinBox::AutoSpinBox(QWidget * parent):
  QSpinBox(parent),
  AutoWidget(parent),
  m_field(nullptr),
  m_value(0)
{
  connect(this, QOverload<int>::of(&QSpinBox::valueChanged), this, &AutoSpinBox::onValueChanged);
}

AutoSpinBox::~AutoSpinBox()
{
}

void AutoSpinBox::setField(int & field, AbstractPanel * panel)
{
  m_field = &field;
  setFieldInit(panel);
}

void AutoSpinBox::setField(unsigned int & field, AbstractPanel * panel)
{
  m_field = (int *)&field;
  setFieldInit(panel);
}

void AutoSpinBox::setFieldInit(AbstractPanel * panel)
{
  setPanel(panel);
  updateValue();
}

void AutoSpinBox::setValue(int val, AbstractPanel * panel)
{
  m_value = val;
  setFieldInit(panel);
}

void AutoSpinBox::setValue(int val)
{
  m_value = val;
  updateValue();
}

void AutoSpinBox::updateValue()
{
  setLock(true);

  if (m_field)
    QSpinBox::setValue(*m_field);
  else
    QSpinBox::setValue(m_value);

  setLock(false);
}

void AutoSpinBox::onValueChanged(int value)
{
  if (!lock()) {
    if (m_field && *m_field != value)
      *m_field = value;
    else if (m_value != value)
      m_value = value;

    emit currentDataChanged(value);
    runPostChanged();
  }
}
